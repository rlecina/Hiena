#pragma once

#include <string_view>

#include "Hiena/JArray.hpp"
#include "Hiena/utility/Macros.hpp"

namespace java::lang
{
	class Class;
	class ClassLoader;
	class Object;
	class String;
	class Throwable;

	class Object : public hiena::detail::JavaObjectBase
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(Object, JavaObjectBase, jobject)

		Class getClass(JNIEnv* Env = nullptr);
		ClassLoader getClassLoader();
	};

	class Class : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(Class, Object, jclass)
	};

	class ClassLoader : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(ClassLoader, Object, jobject)

		Class findClass(String Classname);
	};

	class String : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(String, Object, jstring)

		String(const char* Text, JNIEnv* Env = nullptr);
		~String();

		std::string_view ToCppString(JNIEnv* Env = nullptr);

	private:
		const char* Content = nullptr;
	};

	class Throwable : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(Throwable, Object, jthrowable)

		String 	getMessage();
	};
}