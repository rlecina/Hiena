#include "Hiena/detail/JavaObjectBase.hpp"

#include <utility>

#include "Hiena/Hiena.hpp"

namespace hiena::detail
{
	JavaObjectBase::JavaObjectBase(jobject Instance)
	: Instance(Instance)
	{
	}

	JavaObjectBase::JavaObjectBase(jobject Instance, LocalOwnership_t)
	: JavaObjectBase(Instance)
	{
		if (Instance)
		{
			InstanceRefType = JavaRefType::OwningLocalRef;
		}
	}

	JavaObjectBase::JavaObjectBase(const JavaObjectBase& Other)
	{
		*this = Other;
	}

	JavaObjectBase::JavaObjectBase(JavaObjectBase&& Other)
	{
		*this = std::move(Other);
	}

	JavaObjectBase& JavaObjectBase::operator=(JavaObjectBase&& Other)
	{
		Release();
		Instance = std::exchange(Other.Instance, nullptr);
		InstanceRefType = std::exchange(Other.InstanceRefType, JavaRefType::Ignored);
		Clazz = std::exchange(Other.Clazz, nullptr);
		ClazzRefType = std::exchange(Other.ClazzRefType, JavaRefType::Ignored);
		return *this;
	}

	JavaObjectBase& JavaObjectBase::operator=(const JavaObjectBase& Other)
	{
		Release();
		JNIEnv* Env = nullptr;
		if (Other.Instance)
		{
			switch (Other.InstanceRefType)
			{
				case JavaRefType::Ignored:
				{
					Instance = Other.Instance;
					break;
				}
				case JavaRefType::OwningLocalRef:
				{
					Env = GetEnv(Env);
					Instance = Env->NewLocalRef(Other.Instance);
					break;
				}
				case JavaRefType::OwningGlobalRef:
				{
					Env = GetEnv(Env);
					Instance = Env->NewGlobalRef(Other.Instance);
					break;
				}
			}
			InstanceRefType = Other.InstanceRefType;
		}
		if (Other.Clazz)
		{
			switch (Other.ClazzRefType)
			{
				case JavaRefType::Ignored:
				{
					Clazz = Other.Clazz;
					break;
				}
				case JavaRefType::OwningLocalRef:
				{
					Env = GetEnv(Env);
					Clazz = (jclass)Env->NewLocalRef(Other.Clazz);
					break;
				}
				case JavaRefType::OwningGlobalRef:
				{
					Env = GetEnv(Env);
					Clazz = (jclass)Env->NewGlobalRef(Other.Clazz);
					break;
				}
			}
			ClazzRefType = Other.ClazzRefType;
		}
		return *this;
	}

	JavaObjectBase::~JavaObjectBase()
	{
		Release();
	}

	jclass JavaObjectBase::GetOrInitClassInternal(JNIEnv* Env) const
	{
		if (Clazz)
		{
			return Clazz;
		}
		if (!Instance)
		{
			return nullptr;
		}

		Env = GetEnv(Env);

		Clazz = Env->GetObjectClass(Instance);
		if (CheckException(Env))
		{
			return nullptr;
		}
		if(InstanceRefType == JavaRefType::OwningLocalRef)
		{
			Clazz = (jclass)Env->NewLocalRef(Clazz);
			ClazzRefType = JavaRefType::OwningLocalRef;
		}
		return Clazz;
	}

	void JavaObjectBase::Release()
	{
		if (!Instance)
		{
			return;
		}
		JNIEnv* Env = nullptr;
		switch (InstanceRefType)
		{
			case JavaRefType::OwningLocalRef:
			{
				Env = GetEnv(Env);
				Env->DeleteLocalRef(Instance);
				break;
			}
			case JavaRefType::OwningGlobalRef:
			{
				Env = GetEnv(Env);
				Env->DeleteGlobalRef(Instance);
				break;
			}
			default:
				break;
		}
		switch (ClazzRefType)
		{
			case JavaRefType::OwningLocalRef:
			{
				Env = GetEnv(Env);
				Env->DeleteLocalRef(Clazz);
				break;
			}
			case JavaRefType::OwningGlobalRef:
			{
				Env = GetEnv(Env);
				Env->DeleteGlobalRef(Clazz);
				break;
			}
			default:
				break;
		}
		Instance = nullptr;
		InstanceRefType = JavaRefType::Ignored;
		Clazz = nullptr;
		ClazzRefType = JavaRefType::Ignored;
	}
}