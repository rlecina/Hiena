#pragma once

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

			explicit JArrayBase(ArrayType Instance, LocalOwnership_t Tag, JNIEnv* Env)
				: JavaObjectBase(Instance, Tag)
				  , Size(Instance ? GetEnv(Env)->GetArrayLength(Instance) : 0)
			{
			}

			friend ArrayType ToArgument(const JArrayBase& Obj ) { return Obj.GetInstance(); }

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

			ValueType SetAt(jsize Idx, const T& Obj, JNIEnv* Env = nullptr)
			{
				ValueType Ret;
				Ret = GetEnv(Env)->SetObjectArrayElement(this->GetInstance(), Idx, ToArgument(Obj));
				//CheckException
				return Ret;
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
		static void ReleaseArrayElements(ArrayType Obj, TYPE* Elems, JNIEnv* Env = nullptr) \
		{ \
			GetEnv(Env)->Release##PRIMITIVE_TYPE##ArrayElements(Obj, Elems, 0); \
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

		template <typename T>
		class JPrimitiveArray : public JArrayBase<T>
		{
			static_assert(IsJniPrimitiveType<T>, "UnsupportedType");
		public:
			using ValueType = T;

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
				JArrayBase<ValueType>::operator=(std::move(Rhs));
				this->StoredRange = std::exchange(Rhs.StoredRange, nullptr);
				return *this;
			}

			JPrimitiveArray& operator=(const JPrimitiveArray& Rhs)
			{
				Release();
				JArrayBase<ValueType>::operator=(Rhs);
				return *this;
			}

			~JPrimitiveArray()
			{
				Release();
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
			
			void LoadRange(JNIEnv* Env)
			{
				if (StoredRange == nullptr)
				{
					StoredRange = PrimitiveArrayOps<ValueType>::GetArrayElements(GetEnv(Env), this->GetInstance());
				}
			}
			void Release()
			{
				if (StoredRange)
				{
					PrimitiveArrayOps<T>::ReleaseArrayElements(GetEnv(), this->GetInstance(), StoredRange);
					//CheckException?
					StoredRange = nullptr;
				}
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