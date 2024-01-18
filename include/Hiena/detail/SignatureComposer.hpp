#pragma once

#include "Hiena/JArray.hpp"
#include "Hiena/meta/Helpers.hpp"
#include "Hiena/utility/CompileTimeString.hpp"
#include "Hiena/utility/JniTraits.hpp"

namespace hiena
{
	namespace detail
	{
		template <auto CppType>
		static consteval auto JavaJniClassFromCppType()
		{
			// Translate namespaced C++ types into path-like Java type names
			using namespace CompileTime;
			return ReplaceAll<CppType, "::"_cs, "/"_cs>();
		}

		template <auto Func>
		struct GetFuncNameImpl
		{
			static_assert(IsFunctionPointer<decltype(Func)> || std::is_member_function_pointer_v<decltype(Func)>, "Unsupported type");

			// Expected __PRETTY_FUNCTION__ for function myFunc in class MyClass in comments
#ifdef __clang__
			// static auto hiena::detail::GetFuncNameImpl<&MyClass::myFunc>::Get() [Func = &MyClass::myFunc]
			//                                                                                     ^^      ^
			static constexpr auto StartTag = CompileTimeString("::");
			static constexpr auto EndTag = CompileTimeString("]");
#elif defined(__GNUC__) || defined(__GNUG__)
			// static consteval auto hiena::detail::GetFuncNameImpl<Func>::Get() [with auto Func = &MyClass::myFunc]
			//                                                                                             ^^      ^
			static constexpr auto StartTag = CompileTimeString("::");
			static constexpr auto EndTag = CompileTimeString("]");
#endif
			static consteval auto Get()
			{
				return CompileTime::GetLastBetweenTags<CompileTimeString(__PRETTY_FUNCTION__), StartTag, EndTag>();
			}

			static constexpr auto Result = Get();
		};

		template <auto Func>
		struct GetJavaClassFromFuncImpl
		{
			static_assert(IsFunctionPointer<decltype(Func)> || std::is_member_function_pointer_v<decltype(Func)>, "Unsupported type");

			//Expected __PRETTY_FUNCTION__ for function myFunc in class MyClass in comments
#ifdef __clang__
			// static auto hiena::detail::GetJavaClassFromFuncImpl<&MyClass::myFunc>::Get() [Func = &MyClass::myFunc]
			//                                                                                      ^       ^^
			static constexpr auto StartTag = CompileTimeString("&");
			static constexpr auto  EndTag = CompileTimeString("::");
#elif defined(__GNUC__) || defined(__GNUG__)
			// static consteval auto hiena::detail::GetJavaClassFromFuncImpl<Func>::Get() [with auto Func = &MyClass::myFunc]
			//                                                                                              ^       ^^
			static constexpr auto StartTag = CompileTimeString("&");
			static constexpr auto EndTag = CompileTimeString("::");
#endif
			static consteval auto Get()
			{
				constexpr auto CppType = CompileTime::GetLastBetweenTags<CompileTimeString(__PRETTY_FUNCTION__), StartTag, EndTag>();
				return JavaJniClassFromCppType<CppType>();
			}
			static constexpr auto Result = Get();
		};

		template <typename Object>
		struct Mangler
		{
			// Expected __PRETTY_FUNCTION__ for class MyClass in comments
#ifdef __clang__
			// static auto hiena::detail::Mangler<MyClass>::GetJavaClassNameImpl() [Object = MyClass]
			//                                                                     ^^^^^^^^^^       ^
			static constexpr auto StartTag = CompileTimeString("[Object = ");
			static constexpr auto EndTag = CompileTimeString("]");
#elif defined(__GNUC__) || defined(__GNUG__)
			// static consteval auto hiena::detail::Mangler<Object>::GetJavaClassNameImpl() [with Object = MyClass]"
			//                                                                              ^^^^^^^^^^^^^^^       ^
			static constexpr auto StartTag = CompileTimeString("[with Object = ");
			static constexpr auto EndTag = CompileTimeString("]");
#endif

			static consteval auto GetJavaClassNameImpl()
			{
				constexpr auto CppType = CompileTime::GetLastBetweenTags<CompileTimeString(__PRETTY_FUNCTION__), StartTag, EndTag>();
				return JavaJniClassFromCppType<CppType>();
			}

			static consteval auto GetJavaMangledType()
			{
				return "L" + GetJavaClassNameImpl() + ";";
			}
		};

		template <typename T>
		struct GetJavaClassNameImpl
		{
			static_assert(HasJniConversion<ValueType<T>>::Value, "Type does not support Java conversion");
			static constexpr auto Result = Mangler<ValueType<T>>::GetJavaClassNameImpl();
		};

		template <typename T>
		struct MangledName
		{
			static_assert(HasJniConversion<ValueType<T>>::Value, "Type does not support Java conversion");
			static constexpr auto Result = Mangler<T>::GetJavaMangledType();
		};

#define HIENA_PRIMITIVE_MANGLED_NAME(TYPE, MANGLED) \
        template <> \
		struct MangledName<TYPE> { static constexpr auto Result = CompileTimeString(MANGLED); };

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
			static_assert(HasJniConversion<ValueType<Ret>>::Value, "Type does not support Java conversion");
			static_assert((HasJniConversion<ValueType<Args>>::Value && ... && true), "Type does not support Java conversion");
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
	constexpr const char* GetJavaClassName()
	{
		return detail::GetJavaClassNameImpl<T>::Result.c_str();
	}

	template <typename T>
	constexpr const char* GetMangledName()
	{
		return detail::MangledName<ValueType<T>>::Result.c_str();
	}

	template <auto Func>
	constexpr const char* GetFuncName()
	{
		return detail::GetFuncNameImpl<Func>::Result.c_str();
	}

	template <auto Func>
	constexpr const char* GetJavaClassFrom()
	{
		return detail::GetJavaClassFromFuncImpl<Func>::Result.c_str();
	}
}