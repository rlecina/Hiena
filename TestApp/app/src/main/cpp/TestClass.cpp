#include "TestClass.hpp"

#include <Hiena/utility/ImplementationMacros.hpp>

namespace com::testapp::example
{
	TestClass::TestClass(hiena::CreateNew_t)
		:TestClass(hiena::NewObject<TestClass>())
	{}

	TestClass::TestClass(java::lang::String Text)
		:TestClass(hiena::NewObject<TestClass>(Text))
	{}
}