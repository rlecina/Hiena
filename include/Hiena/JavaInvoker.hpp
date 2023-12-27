#pragma once

#include <jni.h>
#include <type_traits>

#include "Hiena/JArray.hpp"
#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/detail/SignatureComposer.hpp"
#include "Hiena/meta/FuncSig.hpp"
#include "Hiena/meta/Helpers.hpp"
#include "Hiena/utility/ScopeExit.hpp"
#include "Hiena/utility/JniTraits.hpp"

namespace hiena
{
	namespace detail
	{
		inline void CheckException(JNIEnv* Env)
		{
			// Need to think about how to report this to caller
			if(Env->ExceptionCheck())
			{
				Env->ExceptionDescribe();
				Env->ExceptionClear();
			}
		}

		template <typename Ret>
		struct InvokerDetail
		{
			static Ret Invoke(JNIEnv* Env, jobject Instance, jmethodID MethodID, ...)
			{
				va_list list;
				va_start(list, MethodID);
				ScopeExit OnExit([&]{va_end(list);});

	#define HIENA_INVOKE_BLOCK(Type, Func) \
				if constexpr (std::is_same_v<Ret, void>) \
				{ \
                    ScopeExit OnCheckOnExit([&] { CheckException(Env); }); \
					return Env->Call##Func##MethodV(Instance, MethodID, list); \
				}

				HIENA_INVOKE_BLOCK(void, Void)
				else HIENA_INVOKE_BLOCK(jboolean, Boolean)
				else HIENA_INVOKE_BLOCK(jbyte, Byte)
				else HIENA_INVOKE_BLOCK(jchar, Char)
				else HIENA_INVOKE_BLOCK(jshort, Short)
				else HIENA_INVOKE_BLOCK(jint, Int)
				else HIENA_INVOKE_BLOCK(jlong, Long)
				else HIENA_INVOKE_BLOCK(jfloat, Float)
				else HIENA_INVOKE_BLOCK(jdouble, Double)
				else if constexpr (IsJniObjectType<Ret>)
				{
					jobject Object = Env->CallObjectMethodV(Instance, MethodID, list);
					CheckException(Env);
					return Ret((typename Ret::SourceJniType)Object, LocalOwnership);
				}
				else
				{
					static_assert(hiena::AlwaysFalse<Ret>, "Unsupported return type");
				}
	#undef HIENA_INVOKE_BLOCK
			}
		};

		template <typename Ret>
		struct StaticInvokerDetail
		{
			static Ret Invoke(JNIEnv* Env, jclass Clazz, jmethodID MethodID, ...)
			{
				va_list list;
				va_start(list, MethodID);
				ScopeExit OnExit([&]{va_end(list);});

	#define HIENA_INVOKE_BLOCK(Type, Func) \
					if constexpr (std::is_same_v<Ret, void>) \
					{ \
						ScopeExit OnCheckOnExit([&] { CheckException(Env); }); \
						return Env->CallStatic##Func##MethodV(Clazz, MethodID, list); \
					}

				HIENA_INVOKE_BLOCK(void, Void)
				else HIENA_INVOKE_BLOCK(jboolean, Boolean)
				else HIENA_INVOKE_BLOCK(jbyte, Byte)
				else HIENA_INVOKE_BLOCK(jchar, Char)
				else HIENA_INVOKE_BLOCK(jshort, Short)
				else HIENA_INVOKE_BLOCK(jint, Int)
				else HIENA_INVOKE_BLOCK(jlong, Long)
				else HIENA_INVOKE_BLOCK(jfloat, Float)
				else HIENA_INVOKE_BLOCK(jdouble, Double)
				else if constexpr (IsJniObjectType<Ret>)
				{
					jobject Object = Env->CallStaticObjectMethodV(Clazz, MethodID, list);
					CheckException(Env);
					return Ret((typename Ret::SourceJniType)Object, LocalOwnership);
				}
				else
				{
					static_assert(hiena::AlwaysFalse<Ret>, "Unsupported return type");
				}
	#undef HIENA_INVOKE_BLOCK
			}
		};
}
	template <typename T>
	std::enable_if_t<IsJniRegularPrimitiveType<T>,T> ToJniArgument(T Arg, JNIEnv*) { return Arg; }

