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
}