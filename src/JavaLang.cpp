#include "JavaLang.hpp"
#include "Hiena.hpp"
#include "JavaInvoker.hpp"

namespace java::lang
{
	Class Object::getClass()
	{
		// Don't invoke the Java method. This is faster and helps complete
		// initialization if needed
		return java::lang::Class(GetOrInitClass(*this));
	}

	ClassLoader Object::getClassLoader()
	{
		return hiena::JavaInvoker<&Object::getClassLoader>::Invoke(this);
	}

	Class ClassLoader::findClass(String Classname)
	{
		return hiena::JavaInvoker<&ClassLoader::findClass>::Invoke(this, Classname);
	}

	String::String(const char* Text)
	: Object(hiena::GetEnv()->NewStringUTF(Text), hiena::LocalOwnership)
	{
	}
}