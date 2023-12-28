#pragma once

#include <jni.h>

#include "Hiena/LowLevel.hpp"
#include "Hiena/CheckException.hpp"
#include "Hiena/detail/CheckedJniMacros.hpp"
#include "Hiena/meta/Helpers.hpp"
#include "Hiena/meta/Preprocessor.hpp"
#include "Hiena/utility/ScopeExit.hpp"

namespace hiena
{
    class CheckedJniEnv
    {
	private:
		JNIEnv* Env;
    public:
		CheckedJniEnv()
			:Env(LowLevelGetEnv(nullptr)) {}
        CheckedJniEnv(JNIEnv* InEnv)
        	:Env(LowLevelGetEnv(InEnv)) {}

        explicit operator bool() const  { return Env != nullptr; }
        explicit operator JNIEnv*() const { return Env; }
        CheckedJniEnv* operator->() { return this; }

        friend bool operator==(const CheckedJniEnv& Lhs, nullptr_t) { return Lhs.Env == nullptr; }
        friend bool operator==(nullptr_t, const CheckedJniEnv& Rhs ) { return Rhs.Env == nullptr; }

		//Wrapped methods
        HIENA_CHECKED_METHOD_NOARGS(jint, GetVersion);
    	HIENA_CHECKED_METHOD(jclass, DefineClass, (const char *, jobject, const jbyte*, jsize))
	    HIENA_CHECKED_METHOD(jclass, FindClass, (const char*))
        HIENA_CHECKED_METHOD(jmethodID, FromReflectedMethod, (jobject))
        HIENA_CHECKED_METHOD(jfieldID, FromReflectedField, (jobject))
        HIENA_CHECKED_METHOD(jobject, ToReflectedMethod, (jclass, jmethodID, jboolean))
        HIENA_CHECKED_METHOD(jclass, GetSuperclass, (jclass))
        HIENA_CHECKED_METHOD(jboolean, IsAssignableFrom, (jclass, jclass))
        HIENA_CHECKED_METHOD(jobject, ToReflectedField, (jclass, jfieldID, jboolean))
        HIENA_CHECKED_METHOD(jint, Throw, (jthrowable))
        HIENA_CHECKED_METHOD(jint, ThrowNew, (jclass, const char*))
        HIENA_UNCHECKED_METHOD_NOARGS(jthrowable, ExceptionOccurred)
        HIENA_UNCHECKED_METHOD_NOARGS(void, ExceptionDescribe)
        HIENA_UNCHECKED_METHOD_NOARGS(void, ExceptionClear)
        HIENA_CHECKED_METHOD(void, FatalError, (const char*))
        HIENA_CHECKED_METHOD(jint, PushLocalFrame, (jint))
        HIENA_CHECKED_METHOD(jobject, PopLocalFrame, (jobject))
        HIENA_CHECKED_METHOD(jobject, NewGlobalRef, (jobject))
        HIENA_CHECKED_METHOD(void, DeleteGlobalRef, (jobject))
        HIENA_CHECKED_METHOD(void, DeleteLocalRef, (jobject))
        HIENA_CHECKED_METHOD(jboolean, IsSameObject, (jobject, jobject))
        HIENA_CHECKED_METHOD(jobject, NewLocalRef, (jobject))
        HIENA_CHECKED_METHOD(jint, EnsureLocalCapacity, (jint))
        HIENA_CHECKED_METHOD(jobject, AllocObject, (jclass))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jobject, NewObject, (jclass, jmethodID))
        HIENA_CHECKED_METHOD(jclass, GetObjectClass, (jobject))
        HIENA_CHECKED_METHOD(jboolean, IsInstanceOf, (jobject, jclass))
        HIENA_CHECKED_METHOD(jmethodID, GetMethodID, (jclass, const char*, const char*))

