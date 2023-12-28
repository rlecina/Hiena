#include "Hiena/detail/JavaObjectBase.hpp"

#include <utility>

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
		if (Other.Instance)
		{
			switch (Other.InstanceRefType)
			{
				case JavaRefType::Ignored:
				{
					Instance = Other.Instance;
					InstanceRefType = JavaRefType::Ignored;
					break;
				}
				case JavaRefType::OwningLocalRef:
				case JavaRefType::OwningGlobalRef:
				{
					Instance = CheckedJniEnv()->NewLocalRef(Other.Instance);
					InstanceRefType = JavaRefType::OwningLocalRef;
					break;
				}
			}
		}
		if (Other.Clazz)
		{
			switch (Other.ClazzRefType)
			{
				case JavaRefType::Ignored:
				{
					Clazz = Other.Clazz;
					ClazzRefType = JavaRefType::Ignored;
					break;
				}
				case JavaRefType::OwningLocalRef:
				case JavaRefType::OwningGlobalRef:
				{
					Clazz = (jclass)CheckedJniEnv()->NewLocalRef(Other.Clazz);
					ClazzRefType = JavaRefType::OwningLocalRef;
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

	jclass JavaObjectBase::GetOrInitClassInternal(CheckedJniEnv Env) const
	{
		if (Clazz)
		{
			return Clazz;
		}
		if (!Instance)
		{
			return nullptr;
		}

		Clazz = Env->GetObjectClass(Instance);
		if(Clazz && InstanceRefType == JavaRefType::OwningLocalRef)
		{
			Clazz = (jclass)Env->NewLocalRef(Clazz);
			ClazzRefType = JavaRefType::OwningLocalRef;
		}
		return Clazz;
	}

	void JavaObjectBase::Release()
	{
		if (Instance)
		{
			switch (InstanceRefType)
			{
				case JavaRefType::OwningLocalRef:
				{
					CheckedJniEnv()->DeleteLocalRef(Instance);
					break;
				}
				case JavaRefType::OwningGlobalRef:
				{
					CheckedJniEnv()->DeleteGlobalRef(Instance);
					break;
				}
				default:
					break;
			}
			Instance = nullptr;
			InstanceRefType = JavaRefType::Ignored;
		}
		if (Clazz)
		{
			switch (ClazzRefType)
			{
				case JavaRefType::OwningLocalRef:
				{
					CheckedJniEnv().DeleteLocalRef(Clazz);
					break;
				}
				case JavaRefType::OwningGlobalRef:
				{
					CheckedJniEnv().DeleteGlobalRef(Clazz);
					break;
				}
				default:
					break;
			}
			Clazz = nullptr;
			ClazzRefType = JavaRefType::Ignored;
		}
	}
}