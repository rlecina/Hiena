#pragma once

#include "Hiena/detail/JavaObjectBase.hpp"

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

		Class getClass();
		ClassLoader getClassLoader();
	};

	class Class : public Object
	{
	public:
		using Object::Object;
	};

	inline jclass ToArgument(const Class& Obj) { return (jclass)ToArgument((Object&)Obj); }

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

		String(const char* Text);
	};

}