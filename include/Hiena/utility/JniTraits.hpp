#pragma once

#include <jni.h>
#include <type_traits>

#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/meta/Helpers.hpp"

namespace hiena
{
	template <typename T>
	inline constexpr bool IsJniRegularPrimitiveType = std::is_same_v<jboolean, T> ||
											   std::is_same_v<jbyte, T> ||
											   std::is_same_v<jchar, T> ||
											   std::is_same_v<jint, T> ||
											   std::is_same_v<jshort, T> ||
											   std::is_same_v<jlong, T> ||
											   std::is_same_v<jfloat, T> ||
											   std::is_same_v<jdouble, T>;

	template <typename T>
	inline constexpr bool IsJniPrimitiveType = std::is_same_v<void, T> || IsJniRegularPrimitiveType<T>;

	template <typename T>
	inline constexpr bool IsJniPrimitiveArrayType = std::is_same_v<jbooleanArray, T> ||
			std::is_same_v<jbyteArray, T> ||
			std::is_same_v<jcharArray, T> ||
			std::is_same_v<jshortArray, T> ||
			std::is_same_v<jintArray, T> ||
			std::is_same_v<jlongArray, T> ||
			std::is_same_v<jfloatArray, T> ||
			std::is_same_v<jdoubleArray, T>;

	template <typename T>
	inline constexpr bool IsJniObjectType = std::is_base_of_v<detail::JavaObjectBase, T>;

	namespace detail
	{
		template <typename T, bool IsObjectType = IsJniObjectType<T>>
		struct JniArrayTypeFor
		{
			static_assert(AlwaysFalse<T>, "Unsupported type for array");
		};

		template <typename T>
		struct JniArrayTypeFor<T, true> { using ArrayType = jobjectArray; };

#define HIENA_ASSOCIATE_ARRAYTYPE(TYPE)	template <> struct JniArrayTypeFor<TYPE, false> { using ArrayType = TYPE##Array;};
		HIENA_ASSOCIATE_ARRAYTYPE(jboolean)
		HIENA_ASSOCIATE_ARRAYTYPE(jbyte)
		HIENA_ASSOCIATE_ARRAYTYPE(jchar)
		HIENA_ASSOCIATE_ARRAYTYPE(jshort)
		HIENA_ASSOCIATE_ARRAYTYPE(jint)
		HIENA_ASSOCIATE_ARRAYTYPE(jlong)
		HIENA_ASSOCIATE_ARRAYTYPE(jfloat)
		HIENA_ASSOCIATE_ARRAYTYPE(jdouble)
#undef HIENA_ASSOCIATE_ARRAYTYPE
	}

	template <typename T>
	using JniArrayTypeFor = typename detail::JniArrayTypeFor<T>::ArrayType;

}