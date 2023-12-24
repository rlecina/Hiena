#pragma once

#include <type_traits>

namespace java::lang
{
	class Class;
	class ClassLoader;
	class Object;
	class String;

	class Object
	{
	public:
		ClassLoader* getClassLoader() { return {}; }
	};

	class Class : public Object
	{
	};

	class ClassLoader : public Object
	{
	public:
		Class* findClass(String* Classname) { return {}; }
	};

	class String : public Object
	{
	};

}