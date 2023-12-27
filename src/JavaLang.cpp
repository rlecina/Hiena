#include "JavaLang.hpp"
#include "Hiena.hpp"
#include "JavaInvoker.hpp"

namespace java::lang
{
	Class Object::getClass(JNIEnv* Env)
	{
		// Don't invoke the Java method. This is faster and helps complete
		// initialization if needed
		return java::lang::Class(GetOrInitClassInternal(Env));
	}

	ClassLoader Object::getClassLoader()
	{
		return hiena::JavaInvoker<&Object::getClassLoader>::Invoke(this);
	}

	Class ClassLoader::findClass(String Classname)
	{
		return hiena::JavaInvoker<&ClassLoader::findClass>::Invoke(this, Classname);
	}

	String::String(const char* Text, JNIEnv* Env)
	: Object(hiena::GetEnv(Env)->NewStringUTF(Text), hiena::LocalOwnership)
	{
	}

	String::~String()
	{
		if (*this && Content)
		{
			JNIEnv* Env = hiena::GetEnv();
			Env->ReleaseStringUTFChars(ToJniArgument(*this, Env), Content);
		}
	}

	std::string_view String::ToCppString(JNIEnv* Env)
	{
		if (Content)
		{
			return Content;
		}
		else
		{
			Env = hiena::GetEnv(Env);
			Content = Env->GetStringUTFChars(ToJniArgument(*this, Env), nullptr);
		}
		return Content;
	}

	String Throwable::getMessage()
	{
		return hiena::JavaInvoker<&Throwable::getMessage>::Invoke(this);
	}
}