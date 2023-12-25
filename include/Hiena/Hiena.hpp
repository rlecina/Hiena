#pragma once

#include <jni.h>

#include "JavaLang.hpp"
#include "SignatureComposer.hpp"

namespace hiena
{
	static constexpr jint JNI_VERSION = JNI_VERSION_1_6;
	[[nodiscard]] bool Initialize(JavaVM* Vm, const char* MainClass);

	[[nodiscard]] JNIEnv* GetEnv();
	[[nodiscard]] java::lang::Class FindClass(const char* ClassName);

	template <auto Func>
	jmethodID GetMethodID(jclass Class)
	{
		return GetMethodId<Func>(GetEnv(), Class);
	}

	template <auto Func>
	jmethodID GetMethodID(JNIEnv* Env, jclass Class)
	{
		return Env->GetMethodID(Class, FuncName<Func>(), Mangle(Func));
	}
}