	template <auto Func>
	struct JavaInvoker
	{
		using FuncType = decltype(Func);
		using Ret = hiena::FuncSig_R<FuncType>;
		static_assert(std::is_same_v<Ret, hiena::ValueType<Ret>>, "Only value types supported as return type");

		template <typename T, typename... Args>
		static Ret Invoke(T* Instance, Args&&... Arg)
		{
			static_assert(std::is_invocable_v<FuncType, T*, Args...>, "Arguments in wrong order");
			using namespace detail;
			JNIEnv* Env = hiena::GetEnv();
			constexpr const char* FuncName = GetFuncName<Func>();
			constexpr const char* FuncMangledName = GetMangledName<FuncType>();
			static jmethodID MethodID = Env->GetMethodID(Instance->GetOrInitClassInternal(Env), FuncName, FuncMangledName);
			CheckException(Env);
			return InvokerDetail<Ret>::Invoke(Env, ToJniArgument(*Instance, Env), MethodID, ToJniArgument(Arg, Env)...);
		}

		template <typename... Args>
		static Ret StaticInvoke(Args&&... Arg)
		{
			static_assert(std::is_invocable_v<FuncType, Args...>, "Arguments in wrong order");
			using namespace detail;
			JNIEnv* Env = hiena::GetEnv();
			constexpr const char* ClassName = GetJavaClassFrom<Func>();
			constexpr const char* FuncName = GetFuncName<Func>();
			constexpr const char* FuncMangledName = GetMangledName<FuncType>();
			jclass Clazz = LowLevelFindClass(ClassName, Env);
			static jmethodID MethodID = Env->GetStaticMethodID(Clazz, FuncName, FuncMangledName);
			CheckException(Env);
			Ret R = StaticInvokerDetail<Ret>::Invoke(Env, Clazz, MethodID, ToJniArgument(Arg, Env)...);
			Env->DeleteLocalRef(Clazz);
			return R;
		}
	};

	template <typename R, typename... Args>
	R NewObject(Args&&... Arg)
	{
		static_assert(IsJniObjectType<R>, "Should be an object representation");
		static_assert(std::is_same_v<R, hiena::ValueType<R>>, "Only value types supported as return type");

		using namespace detail;
		JNIEnv* Env = hiena::GetEnv();
		constexpr const char* ClassName = GetJavaClassName<R>();
		constexpr const char* FuncName = "<init>";
		using FuncType = void(*)(Args&&...);
		constexpr const char* FuncMangledName = GetMangledName<FuncType>();
		jclass Clazz = LowLevelFindClass(ClassName, Env);
		static jmethodID MethodID = Env->GetMethodID(Clazz, FuncName, FuncMangledName);
		CheckException(Env);
		jobject Ret = Env->NewObject(Clazz, MethodID, ToJniArgument(Arg, Env)...);
		CheckException(Env);
		Env->DeleteLocalRef(Clazz);
		return R(Ret, LocalOwnership);
	}

	template <typename Ret>
	Ret NewPrimitiveArray(jsize Size, JNIEnv* Env = nullptr)
	{
		using namespace detail;
		static_assert(IsJArrayType<Ret>, "Should be an array");
		static_assert(IsJniPrimitiveType<typename Ret::ValueType>, "Should be a primitive type");

		Env = hiena::GetEnv();
		auto R = detail::PrimitiveArrayOps<typename Ret::ValueType>::NewArray(Size, Env);
		CheckException(Env);
		return Ret((typename Ret::SourceJniType)R, LocalOwnership);
	}

	template <typename Ret>
	Ret NewObjectArray(jsize Size, typename Ret::ValueType Init = {}, JNIEnv* Env = nullptr)
	{
		using namespace detail;
		static_assert(IsJArrayType<Ret>, "Should be an array");
		static_assert(!IsJniPrimitiveType<typename Ret::ValueType>, "Should be a primitive type");

		Env = hiena::GetEnv();
		constexpr const char* ClassName = GetJavaClassName<typename Ret::ValueType>();
		jclass Clazz = LowLevelFindClass(ClassName, Env);
		auto R = Env->NewObjectArray(Size, Clazz, ToJniArgument(Init, Env));
		CheckException(Env);
		Env->DeleteLocalRef(Clazz);
		return Ret((typename Ret::SourceJniType)R, LocalOwnership);
	}
}