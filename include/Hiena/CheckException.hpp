#pragma once

#include <jni.h>

namespace java::lang
{
	class Throwable;
};

namespace hiena
{
	enum class ErrorPolicy
	{
		AutoClear,
		AutoClearAndStore,
		Ignore,
		UseHandler
	};

	enum class HandlerResult
	{
		Clear,
		ClearAndStore,
		Ignore
	};

	using LogExceptionHandler = void (*)(const java::lang::Throwable&, JNIEnv*);
	using ExceptionHandler = HandlerResult (*)(const java::lang::Throwable&, JNIEnv*);

	extern LogExceptionHandler DefaultLogExceptionHandler;

	struct ErrorHandlingConfig
	{
		ErrorPolicy Policy = ErrorPolicy::AutoClear;
		LogExceptionHandler LogHandler = DefaultLogExceptionHandler;
		ExceptionHandler ExceptionHandler;
	};

	bool CheckExceptionFast();
	bool CheckException(JNIEnv* Env = nullptr);
	java::lang::Throwable GetLastErrorForThread(JNIEnv* Env = nullptr);
	void ClearLastErrorForThread();
}