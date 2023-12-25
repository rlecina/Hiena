#pragma once

#include <type_traits>

namespace hiena
{
	template <typename T>
	constexpr inline bool AlwaysFalse = false;

	template <typename T>
	constexpr inline bool AlwaysTrue = true;

	template <typename T>
	using ValueType = std::remove_pointer_t<std::remove_cvref_t<T>>;
}