#pragma once

#include <jni.h>

namespace java::lang
{
	class Class;
}
namespace hiena
{
	[[nodiscard]] bool Initialize(JavaVM* Vm, const char* MainClass, jint JniVersion = JNI_VERSION_1_6);

	[[nodiscard]] JNIEnv* GetEnv(JNIEnv* Env = nullptr);
	[[nodiscard]] java::lang::Class FindClass(const char* ClassName, JNIEnv* Env);
}