#pragma once

#include "Hiena/Hiena.hpp"
#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/utility/Macros.hpp"

namespace hiena::detail
{
	template <typename T, bool IsObjectType = std::is_base_of_v<JavaObjectBase, T>>
	struct JniArrayTypeForImpl
	{
		static_assert(AlwaysFalse<T>, "Unsupported type for array");
	};

	template <typename T>
	struct JniArrayTypeForImpl<T, true> { using ArrayType = jobjectArray; };

#define HIENA_ASSOCIATE_ARRAYTYPE(TYPE) \
		template <> struct JniArrayTypeForImpl<TYPE, false> { \
			using ArrayType = TYPE##Array;};

	HIENA_ASSOCIATE_ARRAYTYPE(jboolean)
	HIENA_ASSOCIATE_ARRAYTYPE(jbyte)
	HIENA_ASSOCIATE_ARRAYTYPE(jchar)
	HIENA_ASSOCIATE_ARRAYTYPE(jshort)
	HIENA_ASSOCIATE_ARRAYTYPE(jint)
	HIENA_ASSOCIATE_ARRAYTYPE(jlong)
	HIENA_ASSOCIATE_ARRAYTYPE(jfloat)
	HIENA_ASSOCIATE_ARRAYTYPE(jdouble)

#undef HIENA_ASSOCIATE_ARRAYTYPE

	template <typename T>
	using JniArrayTypeFor = typename detail::JniArrayTypeForImpl<T>::ArrayType;

	template <typename T>
	class JArrayBase : public JavaObjectBase
	{
	public:
		using ValueType = T;
		using SourceJniType = JniArrayTypeFor<T>;

		JArrayBase() {}
		explicit JArrayBase(SourceJniType Instance, JNIEnv* Env = nullptr)
				: JavaObjectBase(Instance)
					, Size(Instance ? GetEnv(Env)->GetArrayLength(Instance) : 0)
		{
		}

		explicit JArrayBase(SourceJniType Instance, LocalOwnership_t Tag, JNIEnv* Env = nullptr)
				: JavaObjectBase(Instance, Tag)
					, Size(Instance ? GetEnv(Env)->GetArrayLength(Instance) : 0)
		{
		}

		friend SourceJniType ToJniArgument(const JArrayBase& Obj, JNIEnv*) { return Obj.GetInstance(); }

		jsize GetSize()
		{
			return Size;
		}
	protected:
		jsize Size = 0;

		SourceJniType GetInstance() const
		{
			return (SourceJniType)JavaObjectBase::GetInstance();
		}
	};

	template <typename>
	struct PrimitiveArrayOps;

#define HIENA_SETUP_PRIMITIVEARRAY_OPS(TYPE, PRIMITIVE_TYPE) \
	template <>\
	struct PrimitiveArrayOps<TYPE> \
	{ \
		using SourceJniType = JniArrayTypeFor<TYPE>; \
		static SourceJniType NewArray(jsize Size, JNIEnv* Env = nullptr) \
		{ \
			SourceJniType Ret = GetEnv(Env)->New##PRIMITIVE_TYPE##Array(Size); \
			/*CheckException*/ \
			return Ret; \
		} \
		static TYPE* GetArrayElements(SourceJniType Obj, JNIEnv* Env = nullptr) \
		{ \
			TYPE* Ret = GetEnv(Env)->Get##PRIMITIVE_TYPE##ArrayElements(Obj, nullptr); \
			/*CheckException*/ \
			return Ret; \
		} \
		static void ReleaseArrayElements(SourceJniType Obj, TYPE* Elems, jint Mode, JNIEnv* Env = nullptr) \
		{ \
			GetEnv(Env)->Release##PRIMITIVE_TYPE##ArrayElements(Obj, Elems, Mode); \
			/*CheckException*/ \
		} \
		static void GetArrayRegion(SourceJniType Obj, jsize start, jsize len, TYPE* Dst, JNIEnv* Env = nullptr) \
		{ \
			GetEnv(Env)->Get##PRIMITIVE_TYPE##ArrayRegion(Obj, start, len, Dst); \
			/*CheckException*/ \
		} \
		static void SetArrayRegion(SourceJniType Obj, jsize start, jsize len, const TYPE* Src, JNIEnv* Env = nullptr) \
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
}