#pragma once

#include <jni.h>

namespace hiena
{
	[[nodiscard]] JNIEnv* LowLevelGetEnv(JNIEnv* Env = nullptr);
	[[nodiscard]] jclass LowLevelFindClass(const char* ClassName, JNIEnv* Env);
}