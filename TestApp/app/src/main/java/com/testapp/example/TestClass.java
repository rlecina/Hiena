package com.testapp.example;

public class TestClass {
    public int A;
    public float B;

    static String S = new String("Hello from Java side");

    TestClass()
    {
        A = 1;
        B = 2;
    }

    TestClass(String Text)
    {
        A = 3;
        B = 4;
        S = Text;
    }
}