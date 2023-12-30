#The Hiena library

##That savage ugly dog that will make JNI easy with a smile on his face

###Description

Hiena is a C++ library wrapper to simplify JNI

The idea is to allow the C++ preprocessor and compiler to generate bindings to Java classes by just providing simple C++ class definition counterparts

An example: 

Given the Java class

```Java
package com.example.jnitest;

public class TestClass {
    public int A;
    public float B;

    TestClass() {
        //...
    }

    TestClass(String Text) {
        //...
    }

    String DoSomethingUseful(String Input) {
        //...
    }
}
```

We just need to provide the following class on C++

TestClass.hpp:
```C++
#include "Hiena/JavaLang.hpp"
#include "Hiena/utility/ClassMacros.hpp"

namespace com::example::jnitest
{
    class TestClass : public java::lang::Object
    {
    public:
        HIENA_JAVA_FIELDS(
            hiena::Field<int> A;
            hiena::Field<float> B;
        )

        HIENA_CLASS_CONSTRUCTORS(TestClass, Object, jobject)

        TestClass(hiena::CreateNew_t);
        TestClass(const java::lang::String& Text);

        String DoSomethingUseful(const String& Input)
    };
}
```

TestClass.cpp:
```C++
#include "TestClass.hpp"
#include "Hiena/utility/ImplementationMacros.hpp"

namespace com::example::jnitest
{
    TestClass::TestClass(hiena::CreateNew_t)
        :TestClass(hiena::NewObject<TestClass>())
    {}

    TestClass::TestClass(const java::lang::String& Text)
        :TestClass(hiena::NewObject<TestClass>(Text))
    {}

    HIENA_IMPLEMENT_METHOD(String, TestClass::DoSomethinUseful, (const String&))
}
```

Then we can use that C++ class

```C++
TestClass MyNewInstance(hiena::CreateNew);
MyNewInstance->A = 42;

java::lang::String Arg("Hellow World!");

java::lang::String Result = MyNewInstance.DoSomethingUseful(Arg);

```

###Current state

Hiena currently supports:
 - Invoking static methods
 - Invoking non static methods
 - Invoking non virtual methods
 - Access to primitive fields
 - Access to object type fields
 - Arrays
 - LocalRef, GlobalRef, just wrapper object distinction

Work is still incomplete. The repo still needs examples and thorough testing, but current status is a good enough proof of concept. Most tests have been done using clang on Android with NDK 26.1.10909125