#include "Hiena.hpp"

#include "Hiena/JavaLang.hpp"

#include "Hiena/LowLevel.hpp"
#include "Hiena/CheckedJniEnv.hpp"

#include <pthread.h>

namespace hiena
{
namespace
{
	JavaVM* gVirtualMachine = nullptr;
	jint gJniVersion = gJniVersion;
	java::lang::ClassLoader gClassLoader;

	struct EnvData
	{
	private:
		static constexpr uintptr_t AttachedMask = 0x1;
		static constexpr uintptr_t JniEnvMaskMask = ~AttachedMask;

		inline static pthread_key_t EnvDataKey;

	public:
		JNIEnv* Env = nullptr;
		bool Attached = false;

		static bool Initialize(void (*ThreadDestructor)(void*))
		{
			return pthread_key_create(&EnvDataKey, ThreadDestructor) == 0;
		}

		static EnvData CreateFromPointer(void* DataPtr)
		{
			uintptr_t Data = reinterpret_cast<uintptr_t>(DataPtr);
			return {.Env = reinterpret_cast<JNIEnv*>(Data & JniEnvMaskMask),
					.Attached = (Data & AttachedMask) != 0};
		}

		static EnvData CreateFromKey()
		{
			return CreateFromPointer(pthread_getspecific(EnvDataKey));
		}

		void StoreToKey()
		{
			uintptr_t Data = reinterpret_cast<uintptr_t>(Env) | (Attached? AttachedMask:0);
			pthread_setspecific(EnvDataKey,reinterpret_cast<void*>(Data));
		}
	};

	void DetachCurrentThread(void* Param)
	{
		auto Data = EnvData::CreateFromPointer(Param);
		if (Data.Attached)
		{
			gVirtualMachine->DetachCurrentThread();
		}
	}
}
	namespace detail
	{
		void SetupErrorHandling(ErrorHandlingConfig InConfig);
	}

	jint Initialize(JavaVM* Vm, Config InConfig)
	{
		gVirtualMachine = Vm;
		gJniVersion = InConfig.JniVersion;

		detail::SetupErrorHandling(InConfig.ErrorHandling);

		if (!EnvData::Initialize(&DetachCurrentThread))
		{
			return JNI_ERR;
		}

		CheckedJniEnv Env;

		if (!Env)
		{
			return JNI_ERR;
		}

		if (InConfig.MainClass)
		{
			java::lang::Class ActivityClazz(Env->FindClass(InConfig.MainClass));
			if (ActivityClazz == nullptr)
			{
				return JNI_ERR;
			}
			gClassLoader = ActivityClazz.getClassLoader();
			if (gClassLoader == nullptr)
			{
				return JNI_ERR;
			}
			gClassLoader = NewGlobalRef(gClassLoader, Env);
		}
		return gJniVersion;
	}

	JNIEnv* LowLevelGetEnv(JNIEnv* Env)
	{
		if (Env != nullptr)
		{
			return Env;
		}
		EnvData Data = EnvData::CreateFromKey();
		if (!Data.Env)
		{
			if (gVirtualMachine->GetEnv(reinterpret_cast<void**>(&Data.Env), gJniVersion) == JNI_EDETACHED)
			{
				if (gVirtualMachine->AttachCurrentThread(&Data.Env, nullptr) == JNI_OK)
				{
					Data.Attached = true;
					Data.StoreToKey();
				}
			}
		}
		return Data.Env;
	}

	jclass LowLevelFindClass(const char* ClassName, JNIEnv* Env)
	{
		if(jclass Clazz = Env->FindClass(ClassName))
		{
			return Clazz;
		}

		if(Env->ExceptionCheck())
		{
			if (gClassLoader)
			{
				Env->ExceptionClear();
				java::lang::String Name(ClassName, Env);
				java::lang::Class Clazz = gClassLoader.findClass(Name);
				return (jclass)Env->NewLocalRef(ToJniArgument(Clazz, Env));
			}
			CheckException(Env);
		}
		return nullptr;
	}

	java::lang::Class FindClass(const char* ClassName, CheckedJniEnv Env)
	{
		return java::lang::Class(LowLevelFindClass(ClassName, (JNIEnv*)Env), LocalOwnership);
	}
}