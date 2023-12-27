#pragma once

#include <jni.h>
#include <ostream>

#include "Hiena/detail/FieldBase.hpp"
#include "Hiena/utility/JniTraits.hpp"

namespace hiena
{
	template <typename T, typename F>
	F InitFields(T* Owner, F& Fields)
	{
		static_assert(IsJniObjectType<T>, "Invalid Field owner");
		auto Tuple = ToTuple(Fields);
		auto Names = GetFieldNames<F>();
		IndexedTupleFor(Tuple, [&](size_t Idx, auto& Field)
			{
				Field.Setup(Owner, GetJavaClassName<T>(), Names[Idx]);
			});
		return {};
	}

	template <typename T>
	class Field: public detail::FieldBase
	{
	 public:
		Field& operator=(const T& Value)
		{
			Set(Value);
			return *this;
		}

		Field& operator=(T&& Value)
		{
			Set(std::move(Value));
			return *this;
		}

		operator T() const
		{
			return Get();
		}

		T Get(JNIEnv* Env = nullptr)
		{
			using namespace detail;
			using JniType = LessSpecializedJniType<decltype(ToJniArgument(std::declval<T>(), nullptr))>;
			jfieldID FieldID = GetFieldId(GetMangledName<T>(), Env);
			if (CheckExceptionFast())
			{
				return  T{};
			}
			auto ret = FieldOps<JniType>::GetField(GetOwner(), FieldID, Env);
			if (CheckException(Env))
			{
				return  T{};
			}
			if constexpr(IsJniObjectType<T>)
			{
				return T((typename T::SourceJniType)ret, LocalOwnership);
			}
			else
			{
				return T(ret);
			}
		}

		void Set(const T& Value, JNIEnv* Env = nullptr)
		{
			using namespace detail;
			using JniType = LessSpecializedJniType<decltype(ToJniArgument(std::declval<T>(), nullptr))>;
			Env = GetEnv(Env);
			jfieldID FieldID = GetFieldId(GetMangledName<T>(), Env);
			if (CheckExceptionFast())
			{
				return;
			}
			FieldOps<JniType>::SetField(GetOwner(), FieldID, ToJniArgument(Value, Env), Env);
			CheckException(Env);
		}

		void Set(T&& Value, JNIEnv* Env = nullptr)
		{
			Set((const T&)Value, Env);
			if constexpr (IsJniObjectType<T>)
			{
				Value = {};
			}
		}

		friend std::ostream& operator <<(std::ostream& os, Field<T>& F)
		{
			return (os << F.Get());
		}
	};


	template <typename T>
	class StaticField: public detail::FieldBase
	{
	public:
		StaticField& operator=(const T& Value)
		{
			Set(Value);
			return *this;
		}

		StaticField& operator=(T&& Value)
		{
			Set(std::move(Value));
			return *this;
		}

		operator T() const
		{
			return Get();
		}

		T Get(JNIEnv* Env = nullptr)
		{
			using namespace detail;
			using JniType = LessSpecializedJniType<decltype(ToJniArgument(std::declval<T>(), nullptr))>;
			Env = GetEnv(Env);
			jclass Clazz = GetStaticOwnerClass(Env);
			jfieldID FieldID = GetStaticFieldId(Clazz, GetMangledName<T>(), Env);
			auto ret = FieldOps<JniType>::GetStaticField(Clazz, FieldID, Env);
			ReleaseStaticOwnerClass(Clazz, Env);
			if constexpr(IsJniObjectType<T>)
			{
				return T((typename T::SourceJniType)ret, LocalOwnership);
			}
			else
			{
				return T(ret);
			}
		}

		void Set(const T& Value, JNIEnv* Env = nullptr)
		{
			using namespace detail;
			using JniType = LessSpecializedJniType<decltype(ToJniArgument(std::declval<T>(), nullptr))>;
			Env = GetEnv(Env);
			jclass Clazz = GetStaticOwnerClass(Env);
			jfieldID FieldID = GetStaticFieldId(Clazz, GetMangledName<T>(), Env);
			FieldOps<JniType>::SetStaticField(Clazz, FieldID, ToJniArgument(Value, Env), Env);
			ReleaseStaticOwnerClass(Clazz, Env);
		}

		void Set(T&& Value, JNIEnv* Env = nullptr)
		{
			Set((const T&)Value, Env);
			if constexpr (IsJniObjectType<T>)
			{
				Value = {};
			}
		}

		friend std::ostream& operator <<(std::ostream& os, Field<T>& F)
		{
			return (os << F.Get());
		}
	};
}