#pragma once

#include <string>

#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/Field.hpp"
#include "Hiena/JArray.hpp"
#include "Hiena/utility/ClassMacros.hpp"

namespace java::lang
{
	class Class;
	class ClassLoader;
	class Object;
	class String;
	class Throwable;

	class Object : public hiena::JavaObjectBase
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(Object, JavaObjectBase)

		Class getClass(hiena::CheckedJniEnv Env = {});
		ClassLoader getClassLoader();
	};

	class Class : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS_EX(Class, Object, jclass)

		friend jclass ToJniArgument(const Class& Obj, hiena::CheckedJniEnv)
		{
			return (jclass)Obj.GetInstance();
		}
	};

	class ClassLoader : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(ClassLoader, Object)

		Class findClass(const String& Classname);
	};

	class String : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS_EX(String, Object, jstring)

		explicit String(const char* Text, hiena::CheckedJniEnv Env = {});

		friend jstring ToJniArgument(const String& Obj, hiena::CheckedJniEnv)
		{
			return (jstring)Obj.GetInstance();
		}

		static String valueOf(const Object& obj);
	};

	std::string ToCppString(const String& Str, hiena::CheckedJniEnv Env = {});

	class Throwable : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS_EX(Throwable, Object, jthrowable)

		friend jthrowable ToJniArgument(const Throwable& Obj, hiena::CheckedJniEnv)
		{
			return (jthrowable)Obj.GetInstance();
		}

		String getMessage();
	};
}