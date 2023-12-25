#pragma once

#include <jni.h>
#include <type_traits>

#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/detail/SignatureComposer.hpp"
#include "Hiena/meta/FuncSig.hpp"
#include "Hiena/meta/Helpers.hpp"
#include "Hiena/utility/Helpers.hpp"

namespace hiena
{
	namespace detail
	{
		inline void CheckException(JNIEnv* Env)
		{
			// Need to think about how to report this to caller
			if(Env->ExceptionCheck())
			{
				Env->ExceptionDescribe();
				Env->ExceptionClear();
			}
		}

		template <typename Ret>
		struct InvokerDetail
		{
			static Ret Invoke(JNIEnv* Env, jobject Instance, jmethodID MethodID, ...)
			{
				va_list list;
				va_start(list, MethodID);
				ScopeExit OnExit([&]{va_end(list);});

	#define HIENA_INVOKE_BLOCK(Type, Func) \
				if constexpr (std::is_same_v<Ret, void>) \
				{ \
                    ScopeExit OnCheckOnExit([&] { CheckException(Env); }); \
					return Env->Call##Func##MethodV(Instance, MethodID, list); \
				}

				HIENA_INVOKE_BLOCK(void, Void)
				else HIENA_INVOKE_BLOCK(jboolean, Boolean)
				else HIENA_INVOKE_BLOCK(jbyte, Byte)
				else HIENA_INVOKE_BLOCK(jchar, Char)
				else HIENA_INVOKE_BLOCK(jshort, Short)
				else HIENA_INVOKE_BLOCK(jint, Int)
				else HIENA_INVOKE_BLOCK(jlong, Long)
				else HIENA_INVOKE_BLOCK(jfloat, Float)
				else HIENA_INVOKE_BLOCK(jdouble, Double)
				else if constexpr (std::is_base_of_v<detail::JavaObjectBase, Ret>)
				{
					jobject Object = Env->CallObjectMethodV(Instance, MethodID, list);
					CheckException(Env);
					return Ret(Object);
				}
				else
				{
					static_assert(hiena::AlwaysFalse<Ret>, "Unsupported return type");
				}
	#undef HIENA_INVOKE_BLOCK
			}
		};

		template <typename Ret>
		struct StaticInvokerDetail
		{
			static Ret Invoke(JNIEnv* Env, jclass Clazz, jmethodID MethodID, ...)
			{
				va_list list;
				va_start(list, MethodID);
				ScopeExit OnExit([&]{va_end(list);});

	#define HIENA_INVOKE_BLOCK(Type, Func) \
					if constexpr (std::is_same_v<Ret, void>) \
					{ \
						ScopeExit OnCheckOnExit([&] { CheckException(Env); }); \
						return Env->CallStatic##Func##MethodV(Clazz, MethodID, list); \
					}

				HIENA_INVOKE_BLOCK(void, Void)
				else HIENA_INVOKE_BLOCK(jboolean, Boolean)
				else HIENA_INVOKE_BLOCK(jbyte, Byte)
				else HIENA_INVOKE_BLOCK(jchar, Char)
				else HIENA_INVOKE_BLOCK(jshort, Short)
				else HIENA_INVOKE_BLOCK(jint, Int)
				else HIENA_INVOKE_BLOCK(jlong, Long)
				else HIENA_INVOKE_BLOCK(jfloat, Float)
				else HIENA_INVOKE_BLOCK(jdouble, Double)
				else if constexpr (std::is_base_of_v<detail::JavaObjectBase, Ret>)
				{
					jobject Object = Env->CallStaticObjectMethodV(Clazz, MethodID, list);
					CheckException(Env);
					return Ret(Object);
				}
				else
				{
					static_assert(hiena::AlwaysFalse<Ret>, "Unsupported return type");
				}
	#undef HIENA_INVOKE_BLOCK
			}
		};
}

	template <auto Func>
	struct JavaInvoker
	{
		using FuncType = decltype(Func);
		using Ret = hiena::FuncSig_R<FuncType>;
		static_assert(std::is_same_v<Ret, hiena::ValueType<Ret>>, "Only value types supported as return type");

		template <typename... Args>
		static Ret Invoke(detail::JavaObjectBase* Instance, Args&&... Arg)
		{
			JNIEnv* Env = hiena::GetEnv();
			constexpr const char* FuncName = GetFuncName<Func>();
			constexpr const char* FuncMangledName = GetMangledName(Func);
			static jmethodID MethodID = Env->GetMethodID(GetOrInitClass(*Instance, Env), FuncName, FuncMangledName);
			detail::CheckException(Env);
			return detail::InvokerDetail<Ret>::Invoke(Env, ToArgument(*Instance), MethodID, ToArgument(Arg)...);
		}

		template <typename... Args>
		static Ret StaticInvoke(Args&&... Arg)
		{
			JNIEnv* Env = hiena::GetEnv();

			constexpr const char* ClassName = GetJavaClassFrom<Func>();
			constexpr const char* FuncName = GetFuncName<Func>();
			constexpr const char* FuncMangledName = GetMangledName(Func);
			java::lang::Class Clazz = FindClass(ClassName, Env);
			static jmethodID MethodID = Env->GetStaticMethodID(ToArgument(Clazz), FuncName, FuncMangledName);
			detail::CheckException(Env);
			return detail::StaticInvokerDetail<Ret>::Invoke(Env, ToArgument(Clazz), MethodID, ToArgument(Arg)...);
		}
	};
}