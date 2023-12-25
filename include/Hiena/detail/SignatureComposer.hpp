#pragma once

#include <string_view>

#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/meta/Helpers.hpp"
#include "Hiena/utility/CompileTimeString.hpp"

namespace hiena
{
	template <typename T>
	struct HasJavaConversion
	{
		static constexpr bool Value = std::is_base_of_v<detail::JavaObjectBase, T> ||
		        std::is_same_v<void, T> ||
				std::is_same_v<jboolean, T> ||
				std::is_same_v<jbyte, T> ||
				std::is_same_v<jchar, T> ||
				std::is_same_v<jint, T> ||
				std::is_same_v<jshort, T> ||
				std::is_same_v<jlong, T> ||
				std::is_same_v<jfloat, T> ||
				std::is_same_v<jdouble, T> ||
				std::is_same_v<jshort, T> ||
				std::is_same_v<jshort, T>; /*
 		// How to define array? DoI need wrappers? (Probably yes...)
		//[ type 	type[]
		//
		//jobjectArray (object arrays)
		//jbooleanArray (boolean arrays)
		//jbyteArray (byte arrays)
		//jcharArray (char arrays)
		//jshortArray (short arrays)
		//jintArray (int arrays)
		//jlongArray (long arrays)
		//jfloatArray (float arrays)
		//jdoubleArray (double arrays)
 */
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
		constexpr int GetJavaClassLength(const CompileTimeString<N>& CppType)
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
		constexpr auto GetJavaClassName(const CompileTimeString<N>& CppType)
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
		static constexpr const char StarFuncTag[] = "::";
		static constexpr const char EndFuncTag[] = "]";
#elif defined(__GNUC__) || defined(__GNUG__)
		static constexpr const char StarFuncTag[] = "::";
		static constexpr const char EndFuncTag[] = "]";
#endif

		template <auto Func>
		consteval auto GetFuncName()
		{
			static_assert(IsFunctionPointer<decltype(Func)> || std::is_member_function_pointer_v<decltype(Func)>, "Unsupported type");
			constexpr std::string_view ThisFunction(__PRETTY_FUNCTION__); //Location.function_name());
			constexpr auto Start = FindTagExcluded(ThisFunction, StarFuncTag);
			constexpr auto End = FindTagIncluded(ThisFunction, EndFuncTag);
			return CompileTimeString<End - Start + 1>(ThisFunction.data() + Start, End - Start);
		}

#ifdef __clang__
		static constexpr const char StarFuncClassTag[] = "&";
		static constexpr const char EndFuncClassTag[] = "::";
#elif defined(__GNUC__) || defined(__GNUG__)
		static constexpr const char StarFuncClassTag[] = "&";
		static constexpr const char EndFuncClassTag[] = "::";
#endif

		template <auto Func>
		consteval auto GetJavaClassFromFunc()
		{
			static_assert(IsFunctionPointer<decltype(Func)> || std::is_member_function_pointer_v<decltype(Func)>, "Unsupported type");
			constexpr std::string_view ThisFunction(__PRETTY_FUNCTION__); //Location.function_name());
			constexpr auto Start = FindTagExcluded(ThisFunction, StarFuncClassTag);
			constexpr auto End = FindTagIncluded(ThisFunction, EndFuncClassTag);
			constexpr auto CppType = CompileTimeString<End - Start + 1>(ThisFunction.data() + Start, End - Start);
			constexpr auto MangledSize = GetJavaClassLength(CppType);
			return GetJavaClassName<MangledSize>(CppType);
		}

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

			static consteval auto GetJavaClass()
			{
				constexpr auto CppType = GetCppTypename();
				constexpr auto PathLikeSize = GetJavaClassLength(CppType);
				return GetJavaClassName<PathLikeSize>(CppType);
			}

			static consteval auto GetJavaMangledType()
			{
				return "L" + GetJavaClass() + ";";
			}
		};

		template <typename T>
		struct MangledName
		{
			static_assert(HasJavaConversion<ValueType<T>>::Value, "Type does not support Java conversion");
			static constexpr auto Result = Mangler<T>::GetJavaMangledType();
		};

		template <>
		struct MangledName<void>
		{
			static constexpr const char Result[] = "V";
		};

		template <>
		struct MangledName<jboolean>
		{
			static constexpr const char Result[] = "Z";
		};

		template <>
		struct MangledName<jbyte>
		{
			static constexpr const char Result[] = "B";
		};

		template <>
		struct MangledName<jchar>
		{
			static constexpr const char Result[] = "C";
		};

		template <>
		struct MangledName<jshort>
		{
			static constexpr const char Result[] = "S";
		};

		template <>
		struct MangledName<jint>
		{
			static constexpr const char Result[] = "I";
		};

		template <>
		struct MangledName<jlong>
		{
			static constexpr const char Result[] = "J";
		};

		template <>
		struct MangledName<jfloat>
		{
			static constexpr const char Result[] = "F";
		};

		template <>
		struct MangledName<jdouble>
		{
			static constexpr const char Result[] = "D";
		};

		// How to define array? DoI need wrappers? (Probably yes...)
		//[ type 	type[]
		//
		//jobjectArray (object arrays)
		//jbooleanArray (boolean arrays)
		//jbyteArray (byte arrays)
		//jcharArray (char arrays)
		//jshortArray (short arrays)
		//jintArray (int arrays)
		//jlongArray (long arrays)
		//jfloatArray (float arrays)
		//jdoubleArray (double arrays)


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
	const char* JavaClassName()
	{
		static_assert(HasJavaConversion<ValueType<T>>::Value, "Type does not support Java conversion");
		static constexpr auto Result = detail::Mangler<ValueType<T>>::GetJavaClass();
		return Result.c_str();
	}

	template <typename T>
	constexpr const char* Mangle(T&&)
	{
		return detail::MangledName<ValueType<T>>::Result.c_str();
	}

	template <auto Func>
	const char* FuncName()
	{
		static constexpr auto Result = detail::GetFuncName<Func>();
		return Result.c_str();
	}

	template <auto Func>
	const char* JavaClassFromFunc()
	{
		static constexpr auto Result = detail::GetJavaClassFromFunc<Func>();
		return Result.c_str();
	}
}