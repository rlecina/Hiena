#pragma once

#include <utility>

namespace hiena
{
	template <typename T>
	struct ScopeExit
	{
		ScopeExit(T&& InFun)
		: Fun(std::move(InFun))
		{}
		~ScopeExit() { Fun(); }

	private:
		T Fun;
	};
}