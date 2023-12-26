#pragma once

#include <jni.h>
#include <type_traits>
#include "Hiena/meta/FieldCounter.hpp"

namespace hiena
{
	struct LocalOwnership_t {} inline constexpr LocalOwnership{};

	template <auto>
	struct JavaInvoker;
}

namespace hiena::detail
{
	class FieldBase;

	enum class JavaRefType
	{
		Ignored,
		OwningLocalRef,
		OwningGlobalRef
	};

	class JavaObjectBase
	{
	public:
		JavaObjectBase() = default;
		explicit JavaObjectBase(jobject Instance);
		explicit JavaObjectBase(jobject Instance, LocalOwnership_t);
		explicit JavaObjectBase(const JavaObjectBase& Other);
		explicit JavaObjectBase(JavaObjectBase&& JavaObjectBase);
		JavaObjectBase& operator=(JavaObjectBase&& Rhs);
		JavaObjectBase& operator=(const JavaObjectBase& Rhs);
		~JavaObjectBase();

		// Some utilities
		friend bool operator==(nullptr_t, const JavaObjectBase& Rhs) { return Rhs.Instance == nullptr; }
		friend bool operator==(const JavaObjectBase& Lhs, nullptr_t) { return Lhs.Instance == nullptr; }
		explicit operator bool() const { return Instance != nullptr; }

		friend jobject ToJniArgument(const JavaObjectBase& Obj, JNIEnv*) { return Obj.Instance; }

		template <typename T>
		friend T NewLocalRef(JNIEnv* Env, const T& Other)
		{
			static_assert(std::is_base_of_v<JavaObjectBase, T>, "Should be a java type");
			using JniType = decltype(ToJniArgument(Other, Env));
			JniType Instance = (JniType)Env->NewLocalRef(Other.Instance);
			return T(Instance, LocalOwnership);
		}

		template <typename T>
		friend T NewGlobalRef(JNIEnv* Env, const T& Other)
		{
			static_assert(std::is_base_of_v<JavaObjectBase, T>, "Should be a java type");
			using JniType = decltype(ToJniArgument(Other, Env));
			T New((JniType)Env->NewGlobalRef(Other.Instance));
			// Class always set since global ref instances may be used in different threads
			New.Clazz = (jclass)Env->NewGlobalRef(Other.Clazz);
			New.RefType = JavaRefType::OwningGlobalRef;
			return New;
		}

	protected:
		template <auto>
		friend struct hiena::JavaInvoker;

		friend class FieldBase;

		jobject GetInstance() const { return Instance; }
		jclass GetOrInitClassInternal(JNIEnv* Env = nullptr) const;
	private:
		jobject Instance = nullptr;
		mutable jclass Clazz = nullptr;
		JavaRefType RefType = JavaRefType::Ignored;

		void Release();
	};
}