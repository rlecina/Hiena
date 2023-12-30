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

	class Object : public hiena::detail::JavaObjectBase
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(Object, JavaObjectBase, jobject)

		Class getClass(hiena::CheckedJniEnv Env = {});
		ClassLoader getClassLoader();
	};

	class Class : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(Class, Object, jclass)

		friend jclass ToJniArgument(const Class& Obj, hiena::CheckedJniEnv)
		{
			return (jclass)Obj.GetInstance();
		}
	};

	class ClassLoader : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(ClassLoader, Object, jobject)

		Class findClass(const String& Classname);
	};

	class String : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(String, Object, jstring)

		explicit String(const char* Text, hiena::CheckedJniEnv Env = {});

		friend jstring ToJniArgument(const String& Obj, hiena::CheckedJniEnv)
		{
			return (jstring)Obj.GetInstance();
		}

		std::string ToCppString(hiena::CheckedJniEnv Env = {});

		static String valueOf(const Object& obj);
	};

	class Throwable : public Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(Throwable, Object, jthrowable)

		friend jthrowable ToJniArgument(const Throwable& Obj, hiena::CheckedJniEnv)
		{
			return (jthrowable)Obj.GetInstance();
		}

		String getMessage();
	};
}