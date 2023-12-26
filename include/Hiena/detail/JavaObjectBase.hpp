#pragma once

#include <jni.h>
#include <type_traits>

namespace hiena
{
	struct LocalOwnership_t {} inline constexpr LocalOwnership{};
}

namespace hiena::detail
{
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

		friend jobject ToArgument(const JavaObjectBase& Obj) { return Obj.Instance; }
		friend jclass GetOrInitClass(const JavaObjectBase& Obj, JNIEnv* Env = nullptr) { return Obj.GetOrInitClassInternal(Env); }

		template <typename T>
		friend T NewLocalRef(JNIEnv* Env, const T& Other)
		{
			static_assert(std::is_base_of_v<JavaObjectBase, T>, "Should be a java type");
			jobject Instance = Env->NewLocalRef(Other.Instance);
			return T(Instance, LocalOwnership);
		}

		template <typename T>
		friend T NewGlobalRef(JNIEnv* Env, const T& Other)
		{
			static_assert(std::is_base_of_v<JavaObjectBase, T>, "Should be a java type");
			T New;
			New.Instance = Env->NewGlobalRef(Other.Instance);
			// Class always set since global ref instances may be used in different threads
			New.Clazz = (jclass)Env->NewGlobalRef(Other.Clazz);
			New.RefType = JavaRefType::OwningGlobalRef;
			return New;
		}
	protected:
		jobject GetInstance() const { return Instance; }
	private:
		jobject Instance = nullptr;
		mutable jclass Clazz = nullptr;
		JavaRefType RefType = JavaRefType::Ignored;

		jclass GetOrInitClassInternal(JNIEnv* Env = nullptr) const;
		void Release();
	};
}