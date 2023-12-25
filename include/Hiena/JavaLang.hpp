#pragma once

#include "Hiena/detail/JavaObjectBase.hpp"

namespace java::lang
{
	class Class;
	class ClassLoader;
	class Object;
	class String;

class Object : public hiena::JavaObjectBase
	{
	public:
		using JavaObjectBase::JavaObjectBase;

		Class getClass();
		ClassLoader getClassLoader() const;
	};

	class Class : public Object
	{
	public:
		using Object::Object;
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

		String(const char* Text);
	};

}