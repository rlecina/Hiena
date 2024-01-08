#pragma once

#include <Hiena/JavaLang.hpp>

namespace com::testapp::example
{
	class TestClass  : public java::lang::Object
	{
	public:
		HIENA_CLASS_CONSTRUCTORS(TestClass, Object)

		TestClass(::hiena::CreateNew_t);
		TestClass(java::lang::String Text);

		HIENA_JAVA_FIELDS(
			::hiena::Field<int> A;
			::hiena::Field<float> B;
			::hiena::StaticField<java::lang::String> S;
		)
	};
}