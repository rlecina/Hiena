#pragma once

#include <jni.h>
#include <tuple>
#include <type_traits>
#include <utility>

namespace hiena
{
	class JavaObjectBase
	{
	public:
		JavaObjectBase() = default;
		explicit JavaObjectBase(jobject Instance, bool ShouldAddLocalRef = true);
		explicit JavaObjectBase(const JavaObjectBase& Other);
		explicit JavaObjectBase(JavaObjectBase&& JavaObjectBase);
		JavaObjectBase& operator=(JavaObjectBase&& Rhs);
		JavaObjectBase& operator=(const JavaObjectBase& Rhs);
		~JavaObjectBase();

		void MakeGlobalRef();
		void ReleaseGlobalRef();
		bool IsGlobalRef() const { return IsGlobalReference; }

		// Some utilities
		friend bool operator==(nullptr_t, const JavaObjectBase& Rhs) { return Rhs.Instance == nullptr; }
		friend bool operator==(const JavaObjectBase& Rhs, nullptr_t) { return Rhs.Instance == nullptr; }
		explicit operator bool() const { return Instance != nullptr; }

		friend jobject ToArgument(JavaObjectBase& Obj) { return Obj.Instance; }

	protected:
		jobject Instance = nullptr;
		jclass Clazz = nullptr;
		bool IsGlobalReference = false;
	private:
		void Release();
		std::tuple<jobject,jclass> CloneFields() const;
	};
}