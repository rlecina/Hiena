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
				ValueType Ret;
				Ret = Env->GetObjectArrayElement(this->GetInstance(),Idx);
				return Ret;
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

			JPrimitiveArray() {}

			explicit JPrimitiveArray(SourceJniType Instance, CheckedJniEnv Env = {})
			: JArrayBase<T>(Instance, Env)
			{
			}

			JPrimitiveArray(SourceJniType Instance, LocalOwnership_t Tag, CheckedJniEnv Env = {})
			: JArrayBase<T>(Instance, Tag)
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
					StoredRange = std::exchange(Rhs.StoredRange, nullptr);
					SharedCount = std::exchange(Rhs.SharedCount, nullptr);
				}
				return *this;
			}

			JPrimitiveArray& operator=(const JPrimitiveArray& Rhs)
			{
				if (this != &Rhs)
				{
					Release();
					JArrayBase<ValueType>::operator=(Rhs);
					StoredRange = Rhs.StoredRange;
					SharedCount = Rhs.SharedCount;
				}
				return *this;
			}

			~JPrimitiveArray()
			{
				Release();
			}

			friend SourceJniType ToJniArgument(const JPrimitiveArray& Obj, CheckedJniEnv Env)
			{
				Obj.Commit(Env);
				return (SourceJniType)Obj.GetInstance();
			}

			ValueType GetAt(jsize Idx, CheckedJniEnv Env = {})
			{
				LoadRange(Env);
				return StoredRange[Idx];
			}

			void SetAt(jsize Idx, ValueType Value, CheckedJniEnv Env = {})
			{
				LoadRange(Env);
				StoredRange[Idx] = Value;
			}
		private:
			ValueType* StoredRange = nullptr;
			std::shared_ptr<Empty_t> SharedCount;

			void LoadRange(CheckedJniEnv Env)
			{
				if (StoredRange == nullptr)
				{
					StoredRange = PrimitiveArrayOps<ValueType>::GetArrayElements(this->GetInstance(), Env);
					SharedCount = std::make_shared<Empty_t>();
				}
			}
			void Commit(CheckedJniEnv Env) const
			{
				if (StoredRange)
				{
					PrimitiveArrayOps<T>::ReleaseArrayElements(this->GetInstance(), StoredRange, JNI_COMMIT, Env);
				}
			}

			void Release()
			{
				if (StoredRange && SharedCount.use_count() == 1)
				{
					PrimitiveArrayOps<T>::ReleaseArrayElements(this->GetInstance(), StoredRange, 0);
				}
				StoredRange = nullptr;
				SharedCount = nullptr;
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