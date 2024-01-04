#pragma once

#include <jni.h>

#include "Hiena/CheckedJniEnv.hpp"
#include "Hiena/CheckException.hpp"

namespace java::lang
{
	class Class;
}
namespace hiena
{
	class CheckedJniEnv;

	struct Config
	{
		const char* MainClass = nullptr;
		jint JniVersion = JNI_VERSION_1_6;
		ErrorHandlingConfig ErrorHandling;
	};

	[[nodiscard]] jint Initialize(JavaVM* Vm, Config InConfig);
	[[nodiscard]] java::lang::Class FindClass(const char* ClassName, CheckedJniEnv Env);
}