#pragma once

#include <memory>
#include <utility>

#include "Hiena/Hiena.hpp"
#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/utility/JniTraits.hpp"

namespace hiena
{
	namespace detail
	{
		template <typename T>
		class JArrayBase : public JavaObjectBase
		{
		public:
			using ArrayType = JniArrayTypeFor<T>;

			JArrayBase() = default;
			explicit JArrayBase(ArrayType Instance, JNIEnv* Env = nullptr)
				: JavaObjectBase(Instance)
				, Size(Instance ? GetEnv(Env)->GetArrayLength(Instance) : 0)
			{
			}

			explicit JArrayBase(ArrayType Instance, LocalOwnership_t Tag, JNIEnv* Env = nullptr)
				: JavaObjectBase(Instance, Tag)
				  , Size(Instance ? GetEnv(Env)->GetArrayLength(Instance) : 0)
			{
			}

			friend ArrayType ToJniArgument(const JArrayBase& Obj, JNIEnv*) { return Obj.GetInstance(); }

			jsize GetSize()
			{
				return Size;
			}
		protected:
			jsize Size = 0;

			ArrayType GetInstance() const
			{
				return (ArrayType)JavaObjectBase::GetInstance();
			}
		};

		template <typename T>
		class JObjectArray : public JArrayBase<T>
		{
			static_assert(IsJniObjectType<T>, "Unsupported type");
		public:
			using ValueType = T;

			using JArrayBase<T>::JArrayBase;

			ValueType GetAt(jsize Idx, JNIEnv* Env = nullptr)
			{
				ValueType Ret;
				Ret = GetEnv(Env)->GetObjectArrayElement(this->GetInstance(),Idx);
				//CheckException
				return Ret;
			}

			void SetAt(jsize Idx, const T& Obj, JNIEnv* Env = nullptr)
			{
				GetEnv(Env)->SetObjectArrayElement(this->GetInstance(), Idx, ToJniArgument(Obj, Env));
				//CheckException
			}
		};

		template <typename>
		struct PrimitiveArrayOps;

#define HIENA_SETUP_PRIMITIVEARRAY_OPS(TYPE, PRIMITIVE_TYPE) \
	template <>\
	struct PrimitiveArrayOps<TYPE> \
	{ \
		using ArrayType = JniArrayTypeFor<TYPE>; \
		static ArrayType NewArray(jsize Size, JNIEnv* Env = nullptr) \
		{ \
			ArrayType Ret = GetEnv(Env)->New##PRIMITIVE_TYPE##Array(Size); \
			/*CheckException*/ \
			return Ret; \
		} \
		static TYPE* GetArrayElements(ArrayType Obj, JNIEnv* Env = nullptr) \
		{ \
			TYPE* Ret = GetEnv(Env)->Get##PRIMITIVE_TYPE##ArrayElements(Obj, nullptr); \
			/*CheckException*/ \
			return Ret; \
		} \
		static void ReleaseArrayElements(ArrayType Obj, TYPE* Elems, jint Mode, JNIEnv* Env = nullptr) \
		{ \
			GetEnv(Env)->Release##PRIMITIVE_TYPE##ArrayElements(Obj, Elems, Mode); \
			/*CheckException*/ \
		} \
		static void GetArrayRegion(ArrayType Obj, jsize start, jsize len, TYPE* Dst, JNIEnv* Env = nullptr) \
		{ \
			GetEnv(Env)->Get##PRIMITIVE_TYPE##ArrayRegion(Obj, start, len, Dst); \
			/*CheckException*/ \
		} \
		static void SetArrayRegion(ArrayType Obj, jsize start, jsize len, const TYPE* Src, JNIEnv* Env = nullptr) \
		{ \
			GetEnv(Env)->Set##PRIMITIVE_TYPE##ArrayRegion(Obj, start, len, Src); \
			/*CheckException*/ \
		} \
	};

		HIENA_SETUP_PRIMITIVEARRAY_OPS(jboolean, Boolean)
		HIENA_SETUP_PRIMITIVEARRAY_OPS(jbyte, Byte)
		HIENA_SETUP_PRIMITIVEARRAY_OPS(jchar, Char)
		HIENA_SETUP_PRIMITIVEARRAY_OPS(jshort, Short)
		HIENA_SETUP_PRIMITIVEARRAY_OPS(jint, Int)
		HIENA_SETUP_PRIMITIVEARRAY_OPS(jlong, Long)
		HIENA_SETUP_PRIMITIVEARRAY_OPS(jfloat, Float)
		HIENA_SETUP_PRIMITIVEARRAY_OPS(jdouble, Double)

#undef HIENA_SETUP_PRIMITIVEARRAY_OPS

		struct Empty_t{};

		template <typename T>
		class JPrimitiveArray : public JArrayBase<T>
		{
			static_assert(IsJniPrimitiveType<T>, "UnsupportedType");
		public:
			using ValueType = T;
			using ArrayType = typename JArrayBase<T>::ArrayType;

			using JArrayBase<T>::JArrayBase;

			explicit JPrimitiveArray(const JPrimitiveArray& Other)
			{
				*this = Other;
			}

			explicit JPrimitiveArray(JPrimitiveArray&& Other)
			{
				*this = std::move(Other);
			}

			JPrimitiveArray& operator=(JPrimitiveArray&& Rhs)
			{
				Release();
				JArrayBase<ValueType>::operator=(std::move(Rhs));
				StoredRange = std::exchange(Rhs.StoredRange, nullptr);
				SharedCount = std::exchange(Rhs.SharedCount, nullptr);
				return *this;
			}

			JPrimitiveArray& operator=(const JPrimitiveArray& Rhs)
			{
				Release();
				JArrayBase<ValueType>::operator=(Rhs);
				StoredRange = Rhs.StoredRange;
				SharedCount = Rhs.SharedCount;
				return *this;
			}

			~JPrimitiveArray()
			{
				Release();
			}

			friend ArrayType ToJniArgument(const JPrimitiveArray& Obj, JNIEnv* Env)
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
					//CheckException?
				}
			}

			void Release()
			{
				if (StoredRange && SharedCount.use_count() == 1)
				{
					PrimitiveArrayOps<T>::ReleaseArrayElements(this->GetInstance(), StoredRange, 0, GetEnv());
					//CheckException?
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