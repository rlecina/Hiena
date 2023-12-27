#include "CheckException.hpp"
#include "Hiena.hpp"
#include "JavaLang.hpp"

namespace hiena
{
	LogExceptionHandler DefaultLogExceptionHandler = [](const java::lang::Throwable&, JNIEnv* Env)
		{
			Env->ExceptionDescribe();
		};

	namespace
	{
		ErrorHandlingConfig GlobalConfig = {};

		thread_local java::lang::Throwable ThreadException;
	}
	namespace detail
	{
		void SetupErrorHandling(hiena::ErrorHandlingConfig InConfig)
		{
			GlobalConfig = InConfig;
		}
	}

	bool CheckExceptionFast()
	{
		return (bool)ThreadException;
	}

	bool CheckException(JNIEnv* Env)
	{
		if (CheckExceptionFast())
		{
			return true;
		}
		
		Env = GetEnv(Env);
		if(Env->ExceptionCheck())
		{
			java::lang::Throwable Exception;
			if ( (GlobalConfig.LogHandler && GlobalConfig.LogHandler != DefaultLogExceptionHandler) ||
					GlobalConfig.Policy == ErrorPolicy::AutoClearAndStore ||
					GlobalConfig.Policy == ErrorPolicy::UseHandler)
			{
				Exception = java::lang::Throwable(Env->ExceptionOccurred(), LocalOwnership);
			}
			if ( GlobalConfig.LogHandler )
			{
				GlobalConfig.LogHandler(Exception, Env);
			}
			switch(GlobalConfig.Policy)
			{
				case ErrorPolicy::AutoClear:
					Env->ExceptionClear();
					return true;
				case ErrorPolicy::AutoClearAndStore:
					ThreadException = NewGlobalRef(Exception, Env);
					Env->ExceptionClear();
					return true;
				case ErrorPolicy::Ignore:
					return false;
				case ErrorPolicy::UseHandler:
				{
					if (GlobalConfig.ExceptionHandler)
					{
						switch(GlobalConfig.ExceptionHandler(Exception, Env))
						{
							case HandlerResult::Clear:
								Env->ExceptionClear();
								return true;
							case HandlerResult::ClearAndStore:
								ThreadException = NewGlobalRef(Exception, Env);
								Env->ExceptionClear();
								return true;
							case HandlerResult::Ignore:
								return false;
						}
					}
				}
			}
		}
		return false;
	}

	java::lang::Throwable GetLastErrorForThread(JNIEnv* Env)
	{
		return NewLocalRef(ThreadException, Env);
	}

	void ClearLastErrorForThread()
	{
		ThreadException = {};
	}

}