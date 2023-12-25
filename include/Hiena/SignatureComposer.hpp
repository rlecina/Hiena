#pragma once

#include <string_view>

#include "CompileTimeString.hpp"
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

		template <int Length>
		static consteval auto FindTagExcluded(std::string_view ThisFunction, const char (&Tag)[Length])
		{
			return ThisFunction.rfind(Tag) + Length - 1;
		}

		static consteval auto FindTagIncluded(std::string_view ThisFunction, const char* Tag)
		{
			return ThisFunction.rfind(Tag);
		}

		template <typename>
		struct Mangler;

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

#ifdef __clang__
		static constexpr const char StarFuncTag[] = "::";
		static constexpr const char EndFuncTag[] = "]";
#elif defined(__GNUC__) || defined(__GNUG__)
		static constexpr const char StarFuncTag[] = "::";
		static constexpr const char EndFuncTag[] = "]";
#endif

		template <auto Func>
		consteval auto getFuncName()
		{
			static_assert(std::is_member_function_pointer_v<decltype(Func)>, "Unsupported type");
			constexpr std::string_view ThisFunction(__PRETTY_FUNCTION__); //Location.function_name());
			constexpr auto Start = FindTagExcluded(ThisFunction, StarFuncTag);
			constexpr auto End = FindTagIncluded(ThisFunction, EndFuncTag);
			return CompileTimeString<End - Start + 1>(ThisFunction.data() + Start, End - Start);
		}

		template <typename T>
		using ValueType = std::remove_pointer_t<std::remove_cvref_t<T>>;

		template <typename T>
		struct MangledName
		{
			static_assert(HasJavaConversion<ValueType<T>>::Value, "Type does not support Java conversion");
			static constexpr auto Result = Mangler<T>::GetJavaMangledType();
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

#define MANGLED_NAME_FOR_CLASS_MEMBER(CONST, VOLATILE, REF_QUAL, NOEXCEPT)	\
				template <typename Ret, typename C, typename... Args>\
				struct MangledName<Ret(C::*)(Args...) CONST VOLATILE REF_QUAL NOEXCEPT> : public MangledName<Ret(Args...)> {};

		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	/*volatile*/,	,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		/*volatile*/,	,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	volatile,		,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		volatile,		,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	/*volatile*/,	&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		/*volatile*/,	&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	volatile,		&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		volatile,		&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	/*volatile*/,	&&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		/*volatile*/,	&&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	volatile,		&&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		volatile,		&&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	/*volatile*/,	,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		/*volatile*/,	,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	volatile,		,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		volatile,		,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	/*volatile*/,	&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		/*volatile*/,	&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	volatile,		&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		volatile,		&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	/*volatile*/,	&&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		/*volatile*/,	&&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( /*const*/,	volatile,		&&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER( const,		volatile,		&&,	noexcept)

#undef MANGLED_NAME_FOR_CLASS_MEMBER
#define MANGLED_NAME_FOR_CLASS_MEMBER_VAR(CONST, VOLATILE, REF_QUAL, NOEXCEPT)	\
				template <typename Ret, typename C, typename... Args>\
				struct MangledName<Ret(C::*)(Args..., ...) CONST VOLATILE REF_QUAL NOEXCEPT> : public MangledName<Ret(Args...)> {};

		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	/*volatile*/,	,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		/*volatile*/,	,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	volatile,		,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		volatile,		,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	/*volatile*/,	&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		/*volatile*/,	&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	volatile,		&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		volatile,		&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	/*volatile*/,	&&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		/*volatile*/,	&&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	volatile,		&&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		volatile,		&&,	/*noexcept*/)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	/*volatile*/,	,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		/*volatile*/,	,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	volatile,		,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		volatile,		,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	/*volatile*/,	&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		/*volatile*/,	&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	volatile,		&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		volatile,		&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	/*volatile*/,	&&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		/*volatile*/,	&&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( /*const*/,	volatile,		&&,	noexcept)
		MANGLED_NAME_FOR_CLASS_MEMBER_VAR( const,		volatile,		&&,	noexcept)
#undef MANGLED_NAME_FOR_CLASS_MEMBER_VAR
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
	const char* Mangle(T&&)
	{
		return detail::MangledName<detail::ValueType<T>>::Result.c_str();
	};

	template <auto Func>
	const char* FuncName()
	{
		static constexpr auto Result = detail::getFuncName<Func>();
		return Result.c_str();
	};
}