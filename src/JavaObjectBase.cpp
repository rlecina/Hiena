#include "Hiena/detail/JavaObjectBase.hpp"

#include "Hiena/Hiena.hpp"

namespace hiena
{
	JavaObjectBase::JavaObjectBase(jobject Instance, bool ShouldAddLocalRef)
	: Instance(Instance)
	{
		if (Instance)
		{
			JNIEnv* Env = GetEnv();
			if (ShouldAddLocalRef)
			{
				Instance = Env->NewLocalRef(Instance);
			}
			Clazz = (jclass)Env->NewLocalRef(Env->GetObjectClass(Instance));
		}
	}

	JavaObjectBase::JavaObjectBase(const JavaObjectBase& Other)
	{
		if (Other.Instance)
		{
			std::tie(Instance, Clazz) = Other.CloneFields();
			IsGlobalReference = Other.IsGlobalReference;
		}
	}

	JavaObjectBase::JavaObjectBase(JavaObjectBase&& Other)
	: Instance(std::exchange(Other.Instance, nullptr))
	, Clazz(std::exchange(Other.Clazz, nullptr))
	, IsGlobalReference(std::exchange(Other.IsGlobalReference, false))
	{
	}

	JavaObjectBase& JavaObjectBase::operator=(JavaObjectBase&& Rhs)
	{
		Release();
		Instance = std::exchange(Rhs.Instance, nullptr);
		Clazz = std::exchange(Rhs.Clazz, nullptr);
		IsGlobalReference = std::exchange(Rhs.IsGlobalReference, false);
		return *this;
	}

	JavaObjectBase& JavaObjectBase::operator=(const JavaObjectBase& Rhs)
	{
		Release();
		std::tie(Instance, Clazz) = Rhs.CloneFields();
		IsGlobalReference = Rhs.IsGlobalRef();
		return *this;
	}

	JavaObjectBase::~JavaObjectBase()
	{
		Release();
	}

	void JavaObjectBase::MakeGlobalRef()
	{
		if (!IsGlobalReference && Instance)
		{
			jobject OldInstance = Instance;
			jclass OldClazz = Clazz;
			JNIEnv* Env = GetEnv();
			Instance = Env->NewGlobalRef(OldInstance);
			Clazz = (jclass)Env->NewGlobalRef(OldClazz);
			Env->DeleteLocalRef(OldInstance);
			Env->DeleteLocalRef(OldClazz);
			IsGlobalReference = true;
		}
	}

	void JavaObjectBase::ReleaseGlobalRef()
	{
		if (IsGlobalReference && Instance)
		{
			jobject OldInstance = Instance;
			jclass OldClazz = Clazz;
			JNIEnv* Env = GetEnv();
			Instance = Env->NewLocalRef(OldInstance);
			Clazz = (jclass)Env->NewLocalRef(OldClazz);
			Env->DeleteGlobalRef(OldInstance);
			Env->DeleteGlobalRef(OldClazz);
			IsGlobalReference = false;
		}
	}

	void JavaObjectBase::Release()
	{
		if (!Instance)
		{
			return;
		}
		JNIEnv* Env = GetEnv();
		if (IsGlobalReference)
		{
			Env->DeleteGlobalRef(Instance);
			Env->DeleteGlobalRef(Clazz);
		}
		else
		{
			Env->DeleteLocalRef(Instance);
			Env->DeleteLocalRef(Clazz);
		}
		Instance = nullptr;
		Clazz = nullptr;
	}

	std::tuple<jobject,jclass> JavaObjectBase::CloneFields() const
	{
		if (!Instance)
		{
			return {};
		}
		JNIEnv* Env = GetEnv();
		if (IsGlobalReference)
		{
			return std::tuple(Env->NewGlobalRef(Instance), (jclass)Env->NewGlobalRef(Clazz));
		}
		else
		{
			return std::tuple(Env->NewLocalRef(Instance), (jclass)Env->NewLocalRef(Clazz));
		}
	}
}