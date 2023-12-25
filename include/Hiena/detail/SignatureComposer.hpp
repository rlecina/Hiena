#pragma once

#include <string_view>

#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/JArray.hpp"
#include "Hiena/meta/Helpers.hpp"
#include "Hiena/utility/CompileTimeString.hpp"
#include "Hiena/utility/JniTraits.hpp"

namespace hiena
{
	template <typename T>
	struct HasJavaConversion
	{
		static constexpr bool Value = IsJniObjectType<T> || IsJniPrimitiveType<T>;
	};

	namespace detail
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

		template <auto N>
		constexpr int ComputeJavaClassLength(const CompileTimeString<N>& CppType)
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
		}

		template <auto PathLikeSize, auto N>
		constexpr auto ComputeJavaClass(const CompileTimeString<N>& CppType)
		{
			char Buffer[PathLikeSize];
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
			return CompileTimeString<PathLikeSize>(Buffer);
		}

#ifdef __clang__
		static constexpr const char StartFuncTag[] = "::";
		static constexpr const char EndFuncTag[] = "]";
#elif defined(__GNUC__) || defined(__GNUG__)
		static constexpr const char StartFuncTag[] = "::";
		static constexpr const char EndFuncTag[] = "]";
#endif

		template <auto Func>
		struct GetFuncName
		{
			static constexpr auto Result = [] {
					static_assert(IsFunctionPointer<decltype(Func)> || std::is_member_function_pointer_v<decltype(Func)>, "Unsupported type");
					constexpr std::string_view ThisFunction(__PRETTY_FUNCTION__); //Location.function_name());
					constexpr auto Start = FindTagExcluded(ThisFunction, StartFuncTag);
					constexpr auto End = FindTagIncluded(ThisFunction, EndFuncTag);
					return CompileTimeString<End - Start + 1>(ThisFunction.data() + Start, End - Start);
				}();
		};

#ifdef __clang__
		static constexpr const char StartFuncClassTag[] = "&";
		static constexpr const char EndFuncClassTag[] = "::";
#elif defined(__GNUC__) || defined(__GNUG__)
		static constexpr const char StartFuncClassTag[] = "&";
		static constexpr const char EndFuncClassTag[] = "::";
#endif

		template <auto Func>
		struct GetJavaClassFromFunc
		{
			static constexpr auto Result = [] {
				static_assert(IsFunctionPointer<decltype(Func)> || std::is_member_function_pointer_v<decltype(Func)>, "Unsupported type");
				constexpr std::string_view ThisFunction(__PRETTY_FUNCTION__); //Location.function_name());
				constexpr auto Start = FindTagExcluded(ThisFunction, StartFuncClassTag);
				constexpr auto End = FindTagIncluded(ThisFunction, EndFuncClassTag);
				constexpr auto CppType = CompileTimeString<End - Start + 1>(ThisFunction.data() + Start, End - Start);
				constexpr auto MangledSize = ComputeJavaClassLength(CppType);
				return ComputeJavaClass<MangledSize>(CppType);
			}();
		};

#ifdef __clang__
		static constexpr const char StartTag[] = "[Object = ";
		static constexpr const char EndTag[] = "]";
#elif defined(__GNUC__) || defined(__GNUG__)
		static constexpr const char StartTag[] = "[with Object = ";
		static constexpr const char EndTag[] = "]";
#endif

		template <typename Object>
		struct Mangler
		{
			static consteval auto GetCppTypename()
			{
				//constexpr auto Location = std::source_location::current(); // __PRETTY_FUNCTION__ works on gcc too
				constexpr std::string_view ThisFunction(__PRETTY_FUNCTION__); //Location.function_name());
				constexpr auto Start = FindTagExcluded(ThisFunction, StartTag);
				constexpr auto End = FindTagIncluded(ThisFunction, EndTag);
				constexpr auto Length = End - Start;
				return CompileTimeString<Length+1>(ThisFunction.data() + Start, Length);
			}

			static consteval auto GetJavaClassName()
			{
				constexpr auto CppType = GetCppTypename();
				constexpr auto PathLikeSize = ComputeJavaClassLength(CppType);
				return ComputeJavaClass<PathLikeSize>(CppType);
			}

			static consteval auto GetJavaMangledType()
			{
				return "L" + GetJavaClassName() + ";";
			}
		};

		template <typename T>
		struct GetJavaClassName
		{
			static_assert(HasJavaConversion<ValueType<T>>::Value, "Type does not support Java conversion");
			static constexpr auto Result = Mangler<ValueType<T>>::GetJavaClassName();
		};

		template <typename T>
		struct MangledName
		{
			static_assert(HasJavaConversion<ValueType<T>>::Value, "Type does not support Java conversion");
			static constexpr auto Result = Mangler<T>::GetJavaMangledType();
		};

#define HIENA_PRIMITIVE_MANGLED_NAME(TYPE, MANGLED) \
        template <> \
		struct MangledName<TYPE> { static constexpr auto Result = MANGLED##_cs; };

		HIENA_PRIMITIVE_MANGLED_NAME(void, "V")
		HIENA_PRIMITIVE_MANGLED_NAME(jboolean, "Z")
		HIENA_PRIMITIVE_MANGLED_NAME(jbyte, "B")
		HIENA_PRIMITIVE_MANGLED_NAME(jchar, "C")
		HIENA_PRIMITIVE_MANGLED_NAME(jshort, "S")
		HIENA_PRIMITIVE_MANGLED_NAME(jint, "I")
		HIENA_PRIMITIVE_MANGLED_NAME(jlong, "J")
		HIENA_PRIMITIVE_MANGLED_NAME(jfloat, "F")
		HIENA_PRIMITIVE_MANGLED_NAME(jdouble, "D")

#undef HIENA_PRIMITIVE_MANGLED_NAME

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

		template <typename T>
		struct MangledName<JArray<T>>
		{
			static constexpr auto Result = "[" + MangledName<ValueType<T>>::Result;
		};

//Variadic functions not supported

		template <typename Ret, typename... Args>
		struct MangledName<Ret(*)(Args...)> : MangledName<Ret(Args...)> {};

#define HIENA_MEMFUNC(CONST, VOLATILE, REF_QUAL, NOEXCEPT)	\
				template <typename Ret, typename C, typename... Args>\
				struct MangledName<Ret(C::*)(Args...) CONST VOLATILE REF_QUAL NOEXCEPT> : MangledName<Ret(Args...)> {};

//Variadic member functions not supported
//#define HIENA_MEMFUNC_VARIADIC(CONST, VOLATILE, REF_QUAL, NOEXCEPT)	\
				template <typename Ret, typename C, typename... Args>\
				struct MangledName<Ret(C::*)(Args..., ...) CONST VOLATILE REF_QUAL NOEXCEPT> : MangledName<Ret(Args...)> {};

#include "Hiena/meta/MemFunVariants.hpp"

#undef HIENA_MEMFUNC
//#undef HIENA_MEMFUNC_VARIADIC

	}

	template <typename T>
	const char* GetJavaClassName()
	{
		return detail::GetJavaClassName<T>::Result.Content;
	}

	template <typename T>
	constexpr const char* GetMangledName(T&&)
	{
		return detail::MangledName<ValueType<T>>::Result.Content;
	}

	template <auto Func>
	constexpr const char* GetFuncName()
	{
		return detail::GetFuncName<Func>::Result.Content;
	}

	template <auto Func>
	constexpr const char* GetJavaClassFrom()
	{
		return detail::GetJavaClassFromFunc<Func>::Result.Content;
	}
}