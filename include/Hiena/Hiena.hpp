#pragma once

#include <jni.h>

namespace hiena
{
	static constexpr jint JNI_VERSION = JNI_VERSION_1_6;
	[[nodiscard]] bool Initialize(JavaVM* Vm, const char* MainClass);

	[[nodiscard]] JNIEnv* GetEnv();
	[[nodiscard]] jclass FindClass(const char* ClassName);
}