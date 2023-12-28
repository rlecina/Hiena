#pragma once

#include <jni.h>
#include <type_traits>

#include "Hiena/CheckedJniEnv.hpp"
#include "Hiena/JArray.hpp"
#include "Hiena/LowLevel.hpp"
#include "Hiena/detail/JavaObjectBase.hpp"
#include "Hiena/detail/SignatureComposer.hpp"
#include "Hiena/meta/FuncSig.hpp"
#include "Hiena/meta/Helpers.hpp"
#include "Hiena/utility/JniTraits.hpp"

namespace hiena
{
	namespace detail
	{
	#define HIENA_INVOKE_BLOCK(TYPE, FUNC, ARG) \
				if constexpr (std::is_same_v<Ret, TYPE>) \
				{ \
					if constexpr (std::is_same_v<Ret, void>) \
					{ \
						va_list list; \
						va_start(list, MethodID); \
						Env->FUNC##MethodV(ARG, MethodID, list); \
						va_end(list); \
					} \
					else \
					{ \
						va_list list; \
						va_start(list, MethodID); \
						Ret R =  Env->FUNC##MethodV(ARG, MethodID, list); \
						va_end(list); \
						return R; \
					} \
				}

		template <typename Ret>
		struct InvokerDetail
		{
			static Ret Invoke(CheckedJniEnv Env, jobject Instance, jmethodID MethodID, ...)
			{
				HIENA_INVOKE_BLOCK(void, CallVoid, Instance)
				else HIENA_INVOKE_BLOCK(jboolean, CallBoolean, Instance)
				else HIENA_INVOKE_BLOCK(jbyte, CallByte, Instance)
				else HIENA_INVOKE_BLOCK(jchar, CallChar, Instance)
				else HIENA_INVOKE_BLOCK(jshort, CallShort, Instance)
				else HIENA_INVOKE_BLOCK(jint, CallInt, Instance)
				else HIENA_INVOKE_BLOCK(jlong, CallLong, Instance)
				else HIENA_INVOKE_BLOCK(jfloat, CallFloat, Instance)
				else HIENA_INVOKE_BLOCK(jdouble, CallDouble, Instance)
				else if constexpr (IsJniObjectType<Ret>)
				{
					va_list list;
					va_start(list, MethodID);
					jobject Object = Env->CallObjectMethodV(Instance, MethodID, list);
					va_end(list);
					return Ret((typename Ret::SourceJniType)Object, LocalOwnership);
				}
				else
				{
					static_assert(hiena::AlwaysFalse<Ret>, "Unsupported return type");
				}
			}
		};

		template <typename Ret>
		struct StaticInvokerDetail
		{
			static Ret Invoke(CheckedJniEnv Env, jclass Clazz, jmethodID MethodID, ...)
			{
				HIENA_INVOKE_BLOCK(void, CallStaticVoid, Clazz)
				else HIENA_INVOKE_BLOCK(jboolean, CallStaticBoolean, Clazz)
				else HIENA_INVOKE_BLOCK(jbyte, CallStaticByte, Clazz)
				else HIENA_INVOKE_BLOCK(jchar, CallStaticChar, Clazz)
				else HIENA_INVOKE_BLOCK(jshort, CallStaticShort, Clazz)
				else HIENA_INVOKE_BLOCK(jint, CallStaticInt, Clazz)
				else HIENA_INVOKE_BLOCK(jlong, CallStaticLong, Clazz)
				else HIENA_INVOKE_BLOCK(jfloat, CallStaticFloat, Clazz)
				else HIENA_INVOKE_BLOCK(jdouble, CallStaticDouble, Clazz)
				else if constexpr (IsJniObjectType<Ret>)
				{
					va_list list;
					va_start(list, MethodID);
					jobject Object = Env->CallStaticObjectMethodV(Clazz, MethodID, list);
					va_end(list);
					return Ret((typename Ret::SourceJniType)Object, LocalOwnership);
				}
				else
				{
					static_assert(hiena::AlwaysFalse<Ret>, "Unsupported return type");
				}
			}
		};
#undef HIENA_INVOKE_BLOCK
}
	template <typename T>
	std::enable_if_t<IsJniRegularPrimitiveType<T>,T> ToJniArgument(T Arg, CheckedJniEnv) { return Arg; }

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
			constexpr const char* FuncName = GetFuncName<Func>();
			constexpr const char* FuncMangledName = GetMangledName<FuncType>();
			CheckedJniEnv Env;
			static jmethodID MethodID = Env->GetMethodID(Instance->GetOrInitClassInternal(Env), FuncName, FuncMangledName);
			return InvokerDetail<Ret>::Invoke(Env, ToJniArgument(*Instance, Env), MethodID, ToJniArgument(Arg, Env)...);
		}

		template <typename... Args>
		static Ret StaticInvoke(Args&&... Arg)
		{
			static_assert(std::is_invocable_v<FuncType, Args...>, "Arguments in wrong order");
			using namespace detail;
			constexpr const char* ClassName = GetJavaClassFrom<Func>();
			constexpr const char* FuncName = GetFuncName<Func>();
			constexpr const char* FuncMangledName = GetMangledName<FuncType>();
			CheckedJniEnv Env;
			jclass Clazz = LowLevelFindClass(ClassName, (JNIEnv*)Env);
			static jmethodID MethodID = Env->GetStaticMethodID(Clazz, FuncName, FuncMangledName);
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
		constexpr const char* ClassName = GetJavaClassName<R>();
		constexpr const char* FuncName = "<init>";
		using FuncType = void(*)(Args&&...);
		constexpr const char* FuncMangledName = GetMangledName<FuncType>();
		CheckedJniEnv Env;
		jclass Clazz = LowLevelFindClass(ClassName, (JNIEnv*)Env);
		static jmethodID MethodID = Env->GetMethodID(Clazz, FuncName, FuncMangledName);
		jobject Ret = Env->NewObject(Clazz, MethodID, ToJniArgument(Arg, Env)...);
		Env->DeleteLocalRef(Clazz);
		return R(Ret, LocalOwnership);
	}

	template <typename Ret>
	Ret NewPrimitiveArray(jsize Size, CheckedJniEnv Env = nullptr)
	{
		using namespace detail;
		static_assert(IsJArrayType<Ret>, "Should be an array");
		static_assert(IsJniPrimitiveType<typename Ret::ValueType>, "Should be a primitive type");

		auto R = detail::PrimitiveArrayOps<typename Ret::ValueType>::NewArray(Size, Env);
		return Ret((typename Ret::SourceJniType)R, LocalOwnership);
	}

	template <typename Ret>
	Ret NewObjectArray(jsize Size, typename Ret::ValueType Init = {}, CheckedJniEnv Env = nullptr)
	{
		using namespace detail;
		static_assert(IsJArrayType<Ret>, "Should be an array");
		static_assert(!IsJniPrimitiveType<typename Ret::ValueType>, "Should be a primitive type");

		constexpr const char* ClassName = GetJavaClassName<typename Ret::ValueType>();
		jclass Clazz = LowLevelFindClass(ClassName, (JNIEnv*)Env);
		auto R = Env->NewObjectArray(Size, Clazz, ToJniArgument(Init, Env));
		Env->DeleteLocalRef(Clazz);
		return Ret((typename Ret::SourceJniType)R, LocalOwnership);
	}
}