        HIENA_CHECKED_VARIADIC_VOID_METHOD_SET(CallVoidMethod, (jobject, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jobject, CallObjectMethod, (jobject, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jboolean, CallBooleanMethod, (jobject, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jbyte, CallByteMethod, (jobject, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jchar, CallCharMethod, (jobject, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jshort, CallShortMethod, (jobject, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jint, CallIntMethod, (jobject, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jlong, CallLongMethod, (jobject, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jfloat, CallFloatMethod, (jobject, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jdouble, CallDoubleMethod, (jobject, jmethodID))

        HIENA_CHECKED_VARIADIC_VOID_METHOD_SET(CallNonvirtualVoidMethod, (jobject, jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jobject, CallNonvirtualObjectMethod, (jobject, jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jboolean, CallNonvirtualBooleanMethod, (jobject, jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jbyte, CallNonvirtualByteMethod, (jobject, jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jchar, CallNonvirtualCharMethod, (jobject, jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jshort, CallNonvirtualShortMethod, (jobject, jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jint, CallNonvirtualIntMethod, (jobject, jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jlong, CallNonvirtualLongMethod, (jobject, jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jfloat, CallNonvirtualFloatMethod, (jobject, jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jdouble, CallNonvirtualDoubleMethod, (jobject, jclass, jmethodID))

        HIENA_CHECKED_METHOD(jfieldID, GetFieldID, (jclass, const char*, const char*))
        HIENA_CHECKED_GETTERS(Get, Field, (jobject, jfieldID))
        HIENA_CHECKED_SETTERS(Set, Field, (jobject, jfieldID))

        HIENA_CHECKED_METHOD(jmethodID, GetStaticMethodID, (jclass, const char*, const char*))
        HIENA_CHECKED_VARIADIC_VOID_METHOD_SET(CallStaticVoidMethod, (jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jobject, CallStaticObjectMethod, (jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jboolean, CallStaticBooleanMethod, (jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jbyte, CallStaticByteMethod, (jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jchar, CallStaticCharMethod, (jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jshort, CallStaticShortMethod, (jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jint, CallStaticIntMethod, (jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jlong, CallStaticLongMethod, (jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jfloat, CallStaticFloatMethod, (jclass, jmethodID))
        HIENA_CHECKED_VARIADIC_METHOD_SET(jdouble, CallStaticDoubleMethod, (jclass, jmethodID))

        HIENA_CHECKED_METHOD(jfieldID, GetStaticFieldID, (jclass, const char*, const char*))
        HIENA_CHECKED_GETTERS(GetStatic, Field, (jclass, jfieldID))
        HIENA_CHECKED_SETTERS(SetStatic, Field, (jclass, jfieldID))

        HIENA_CHECKED_METHOD(jstring, NewString, (const jchar*, jsize))
        HIENA_CHECKED_METHOD(jsize, GetStringLength, (jstring))
        HIENA_CHECKED_METHOD(const jchar*, GetStringChars, (jstring, jboolean*))
        HIENA_CHECKED_METHOD(void, ReleaseStringChars, (jstring, const jchar*))
        HIENA_CHECKED_METHOD(jstring, NewStringUTF, (const char*))
        HIENA_CHECKED_METHOD(jsize, GetStringUTFLength, (jstring))
        HIENA_CHECKED_METHOD(const char*, GetStringUTFChars, (jstring, jboolean*))
        HIENA_CHECKED_METHOD(void, ReleaseStringUTFChars, (jstring, const char*))

        HIENA_CHECKED_METHOD(jsize, GetArrayLength, (jarray))
        HIENA_CHECKED_METHOD(jobjectArray, NewObjectArray, (jsize, jclass, jobject))
        HIENA_CHECKED_METHOD(jobject, GetObjectArrayElement, (jobjectArray, jsize))
        HIENA_CHECKED_METHOD(void, SetObjectArrayElement, (jobjectArray, jsize, jobject))

        HIENA_CHECKED_METHOD(jbooleanArray, NewBooleanArray, (jsize))
        HIENA_CHECKED_METHOD(jbyteArray, NewByteArray, (jsize))
        HIENA_CHECKED_METHOD(jcharArray, NewCharArray, (jsize))
        HIENA_CHECKED_METHOD(jshortArray, NewShortArray, (jsize))
        HIENA_CHECKED_METHOD(jintArray, NewIntArray, (jsize))
        HIENA_CHECKED_METHOD(jlongArray, NewLongArray, (jsize))
        HIENA_CHECKED_METHOD(jfloatArray, NewFloatArray, (jsize))
        HIENA_CHECKED_METHOD(jdoubleArray, NewDoubleArray, (jsize))

        HIENA_CHECKED_METHOD(jboolean*, GetBooleanArrayElements, (jbooleanArray, jboolean*))
        HIENA_CHECKED_METHOD(jbyte*, GetByteArrayElements, (jbyteArray, jboolean*))
        HIENA_CHECKED_METHOD(jchar*, GetCharArrayElements, (jcharArray, jboolean*))
        HIENA_CHECKED_METHOD(jshort*, GetShortArrayElements, (jshortArray, jboolean*))
        HIENA_CHECKED_METHOD(jint*, GetIntArrayElements, (jintArray, jboolean*))
        HIENA_CHECKED_METHOD(jlong*, GetLongArrayElements, (jlongArray, jboolean*))
        HIENA_CHECKED_METHOD(jfloat*, GetFloatArrayElements, (jfloatArray, jboolean*))
        HIENA_CHECKED_METHOD(jdouble*, GetDoubleArrayElements, (jdoubleArray, jboolean*))

        HIENA_CHECKED_METHOD(void, ReleaseBooleanArrayElements, (jbooleanArray, jboolean*, jint))
        HIENA_CHECKED_METHOD(void, ReleaseByteArrayElements, (jbyteArray, jbyte*, jint))
        HIENA_CHECKED_METHOD(void, ReleaseCharArrayElements, (jcharArray, jchar*, jint))
        HIENA_CHECKED_METHOD(void, ReleaseShortArrayElements, (jshortArray, jshort*, jint))
        HIENA_CHECKED_METHOD(void, ReleaseIntArrayElements, (jintArray, jint*, jint))
        HIENA_CHECKED_METHOD(void, ReleaseLongArrayElements, (jlongArray, jlong*, jint))
        HIENA_CHECKED_METHOD(void, ReleaseFloatArrayElements, (jfloatArray, jfloat*, jint))
        HIENA_CHECKED_METHOD(void, ReleaseDoubleArrayElements, (jdoubleArray, jdouble*, jint))

        HIENA_CHECKED_METHOD(void, GetBooleanArrayRegion, (jbooleanArray, jsize, jsize, jboolean*))
        HIENA_CHECKED_METHOD(void, GetByteArrayRegion, (jbyteArray, jsize, jsize, jbyte*))
        HIENA_CHECKED_METHOD(void, GetCharArrayRegion, (jcharArray, jsize, jsize, jchar*))
        HIENA_CHECKED_METHOD(void, GetShortArrayRegion, (jshortArray, jsize, jsize, jshort*))
        HIENA_CHECKED_METHOD(void, GetIntArrayRegion, (jintArray, jsize, jsize, jint*))
        HIENA_CHECKED_METHOD(void, GetLongArrayRegion, (jlongArray, jsize, jsize, jlong*))
        HIENA_CHECKED_METHOD(void, GetFloatArrayRegion, (jfloatArray, jsize, jsize, jfloat*))
        HIENA_CHECKED_METHOD(void, GetDoubleArrayRegion, (jdoubleArray, jsize, jsize, jdouble*))

        HIENA_CHECKED_METHOD(void, SetBooleanArrayRegion, (jbooleanArray, jsize, jsize, const jboolean*))
        HIENA_CHECKED_METHOD(void, SetByteArrayRegion, (jbyteArray, jsize, jsize, const jbyte*))
        HIENA_CHECKED_METHOD(void, SetCharArrayRegion, (jcharArray, jsize, jsize, const jchar*))
        HIENA_CHECKED_METHOD(void, SetShortArrayRegion, (jshortArray, jsize, jsize, const jshort*))
        HIENA_CHECKED_METHOD(void, SetIntArrayRegion, (jintArray, jsize, jsize, const jint*))
        HIENA_CHECKED_METHOD(void, SetLongArrayRegion, (jlongArray, jsize, jsize, const jlong*))
        HIENA_CHECKED_METHOD(void, SetFloatArrayRegion, (jfloatArray, jsize, jsize, const jfloat*))
        HIENA_CHECKED_METHOD(void, SetDoubleArrayRegion, (jdoubleArray, jsize, jsize, const jdouble*))

        HIENA_CHECKED_METHOD(jint, RegisterNatives, (jclass, const JNINativeMethod*, jint))
        HIENA_CHECKED_METHOD(jint, UnregisterNatives, (jclass))
        HIENA_CHECKED_METHOD(jint, MonitorEnter, (jobject))
        HIENA_CHECKED_METHOD(jint, MonitorExit, (jobject))
        HIENA_CHECKED_METHOD(jint, GetJavaVM, (JavaVM**))

        HIENA_CHECKED_METHOD(void, GetStringRegion, (jstring, jsize, jsize, jchar*))
        HIENA_CHECKED_METHOD(void, GetStringUTFRegion, (jstring, jsize, jsize, char*))

        HIENA_CHECKED_METHOD(void*, GetPrimitiveArrayCritical, (jarray, jboolean*))
        HIENA_CHECKED_METHOD(void, ReleasePrimitiveArrayCritical, (jarray, void*, jint))
        HIENA_CHECKED_METHOD(const jchar*, GetStringCritical, (jstring, jboolean*))
        HIENA_CHECKED_METHOD(void, ReleaseStringCritical, (jstring, const jchar*))

        HIENA_CHECKED_METHOD(jweak, NewWeakGlobalRef, (jobject))
        HIENA_CHECKED_METHOD(void, DeleteWeakGlobalRef, (jweak))
        HIENA_UNCHECKED_METHOD_NOARGS(jboolean, ExceptionCheck)
        HIENA_CHECKED_METHOD(jobject, NewDirectByteBuffer, (void*, jlong))
        HIENA_CHECKED_METHOD(void*, GetDirectBufferAddress, (jobject))
        HIENA_CHECKED_METHOD(jlong, GetDirectBufferCapacity, (jobject))
        HIENA_CHECKED_METHOD(jobjectRefType, GetObjectRefType, (jobject))
    };
}

#include "Hiena/detail/UndefCheckedJniMacros.hpp"
