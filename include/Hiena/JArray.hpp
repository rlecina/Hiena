#pragma once

#include <memory>
#include <utility>

#include "Hiena/CheckedJniEnv.hpp"
#include "Hiena/detail/JArrayBase.hpp"
#include "Hiena/utility/JniTraits.hpp"

namespace hiena
{
	namespace detail
	{
		template <typename T>
		class JObjectArray : public JArrayBase<T>
		{
			static_assert(IsJniObjectType<T>, "Unsupported type");
		public:
			using ValueType = typename JArrayBase<T>::ValueType;
			using SourceJniType = typename JArrayBase<T>::SourceJniType;

			JObjectArray() {}
			explicit JObjectArray(SourceJniType Instance)
				:JArrayBase<T>((SourceJniType)Instance) {}
			JObjectArray(SourceJniType Instance, hiena::LocalOwnership_t Tag)
				:JArrayBase<T>((SourceJniType)Instance, Tag) {}
			JObjectArray(const JObjectArray& Other)
				:JArrayBase<T>(Other) {}
			JObjectArray(JObjectArray&& Other)
				:JArrayBase<T>(Other) {}
			JObjectArray& operator=(JObjectArray&& Rhs) = default;
			JObjectArray& operator=(const JObjectArray& Rhs) = default;

			friend SourceJniType ToJniArgument(const JObjectArray& Obj, hiena::CheckedJniEnv Env)
			{
				return (SourceJniType)Obj.GetInstance();
			}

			ValueType GetAt(jsize Idx, CheckedJniEnv Env = {})
			{
				return Env->GetObjectArrayElement(this->GetInstance(),Idx);
			}

			void SetAt(jsize Idx, const T& Obj, CheckedJniEnv Env = {})
			{
				Env->SetObjectArrayElement(this->GetInstance(), Idx, ToJniArgument(Obj, Env));
			}
		};

		template <typename T>
		class JPrimitiveArray : public JArrayBase<T>
		{
			static_assert(IsJniPrimitiveType<T>, "UnsupportedType");
		public:
			using ValueType = typename JArrayBase<T>::ValueType;
			using SourceJniType = typename JArrayBase<T>::SourceJniType;

			JPrimitiveArray() = default;

			explicit JPrimitiveArray(SourceJniType Instance, CheckedJniEnv Env = {})
			: JArrayBase<T>(Instance, Env)
			, SharedState(std::make_shared<State>())
			{
			}

			JPrimitiveArray(SourceJniType Instance, LocalOwnership_t Tag, CheckedJniEnv Env = {})
			: JArrayBase<T>(Instance, Tag)
			, SharedState(std::make_shared<State>())
			{
			}

			JPrimitiveArray(const JPrimitiveArray& Other)
			{
				*this = Other;
			}

			JPrimitiveArray(JPrimitiveArray&& Other)
			{
				*this = std::move(Other);
			}

			JPrimitiveArray& operator=(JPrimitiveArray&& Rhs)
			{
				if (this != &Rhs)
				{
					Release();
					JArrayBase<ValueType>::operator=(std::move(Rhs));
					SharedState = std::exchange(Rhs.SharedState, nullptr);
				}
				return *this;
			}

			JPrimitiveArray& operator=(const JPrimitiveArray& Rhs)
			{
				if (this != &Rhs)
				{
					Release();
					JArrayBase<ValueType>::operator=(Rhs);
					SharedState = Rhs.SharedState;
				}
				return *this;
			}

			~JPrimitiveArray()
			{
				Release();
			}

			ValueType GetAt(jsize Idx, CheckedJniEnv Env = {})
			{
				if (!SharedState->StoredRange)
				{
					Pull(Env);
				}
				return SharedState->StoredRange[Idx];
			}

			void SetAt(jsize Idx, ValueType Value, CheckedJniEnv Env = {})
			{
				if (!SharedState->StoredRange)
				{
					Pull(Env);
				}
				SharedState->StoredRange[Idx] = Value;
				SharedState->bIsModified = true;
			}

			void Pull(CheckedJniEnv Env = {})
			{
				if (SharedState->StoredRange)
				{
					PrimitiveArrayOps<T>::ReleaseArrayElements(this->GetInstance(), SharedState->StoredRange, JNI_ABORT, Env);
					SharedState->bIsModified = false;
				}
				SharedState->StoredRange = PrimitiveArrayOps<ValueType>::GetArrayElements(this->GetInstance(), Env);
			}

			void Commit(CheckedJniEnv Env = {}) const
			{
				if (SharedState->StoredRange && SharedState->bIsModified)
				{
					PrimitiveArrayOps<T>::ReleaseArrayElements(this->GetInstance(), SharedState->StoredRange, JNI_COMMIT, Env);
					SharedState->bIsModified = false;
				}
			}

			friend SourceJniType ToJniArgument(const JPrimitiveArray& Obj, CheckedJniEnv Env)
			{
				Obj.Commit(Env);
				return (SourceJniType)Obj.GetInstance();
			}

		private:
			struct State
			{
				ValueType* StoredRange = nullptr;
				bool bIsModified = false;
			};
			std::shared_ptr<State> SharedState;

			void Release()
			{
				if (SharedState.use_count() == 1)
				{
					CheckedJniEnv Env;
					if(SharedState->StoredRange)
					{
						jint Mode = SharedState->bIsModified? JNI_COMMIT : JNI_ABORT;
						PrimitiveArrayOps<T>::ReleaseArrayElements(this->GetInstance(), SharedState->StoredRange, Mode, Env);
					}
				}
				SharedState.reset();
			}
		};

		template <typename T>
		using ArrayBaseType = std::conditional_t<IsJniObjectType<T>, JObjectArray<T>, JPrimitiveArray<T>>;
	}

	template <typename T>
	class JArray : public detail::ArrayBaseType<T>
	{
		using Base = detail::ArrayBaseType<T>;
	public:
		using ValueType = T;

		using Base::Base;
	};
}