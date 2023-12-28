#include "Hiena/JavaLang.hpp"
#include "Hiena/JavaInvoker.hpp"

namespace java::lang
{
	Class Object::getClass(hiena::CheckedJniEnv Env)
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

	String::String(const char* Text, hiena::CheckedJniEnv Env)
	: Object(Env->NewStringUTF(Text), hiena::LocalOwnership)
	{
	}

	String::~String()
	{
		if (*this && Content)
		{
			hiena::CheckedJniEnv Env;
			Env->ReleaseStringUTFChars(ToJniArgument(*this, Env), Content);
		}
	}

	std::string_view String::ToCppString(hiena::CheckedJniEnv Env)
	{
		if (Content)
		{
			return Content;
		}
		else
		{
			Content = Env->GetStringUTFChars(ToJniArgument(*this, Env), nullptr);
		}
		return Content;
	}

	String Throwable::getMessage()
	{
		return hiena::JavaInvoker<&Throwable::getMessage>::Invoke(this);
	}
}