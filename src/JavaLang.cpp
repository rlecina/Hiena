#include "JavaLang.hpp"
#include "Hiena.hpp"
#include "Hiena/detail/JavaInvoker.hpp"

namespace java::lang
{
	Class Object::getClass()
	{
		return java::lang::Class(Clazz);
	}

	ClassLoader Object::getClassLoader() const
	{
/*		JNIEnv* Env = hiena::GetEnv();
		static jmethodID MethodID = hiena::GetMethodID<&Object::getClassLoader>(Env, Clazz);
		static_assert(std::is_same_v<ClassLoader, hiena::FuncSig_R<decltype(&Object::getClassLoader)>>, "Hola");
		return ClassLoader(Env->CallObjectMethod(Instance, MethodID));
*/
		return hiena::JavaInvoker<&Object::getClassLoader>::Invoke(Instance, Clazz);
	}

	Class ClassLoader::findClass(String Classname)
	{
/*		JNIEnv* Env = hiena::GetEnv();
		static jmethodID MethodID = hiena::GetMethodID<&ClassLoader::findClass>(Env, Clazz);
		return Class(Env->CallObjectMethod(Instance, MethodID, ToArgument(Classname)));
*/
		return hiena::JavaInvoker<&Object::getClass>::Invoke(Instance, Clazz, Classname);
	}

	String::String(const char* Text)
	: Object(hiena::GetEnv()->NewStringUTF(Text), false)
	{
	}
}