#pragma once

#include <jni.h>
#include <ostream>

#include "Hiena/detail/FieldBase.hpp"

namespace hiena
{
	template <typename F>
	F FieldInitializer(detail::JavaObjectBase* Owner, F& Fields)
	{
		F InitializedFields;
		auto Tuple = ToTuple(InitializedFields);
		auto Names = GetFieldNames<F>();
		IndexedTupleFor(Tuple, [&](size_t Idx, auto& Field)
			{
				Field.Setup(Owner, Names[Idx]);
			});
		return InitializedFields;
	}

	template <typename T>
	class Field: public detail::FieldBase
	{
	 public:
		Field() = default;

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
			using JniType = decltype(ToJniArgument(std::declval<T>(), nullptr));
			Env = GetEnv(Env);
			jfieldID FieldID = GetFieldId(GetMangledName<T>(), Env);
			auto ret = detail::FieldOps<JniType>::GetField(GetOwner(), FieldID, Env);
			// CheckException
			return T(ret);
		}

		void Set(const T& Value, JNIEnv* Env = nullptr)
		{
			using JniType = decltype(ToJniArgument(std::declval<T>(), nullptr));
			Env = GetEnv(Env);
			jfieldID FieldID = GetFieldId(GetMangledName<T>(), Env);
			detail::FieldOps<JniType>::SetField(GetOwner(), FieldID, ToJniArgument(Value, Env), Env);
			// CheckException
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