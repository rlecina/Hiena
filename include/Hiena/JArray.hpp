#pragma once

#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/utility/JniTraits.hpp"

namespace hiena
{
	template <typename T>
	class JArray : public detail::JavaObjectBase
	{
	public:
		using ValueType = T;
		using ArrayType = JniArrayTypeFor<T>;

		JArray() = default;
		explicit JArray(ArrayType Instance)
			:JavaObjectBase(Instance)
		{
		}
		explicit JArray(ArrayType Instance, LocalOwnership_t Tag)
			: JavaObjectBase(Instance, Tag)
		{
		}

		friend ArrayType ToArgument(const JArray& Obj ) { return (ArrayType)Obj.getInstance(); }
	};
}