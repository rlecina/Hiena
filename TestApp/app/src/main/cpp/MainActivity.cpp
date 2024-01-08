#include "MainActivity.hpp"

#include "AndroidOut.hpp"

#include <Hiena/utility/ImplementationMacros.hpp>

namespace com::testapp::example
{
	HIENA_IMPLEMENT_METHOD(void, MainActivity::Toaster, (jint, java::lang::String, hiena::JArray<java::lang::String>, hiena::JArray<jshort>))
}

extern "C" JNIEXPORT void JNICALL Java_com_testapp_example_MainActivity_nativeMethod(JNIEnv* InEnv, jobject Thiz)
{
	using namespace std::literals::string_literals;

	hiena::CheckedJniEnv Env(InEnv);
	auto SArr = hiena::NewObjectArray<java::lang::String>(42, java::lang::String("Hello C++ World!"), Env);
	auto NArr = hiena::NewPrimitiveArray<jshort>(42);
	for (jsize i = 0; i < 42; i++)
	{
		if(i%2)
			SArr.SetAt(i , java::lang::String(("Hello from C++ number "s + std::to_string(i)).c_str()), Env);
		NArr.SetAt(i, i, Env);
	}
	com::testapp::example::MainActivity Act(Thiz);

	com::testapp::example::TestClass Test0;
	com::testapp::example::TestClass Test(hiena::CreateNew);
	Test->A = 42;
	Test->B = 96.f;
	Test->S = java::lang::String("Hello from C++ side");

	Test0 = Test;

	Act->C = Test;

	aout << "Updated!: " << *Test->A << ", " << *Test->B << ", " << ToCppString(Test->S, Env) << std::endl;

	Act.Toaster(42, java::lang::String::valueOf(Test), SArr, NArr);

	aout << "Updated!: " << *Test->A << ", " << *Test->B << ", " << ToCppString(Test->S, Env) << std::endl;
}