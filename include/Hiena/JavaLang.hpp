#pragma once

#include "Hiena/JArray.hpp"

namespace java::lang
{
	class Class;
	class ClassLoader;
	class Object;
	class String;

	class Object : public hiena::detail::JavaObjectBase
	{
	public:
		using JavaObjectBase::JavaObjectBase;

		Class getClass(JNIEnv* Env = nullptr);
		ClassLoader getClassLoader();
	};

	class Class : public Object
	{
	public:
		using Object::Object;
		explicit Class(jclass Instance)
			:Object((jobject)Instance) {}
		explicit Class(jclass Instance, hiena::LocalOwnership_t Tag)
			:Object((jobject)Instance, Tag) {}

		friend jclass ToJniArgument(const Class& Obj, JNIEnv* Env) { return (jclass)ToJniArgument((Object&)Obj, Env); }
	};

	class ClassLoader : public Object
	{
	public:
		using Object::Object;

		Class findClass(String Classname);
	};

	class String : public Object
	{
	public:
		using Object::Object;
		explicit String(jstring Instance)
			:Object((jobject)Instance) {}
		explicit String(jstring Instance, hiena::LocalOwnership_t Tag)
			:Object((jobject)Instance, Tag) {}

		String(const char* Text, JNIEnv* Env = nullptr);

		friend jstring ToJniArgument(const String& Obj, JNIEnv* Env) { return (jstring)ToJniArgument((Object&)Obj, Env); }
	};

}