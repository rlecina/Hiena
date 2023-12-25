#pragma once

#include <type_traits>

namespace hiena
{
	template <typename>
	constexpr inline bool AlwaysFalse = false;

	template <typename>
	constexpr inline bool AlwaysTrue = true;

	template <typename T>
	using ValueType = std::remove_pointer_t<std::remove_cvref_t<T>>;

	template <typename T>
	using ValueType = std::remove_pointer_t<std::remove_cvref_t<T>>;

	template <typename T>
	constexpr inline bool IsFunctionPointer = std::is_function_v<std::remove_pointer_t<T>>;
}