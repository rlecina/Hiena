#pragma once

#include <jni.h>
#include <tuple>
#include <type_traits>
#include <utility>

namespace java::lang
{
	class Class;
	class ClassLoader;
	class Object;
	class String;

	class Object
	{
	public:
		Object() = default;
		explicit Object(jobject Instance, bool ShouldAddLocalRef = true);
		~Object();

		explicit Object(const Object& Other);
		explicit Object(Object&& Other);

		template <typename Other>
		Other& operator=(Other&& Rhs)
		{
			static_assert(std::is_base_of_v<Object,Other>, "Other should be child of Object");
			Release();
			if constexpr (std::is_rvalue_reference_v<Other>)
			{
				Instance = std::exchange(Rhs.Instance, nullptr);
				Clazz = std::exchange(Rhs.Clazz, nullptr);
				IsGlobalReference = std::exchange(Rhs.IsGlobalReference, false);
			}
			else
			{
				std::tie(Instance, Clazz) = Rhs.CloneFields();
				IsGlobalReference = Rhs.IsGlobalRef();
			}
			return static_cast<Other&>(*this);
		}

		void MakeGlobalRef();
		void ReleaseGlobalRef();
		bool IsGlobalRef() const { return IsGlobalReference; }

		// Some utilities
		friend bool operator==(nullptr_t, const Object& Rhs) { return Rhs.Instance == nullptr; }
		friend bool operator==(const Object& Rhs, nullptr_t) { return Rhs.Instance == nullptr; }
		explicit operator bool() const { return Instance != nullptr; }

		friend jobject ToArgument(Object& Obj) { return Obj.Instance; }

		// Java methods
		Class getClass();
		ClassLoader getClassLoader() const;

	protected:
		jobject Instance = nullptr;
		jclass Clazz = nullptr;
		bool IsGlobalReference = false;
	private:
		void Release();
		std::tuple<jobject,jclass> CloneFields() const;
	};

	class Class : public Object
	{
	public:
		using Object::Object;
	};

	class ClassLoader : public Object
	{
	public:
		using Object::Object;

		Class findClass(String Classname);
	};

	class String : public Object
	{
	public:
		using Object::Object;

		String(const char* Text);
	};

}