#include "JavaLang.hpp"
#include "Hiena.hpp"

namespace java::lang
{
	Object::Object(jobject Instance, bool ShouldAddLocalRef)
	: Instance(Instance)
	{
		if (Instance)
		{
			JNIEnv* Env = hiena::GetEnv();
			if (ShouldAddLocalRef)
			{
				Instance = Env->NewLocalRef(Instance);
			}
			Clazz = (jclass)Env->NewLocalRef(Env->GetObjectClass(Instance));
		}
	}

	Object::Object(const Object& Other)
	{
		if (Other.Instance)
		{
			std::tie(Instance, Clazz) = Other.CloneFields();
			IsGlobalReference = Other.IsGlobalReference;
		}
	}

	Object::Object(Object&& Other)
	: Instance(std::exchange(Other.Instance, nullptr))
	, Clazz(std::exchange(Other.Clazz, nullptr))
	, IsGlobalReference(std::exchange(Other.IsGlobalReference, false))
	{
	}

	Object::~Object()
	{
		Release();
	}

	void Object::MakeGlobalRef()
	{
		if (!IsGlobalReference && Instance)
		{
			jobject OldInstance = Instance;
			jclass OldClazz = Clazz;
			JNIEnv* Env = hiena::GetEnv();
			Instance = Env->NewGlobalRef(OldInstance);
			Clazz = (jclass)Env->NewGlobalRef(OldClazz);
			Env->DeleteLocalRef(OldInstance);
			Env->DeleteLocalRef(OldClazz);
			IsGlobalReference = true;
		}
	}

	void Object::ReleaseGlobalRef()
	{
		if (IsGlobalReference && Instance)
		{
			jobject OldInstance = Instance;
			jclass OldClazz = Clazz;
			JNIEnv* Env = hiena::GetEnv();
			Instance = Env->NewLocalRef(OldInstance);
			Clazz = (jclass)Env->NewLocalRef(OldClazz);
			Env->DeleteGlobalRef(OldInstance);
			Env->DeleteGlobalRef(OldClazz);
			IsGlobalReference = false;
		}
	}

	void Object::Release()
	{
		if (!Instance)
		{
			return;
		}
		JNIEnv* Env = hiena::GetEnv();
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

	std::tuple<jobject,jclass> Object::CloneFields() const
	{
		if (!Instance)
		{
			return {};
		}
		JNIEnv* Env = hiena::GetEnv();
		if (IsGlobalReference)
		{
			return std::tuple(Env->NewGlobalRef(Instance), (jclass)Env->NewGlobalRef(Clazz));
		}
		else
		{
			return std::tuple(Env->NewLocalRef(Instance), (jclass)Env->NewLocalRef(Clazz));
		}
	}

	Class Object::getClass()
	{
		return java::lang::Class(Clazz);
	}

	ClassLoader Object::getClassLoader() const
	{
		JNIEnv* Env = hiena::GetEnv();
		static jmethodID MethodID = hiena::GetMethodID<&Object::getClassLoader>(Env, Clazz);
		return ClassLoader(Env->CallObjectMethod(Instance, MethodID));
	}

	Class ClassLoader::findClass(String Classname)
	{
		JNIEnv* Env = hiena::GetEnv();
		static jmethodID MethodID = hiena::GetMethodID<&ClassLoader::findClass>(Env, Clazz);
		return Class(Env->CallObjectMethod(Instance, MethodID, ToArgument(Classname)));
	}

	String::String(const char* Text)
	: Object(hiena::GetEnv()->NewStringUTF(Text), false)
	{
	}
}