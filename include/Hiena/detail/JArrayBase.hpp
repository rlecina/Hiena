#pragma once

#include "Hiena/CheckedJniEnv.hpp"
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
		explicit JArrayBase(SourceJniType Instance, CheckedJniEnv Env = {})
			: JavaObjectBase(Instance)
			, Size(Instance ? Env->GetArrayLength(Instance) : 0)
		{
		}

		explicit JArrayBase(SourceJniType Instance, LocalOwnership_t Tag, CheckedJniEnv Env = {})
			: JavaObjectBase(Instance, Tag)
			, Size(Instance ? Env->GetArrayLength(Instance) : 0)
		{
		}

		friend SourceJniType ToJniArgument(const JArrayBase& Obj, CheckedJniEnv) { return Obj.GetInstance(); }

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
		static SourceJniType NewArray(jsize Size, CheckedJniEnv Env = {}) \
		{ \
			return Env->New##PRIMITIVE_TYPE##Array(Size); \
		} \
		static TYPE* GetArrayElements(SourceJniType Obj, CheckedJniEnv Env = {}) \
		{ \
			return Env->Get##PRIMITIVE_TYPE##ArrayElements(Obj, nullptr); \
		} \
		static void ReleaseArrayElements(SourceJniType Obj, TYPE* Elems, jint Mode, CheckedJniEnv Env = {}) \
		{ \
			Env->Release##PRIMITIVE_TYPE##ArrayElements(Obj, Elems, Mode); \
		} \
		static void GetArrayRegion(SourceJniType Obj, jsize start, jsize len, TYPE* Dst, CheckedJniEnv Env = {}) \
		{ \
			Env->Get##PRIMITIVE_TYPE##ArrayRegion(Obj, start, len, Dst); \
		} \
		static void SetArrayRegion(SourceJniType Obj, jsize start, jsize len, const TYPE* Src, CheckedJniEnv Env = {}) \
		{ \
			Env->Set##PRIMITIVE_TYPE##ArrayRegion(Obj, start, len, Src); \
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