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
			RefType = JavaRefType::OwningLocalRef;
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
		Clazz = std::exchange(Other.Clazz, nullptr);
		RefType = std::exchange(Other.RefType, JavaRefType::Ignored);
		return *this;
	}

	JavaObjectBase& JavaObjectBase::operator=(const JavaObjectBase& Other)
	{
		Release();
		if (Other.Instance)
		{
			switch (Other.RefType)
			{
				case JavaRefType::Ignored:
				{
					Instance = Other.Instance;
					Clazz = Other.Clazz;
					break;
				}
				case JavaRefType::OwningLocalRef:
				{
					JNIEnv* Env = GetEnv();
					Instance = Env->NewLocalRef(Other.Instance);
					Clazz = (jclass)Env->NewLocalRef(Other.Clazz);
					break;
				}
				case JavaRefType::OwningGlobalRef:
				{
					JNIEnv* Env = GetEnv();
					Instance = Env->NewGlobalRef(Other.Instance);
					Clazz = (jclass)Env->NewGlobalRef(Other.Clazz);
					break;
				}
			}
			RefType = Other.RefType;
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
		if(RefType == JavaRefType::OwningLocalRef)
		{
			Clazz = (jclass)Env->NewLocalRef(Clazz);
		}
		return Clazz;
	}

	void JavaObjectBase::Release()
	{
		if (!Instance)
		{
			return;
		}
		switch (RefType)
		{
			case JavaRefType::OwningLocalRef:
			{
				JNIEnv* Env = GetEnv();
				Env->DeleteLocalRef(Instance);
				Env->DeleteLocalRef(Clazz);
				break;
			}
			case JavaRefType::OwningGlobalRef:
			{
				JNIEnv* Env = GetEnv();
				Env->DeleteGlobalRef(Instance);
				Env->DeleteGlobalRef(Clazz);
				break;
			}
			default:
				break;
		}
		Instance = nullptr;
		Clazz = nullptr;
		RefType = JavaRefType::Ignored;
	}
}