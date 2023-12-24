#pragma once

#include <string_view>

#include "support/CompileTimeString.hpp"
#include "JavaLang.hpp"

namespace hiena
{
	template <typename T>
	struct HasJavaConversion
	{
		static constexpr bool Value = std::is_base_of_v<::java::lang::Object, T>;
	};

	namespace detail
	{
#ifdef __clang__
		static constexpr const char StartTag[] = "[Object = ";
		static constexpr const char EndTag[] = "]";
#elif defined(__GNUC__) || defined(__GNUG__)
		static constexpr const char StartTag[] = "[with Object = ";
		static constexpr const char EndTag[] = "]";
#endif

		template <typename>
		struct Mangler;

		template <typename Object>
		struct Mangler
		{
			template <int Length>
			static consteval auto FindTagExcluded(std::string_view ThisFunction, const char (&Tag)[Length])
			{
				return ThisFunction.rfind(Tag) + Length - 1;
			}

			static consteval auto FindTagIncluded(std::string_view ThisFunction, const char* Tag)
			{
				return ThisFunction.rfind(Tag);
			}

			static consteval auto GetCppTypename()
			{
				//constexpr auto Location = std::source_location::current(); // __PRETTY_FUNCTION__ works on gcc too
				constexpr std::string_view ThisFunction(__PRETTY_FUNCTION__); //Location.function_name());
				constexpr auto Start = FindTagExcluded(ThisFunction, StartTag);
				constexpr auto End = FindTagIncluded(ThisFunction, EndTag);
				constexpr auto Length = End - Start;
				return CompileTimeString<Length+1>(ThisFunction.data() + Start, Length);
			}

			static consteval auto GetJavaClass()
			{
				constexpr auto CppType = GetCppTypename();
				constexpr auto MangledSize = [&]
					{
						int Size = 0;
						char Prev = 0;
						for(auto C: CppType)
						{
							if (C != ':' || Prev != ':')
							{
								Size++;
							}
							Prev = C;
						}
						return Size;
					}();
				char Buffer[MangledSize];
				char Prev = 0;
				char* Current = &Buffer[0];
				for(auto C: CppType)
				{
					if (C != ':' || Prev != ':')
					{
						*Current++ = C == ':' ? '/' : C;
					}
					Prev = C;
				}
				return CompileTimeString<MangledSize>(Buffer);
			}

			static consteval auto GetJavaMangledType()
			{
				return "L" + GetJavaClass() + ";";
			}
		};

		template <typename T>
		using ValueType = std::remove_pointer_t<std::remove_cvref_t<T>>;

		template <typename T>
		struct MangledName
		{
			static_assert(HasJavaConversion<ValueType<T>>::Value, "Type does not support Java conversion");
			static constexpr auto Result = Mangler<T>::GetJavaMangledType();
		};

		template <typename Ret, typename... Args>
		struct MangledName<Ret(Args...)>
		{
			static_assert(HasJavaConversion<ValueType<Ret>>::Value, "Type does not support Java conversion");
			static_assert((HasJavaConversion<ValueType<Args>>::Value && ... && true), "Type does not support Java conversion");
			static constexpr auto Result = []
				{
					if constexpr(sizeof...(Args) > 0)
					{
						return "(" + (MangledName<ValueType<Args>>::Result + ...) + ")" + MangledName<ValueType<Ret>>::Result;
					}
					else
					{
						return "()" + MangledName<ValueType<Ret>>::Result;
					}
				}();
		};

		template <typename Ret, typename C, typename... Args>
		struct MangledName<Ret(C::*)(Args...)> : public MangledName<Ret(Args...)>
		{
		};

	}

	template <typename T>
	const char* ClassName()
	{
		static_assert(HasJavaConversion<detail::ValueType<T>>::Value, "Type does not support Java conversion");
		static constexpr auto Result = detail::Mangler<detail::ValueType<T>>::GetJavaClass();
		return Result.c_str();
	};

	template <typename T>
	const char* Mangle()
	{
		return detail::MangledName<T>::Result.c_str();
	};

	template <typename T>
	const char* Mangle(T)
	{
		return detail::MangledName<detail::ValueType<T>>::Result.c_str();
	};
}