#pragma once

#include <jni.h>
#include <type_traits>

#include "Hiena/meta/FuncSig.hpp"
#include "Hiena/meta/Helpers.hpp"
#include "Hiena/utility/Helpers.hpp"

namespace hiena
{
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
			else if constexpr (std::is_base_of_v<::java::lang::Object, Ret>)
			{
				return Ret(Env->CallObjectMethodV(Instance, MethodID, list));
			}
			else
			{
				static_assert(hiena::AlwaysFalse<Ret>, "Unsupported return type");
			}
#undef HIENA_INVOKE_BLOCK
		}
	};

	template <auto Func>
	struct JavaInvoker
	{
		using FuncType = decltype(Func);
		using Ret = hiena::FuncSig_R<FuncType>;
		static_assert(std::is_same_v<Ret, hiena::ValueType<Ret>>, "Only value types supported as return type");

		template <typename... Args>
		static Ret Invoke(jobject Instance, jclass Clazz, Args&&... Arg)
		{
			JNIEnv* Env = hiena::GetEnv();
			static jmethodID MethodID = hiena::GetMethodID<Func>(Env, Clazz);
			return InvokerDetail<Ret>::Invoke(Env, Instance, MethodID, ToArgument(Arg)...);
		}
	};
}