# The Hiena library

## The wild smiley dog that makes JNI easy

### Description

Hiena is a C++20 library wrapper to simplify replicating Java classes to be used through JNI

The idea is to allow the C++ preprocessor and compiler to generate bindings to Java classes by just providing simple C++ class definition counterparts to Java classes

### An example: 

Given the Java class

```Java
package com.example.testapp;

public class TestClass {
    String DoSomethingUseful(String Input) {
        //...
    }
}

```

You just need to provide the following class on C++ in a namespace equivalent to the Java package

TestClass.hpp:
```C++
#include "Hiena/JavaLang.hpp"

namespace com::example::testapp
{
    class TestClass : public java::lang::Object
    {
    public:
        TestClass() = default:
        TestClass(jobject Instance);

        java::lang::String DoSomethingUseful(const java::lang::String& Input)
    };
}
```

Test.cpp
```C++
#include "TestClass.hpp"

namespace com::example::testapp
{
    TestClass::TestClass(jobject Instance)
        :java::lang::Object(Instance)
    {}

    java::lang::String TestClass::DoSomethingUseful(const java::lang::String& Input)
    {
        // This helper function generates Java method, class name and mangling from the template argument
        // It also check arguments are passed in in a meaningful order 
        return hiena::JavaInvoker<&TestClass::DoSomethingUseful>::Invoke(this, Input);
    }
}
```

You can then create an instance of the C++ class passing in a jobject received from a Java native method and invoke Java methods direclty from C++ without dealing with any mangling

```C++
JNIEXPORT void JNICALL Java_com_example_testapp_SomeOtherClass_nativeMethod(JNIEnv*, jobject Thiz, jobject TestClassInstance)
{
    com::example::testapp::TestClass Instance(TestClassInstance);

    // Creates a new Java string
    java::lang::String Hello("Hello World!");

    // Invoke Java method
    java::lang::String Result = Instance.DoSomethinUseful(Hello);

    // ...
}
```

Hiena provides helper macros to reduce the C++ boilerplate on the class declaration/definition

TestClass.hpp:
```C++
#include "Hiena/JavaLang.hpp"

namespace com::example::testapp
{
    class TestClass : public java::lang::Object
    {
    public:
        HIENA_CLASS_CONSTRUCTORS(TestClass, Object)

        String DoSomethingUseful(const String& Input)
    };
}
```

TestClass.cpp:
```C++
#include "TestClass.hpp"
#include "Hiena/utility/ImplementationMacros.hpp"

namespace com::example::testapp
{
    HIENA_IMPLEMENT_METHOD(java::lang::String, TestClass::DoSomethinUseful, (const java::lang::String&))
}
```

### A more complex example

```Java
package com.hiena.example;

public class TestClass {
    public int A;
    public float B;
    public static String C = new String("This is static");

    TestClass() {
        //...
    }

    TestClass(String Text) {
        //...
    }

    String DoSomethingUseful(String Input) {
        //...
    }

    static String StaticDoSomethingElseUseful(String[] Input) {
        //...
    }

}
```

To support this class you should provide

TestClass.hpp:
```C++
#include "Hiena/JavaLang.hpp"

namespace com::example::testapp
{
    class TestClass : public java::lang::Object
    {
    public:
        HIENA_JAVA_FIELDS(
            hiena::Field<int> A;
            hiena::Field<float> B;
            hiena::StaticField<float> B;
        )

        HIENA_CLASS_CONSTRUCTORS(TestClass, Object, jobject)

        // This constructor is needed because default one means null instance
        TestClass(hiena::CreateNew_t); 
        
        TestClass(const java::lang::String& Text);

        String DoSomethingUseful(const String& Input)
        jint StaticDoSomethingElseUseful(String[] Input) {
    };
}
```

TestClass.cpp:
```C++
#include "TestClass.hpp"
#include "Hiena/utility/ImplementationMacros.hpp"

namespace com::example::testapp
{
    TestClass::TestClass(hiena::CreateNew_t)
        :TestClass(hiena::NewObject<TestClass>())
    {}

    TestClass::TestClass(const java::lang::String& Text)
        :TestClass(hiena::NewObject<TestClass>(Text))
    {}

    HIENA_IMPLEMENT_METHOD(String, TestClass::DoSomethinUseful, (const java::lang::String&))
    HIENA_IMPLEMENT_STATIC_METHOD(jint, TestClass::StaticDoSomethingElseUseful, (const hiena::JArray<java::lang::String>&))
}
```

Then we can use that C++ class

```C++
// Create a new instance using Java constructor without arguments
TestClass MyNewInstance(hiena::CreateNew);

// Access to a field
MyNewInstance->A = 42;

// Access to a static field
MyNewInstance->C = java::lang::String("I wrote this from C++");

java::lang::String Arg("Hellow World!");

java::lang::String Result = MyNewInstance.DoSomethingUseful(Arg);

// Create a new array and set its element
hiena::JArray<java::lang::string> JavaArray = hiena::NewObjectArray<java::lang::string>(1);
JavaArray.SetAt(0,Arg);

jint Result2 = MyNewInstance.StaticDoSomethingElseUseful(JavaArray);

```

### Current state

Hiena currently supports:
 - Invoking static methods
 - Invoking non static methods
 - Invoking non virtual methods
 - Access to primitive fields
 - Access to static primitive fields
 - Access to object type fields
 - Access to static object type fields
 - Arrays (both of promitive types and object types)
 - Local/Global reference ownership: By default C++ instances work as thin wrappers. 
   - There is a constructor that takes local reference ownership
   - There are methods to create new local an global references. Those references are released on the destructor

### Still TO-DO
 - Add class check on constructor with a jobject (jobject should represent a compatible object)
 - More complete set of Java types (currently just the bare minimum are added)
 - Add examples
 - Documentation
 - Add tests

### Final notes
Work is still incomplete.
The repo still needs examples and thorough testing, but current status is a good enough proof of concept. 
Most tests have been done using clang on Android with NDK 26.1.10909125
