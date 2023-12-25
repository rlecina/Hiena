#pragma once

#include "meta/Helpers.hpp"

#include <tuple>

namespace hiena
{
	template <typename FuncType>
	struct FuncSig
	{
		static_assert(AlwaysFalse<FuncType>, "Not a function or member function type");
	};

	template <typename R, typename... As>
	struct FuncSig<R(As...)>
	{
		using Ret = R;
		using Args = std::tuple<As...>;

		template <size_t Idx>
		using Arg = std::tuple_element_t<Idx, Args>;

		static inline constexpr int Arity = sizeof...(As);
	};

	template <typename T>
	using FuncSig_R = typename FuncSig<T>::Ret;

	template <typename T>
	using FuncSig_Args = typename FuncSig<T>::Args;

	template <typename T, size_t Index>
	using FuncSig_A = typename FuncSig<T>::template Arg<Index>;

	template <typename Ret, typename... Args>
	struct FuncSig<Ret(Args..., ...)> : FuncSig<Ret(Args...)> {};

#define HIENA_MEMFUNC(CONST, VOLATILE, REF_QUAL, NOEXCEPT)	\
				template <typename Ret, typename C, typename... Args>\
				struct FuncSig<Ret(C::*)(Args...) CONST VOLATILE REF_QUAL NOEXCEPT> : FuncSig<Ret(Args...)> {};

#define HIENA_MEMFUNC_VARIADIC(CONST, VOLATILE, REF_QUAL, NOEXCEPT)	\
				template <typename Ret, typename C, typename... Args>\
				struct FuncSig<Ret(C::*)(Args..., ...) CONST VOLATILE REF_QUAL NOEXCEPT> : FuncSig<Ret(Args...)> {};

#include "meta/MemFunVariants.hpp"

#undef HIENA_MEMFUNC
#undef HIENA_MEMFUNC_VARIADIC
}