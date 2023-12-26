#include "Hiena.hpp"

#include "Hiena/JavaLang.hpp"

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
	bool Initialize(JavaVM* Vm, const char* MainClass, jint JniVersion)
	{
		gVirtualMachine = Vm;
		gJniVersion = JniVersion;

		if (!EnvData::Initialize(&DetachCurrentThread))
		{
			return false;
		}

		JNIEnv *Env = GetEnv();

		if (!Env)
		{
			return false;
		}

		java::lang::Class ActivityClazz(Env->FindClass(MainClass));
		if (ActivityClazz == nullptr)
		{
			return false;
		}
		gClassLoader = ActivityClazz.getClassLoader();
		if (gClassLoader == nullptr)
		{
			return false;
		}
		gClassLoader = NewGlobalRef(Env, gClassLoader);

		return true;
	}

	JNIEnv* GetEnv(JNIEnv* Env)
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

	java::lang::Class FindClass(const char* ClassName, JNIEnv* Env)
	{
		Env = GetEnv(Env);

		if(java::lang::Class Clazz{Env->FindClass(ClassName)}; Clazz)
		{
			return Clazz;
		}
		// Need to think about how to report this
		if(Env->ExceptionCheck())
		{
			Env->ExceptionClear();
			java::lang::String Name(ClassName, Env);
			if (gClassLoader)
			{
				return gClassLoader.findClass(Name);
			}
		}
		return {};
	}
}