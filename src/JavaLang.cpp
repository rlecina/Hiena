#include "Hiena/JavaLang.hpp"
#include "Hiena/JavaInvoker.hpp"
#include "Hiena/utility/ImplementationMacros.hpp"

namespace java::lang
{
	Class Object::getClass(hiena::CheckedJniEnv Env)
	{
		// Don't invoke the Java method. This is faster and helps complete
		// initialization if needed
		return java::lang::Class(GetOrInitClassInternal(Env));
	}

	HIENA_IMPLEMENT_METHOD_NOARG(ClassLoader, Object::getClassLoader)

	HIENA_IMPLEMENT_METHOD(Class, ClassLoader::findClass, (const String&))

	String::String(const char* Text, hiena::CheckedJniEnv Env)
	: Object(Env->NewStringUTF(Text), hiena::LocalOwnership)
	{
	}

	std::string ToCppString(const String& Str, hiena::CheckedJniEnv Env)
	{
		jstring Instance = ToJniArgument(Str, Env);
		const char* Content = Env->GetStringUTFChars(Instance, nullptr);
		std::string Ret = Content;
		Env->ReleaseStringUTFChars(Instance, Content);
		return Ret;
	}

	HIENA_IMPLEMENT_STATIC_METHOD(String, String::valueOf, (const Object&))

	HIENA_IMPLEMENT_METHOD_NOARG(String, Throwable::getMessage)
}