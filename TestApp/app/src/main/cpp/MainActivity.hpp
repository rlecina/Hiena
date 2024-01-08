
#include <Hiena/JavaLang.hpp>

#include "TestClass.hpp"

namespace com::testapp::example
{
	class MainActivity : public java::lang::Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(MainActivity, Object)

		HIENA_JAVA_FIELDS(
			hiena::Field<TestClass> C;
		)

		void Toaster(jint Count, java::lang::String Text, hiena::JArray<java::lang::String> SArg, hiena::JArray<jshort> NArg);
	};
}