#pragma once

#include <jni.h>

#include "Hiena/CheckException.hpp"

namespace java::lang
{
	class Class;
}
namespace hiena
{
	struct Config
	{
		jint JniVersion = JNI_VERSION_1_6;
		ErrorHandlingConfig ErrorHandling;
	};

	[[nodiscard]] jint Initialize(JavaVM* Vm, const char* MainClass, Config InConfig);

	[[nodiscard]] JNIEnv* GetEnv(JNIEnv* Env = nullptr);
	[[nodiscard]] jclass LowLevelFindClass(const char* ClassName, JNIEnv* Env);
	[[nodiscard]] java::lang::Class FindClass(const char* ClassName, JNIEnv* Env);
}