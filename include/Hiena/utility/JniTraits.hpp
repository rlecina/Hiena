#pragma once

#include <jni.h>
#include <type_traits>

#include "Hiena/detail/JArrayBase.hpp"
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
	inline constexpr bool IsJniObjectType = std::is_base_of_v<JavaObjectBase, T>;

	namespace detail
	{
		template <typename T>
		struct IsJArrayTypeImpl
		{
			static inline constexpr bool Value = false;
		};

		template <template <typename> typename Cont, typename T>
		struct IsJArrayTypeImpl<Cont<T>>
		{
			static inline constexpr bool Value = std::is_base_of_v<JArrayBase<T>, Cont<T>>;
		};
	}

	template <typename T>
	inline static constexpr bool IsJArrayType = detail::IsJArrayTypeImpl<T>::Value;

	template <typename T>
	struct HasJniConversion
	{
		static constexpr bool Value = IsJniObjectType<T> || IsJniPrimitiveType<T>;
	};

	template <typename T>
	using LessSpecializedJniType = std::conditional_t<std::is_base_of_v<std::remove_pointer_t<jobject>,
																		std::remove_pointer_t<T>>,
														jobject,
														T>;

	template <typename T>
	std::enable_if_t<IsJniRegularPrimitiveType<T>,T> ToJniArgument(T Arg, CheckedJniEnv) { return Arg; }
}