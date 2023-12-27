#pragma once

#include <memory>
#include <utility>

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

			HIENA_CLASS_CONSTRUCTORS_ARRAY(JObjectArray, JArrayBase<T>, typename JArrayBase<T>::SourceJniType)

			ValueType GetAt(jsize Idx, JNIEnv* Env = nullptr)
			{
				ValueType Ret;
				Env = GetEnv(Env);
				if (CheckExceptionFast())
				{
					return {};
				}
				Ret = Env->GetObjectArrayElement(this->GetInstance(),Idx);
				CheckException(Env);
				return Ret;
			}

			void SetAt(jsize Idx, const T& Obj, JNIEnv* Env = nullptr)
			{
				Env = GetEnv(Env);
				if (CheckExceptionFast())
				{
					return;
				}
				GetEnv(Env)->SetObjectArrayElement(this->GetInstance(), Idx, ToJniArgument(Obj, Env));
				CheckException(Env);
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

			explicit JPrimitiveArray(SourceJniType Instance, JNIEnv* Env = nullptr)
			: JArrayBase<T>(Instance, Env)
			{
			}

			JPrimitiveArray(SourceJniType Instance, LocalOwnership_t Tag, JNIEnv* Env = nullptr)
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

			friend SourceJniType ToJniArgument(const JPrimitiveArray& Obj, JNIEnv* Env)
			{
				Obj.Commit(Env);
				return Obj.GetInstance();
			}

			ValueType GetAt(jsize Idx, JNIEnv* Env = nullptr)
			{
				LoadRange(Env);
				return StoredRange[Idx];
			}

			void SetAt(jsize Idx, ValueType Value, JNIEnv* Env = nullptr)
			{
				LoadRange(Env);
				StoredRange[Idx] = Value;
			}
		private:
			ValueType* StoredRange = nullptr;
			std::shared_ptr<Empty_t> SharedCount;

			void LoadRange(JNIEnv* Env)
			{
				if (StoredRange == nullptr)
				{
					StoredRange = PrimitiveArrayOps<ValueType>::GetArrayElements(this->GetInstance(), Env);
					SharedCount = std::make_shared<Empty_t>();
				}
			}
			void Commit(JNIEnv* Env) const
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
					PrimitiveArrayOps<T>::ReleaseArrayElements(this->GetInstance(), StoredRange, 0, GetEnv());
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