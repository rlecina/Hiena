#include "Hiena.hpp"

#include <pthread.h>

#include <chrono>
#include <thread>

namespace hiena
{
namespace
{
	JavaVM* gVirtualMachine = nullptr;
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
	bool Initialize(JavaVM* Vm, const char* MainClass)
	{
		gVirtualMachine = Vm;

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
		gClassLoader.MakeGlobalRef();

		std::thread([]()
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(5s);
			auto c1 = FindClass("com/example/jnitestbed/MainActivity");
			std::this_thread::sleep_for(5s);
			auto c2 = FindClass("com/example/jnitestbed/2MainActivity");
			std::this_thread::sleep_for(5s);
			auto c3 = FindClass("com/example/jnitestbed/MainActivity");
		}).detach();
		return true;
	}

	JNIEnv* GetEnv()
	{
		EnvData Data = EnvData::CreateFromKey();
		if (!Data.Env)
		{
			if (gVirtualMachine->GetEnv(reinterpret_cast<void**>(&Data.Env), JNI_VERSION) == JNI_EDETACHED)
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

	java::lang::Class FindClass(const char* ClassName)
	{
		auto Env = GetEnv();
		java::lang::Class Clazz;
		java::lang::String Name(ClassName);
		if (gClassLoader)
		{
			Clazz = gClassLoader.findClass(ClassName);
		}
		else
		{
			Clazz = java::lang::Class(Env->FindClass(ClassName));
		}
		if(Env->ExceptionCheck())
		{
			Env->ExceptionDescribe();
			Env->ExceptionClear();
		}
		return Clazz;
	}
}