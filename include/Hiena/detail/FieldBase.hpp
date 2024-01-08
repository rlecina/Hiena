#pragma once

#include "Hiena/CheckedJniEnv.hpp"
#include "Hiena/LowLevel.hpp"
#include "Hiena/detail/JavaObjectBase.hpp"

namespace hiena::detail
{
	class FieldBase
	{
	public:
		FieldBase(){}
		FieldBase(const FieldBase& Other) {}
		FieldBase(FieldBase&& Other) { }
		FieldBase& operator=(const FieldBase& Other)
		{
			this->TryCopyFieldId(Other);
			return *this;
		}
		FieldBase& operator=(FieldBase&& Other)
		{
			this->TryCopyFieldId(Other);
			return *this;
		}

		void Setup(JavaObjectBase* InOwner, const char* InOwnerClassName, const char* InName)
		{
			Owner = InOwner;
			OwnerClassName = InOwnerClassName;
			FieldName = InName;
		}

	protected:
		jobject GetOwner() const
		{
			return Owner->GetInstance();
		}

		jclass GetStaticOwnerClass(CheckedJniEnv Env) const
		{
			if (Owner)
			{
				return Owner->GetOrInitClassInternal(Env);
			}
			return LowLevelFindClass(OwnerClassName, (JNIEnv*)Env);
		}

		void ReleaseStaticOwnerClass(jclass Clazz, CheckedJniEnv Env) const
		{
			if (!Owner)
			{
				Env->DeleteLocalRef(Clazz);
			}
		}


		jfieldID GetFieldId(const char* Signature, CheckedJniEnv Env) const
		{
			if (FieldId != 0)
			{
				return FieldId;
			}
			jclass Clazz = Owner->GetOrInitClassInternal(Env);
			return FieldId = Env->GetFieldID(Clazz, FieldName, Signature);
		}

		jfieldID GetStaticFieldId(jclass Clazz, const char* Signature, CheckedJniEnv Env) const
		{
			if (FieldId != 0)
			{
				return FieldId;
			}
			return Env->GetStaticFieldID(Clazz, FieldName, Signature);
		}

		void TryCopyFieldId(const FieldBase& Other)
		{
			if(FieldId == 0 && Other.FieldId != 0)
			{
				if (std::string_view(FieldName) == std::string_view(Other.FieldName) )
				{
					jclass OwnerClass = Owner->GetClassInternal();
					jclass OtherClass = Other.Owner->GetClassInternal();
					if (OwnerClass == OtherClass)
					{
						FieldId = Other.FieldId;
					}
				}
			}
		}
	private:
		// Those uninitialized fields are left that way to be initialized using Setup during construction
		// After that they are not modified in any way
		JavaObjectBase* Owner;
		const char* FieldName;
		const char* OwnerClassName;
		mutable jfieldID FieldId = 0;
	};

	template <typename T>
	struct FieldOps
	{
		static_assert(AlwaysFalse<T>, "Unsupported type");
	};

#define HIENA_SETUP_FIELD_OPS(TYPE, PRIMITIVE_TYPE) \
	template <>\
	struct FieldOps<TYPE> \
	{ \
		static auto GetField(jobject Obj, jfieldID fieldId, CheckedJniEnv Env = {}) \
		{ \
			return Env->Get##PRIMITIVE_TYPE##Field(Obj, fieldId); \
		} \
		static auto GetStaticField(jclass Clazz, jfieldID fieldId, CheckedJniEnv Env = {}) \
		{ \
			return Env->GetStatic##PRIMITIVE_TYPE##Field(Clazz, fieldId); \
		} \
		static void SetField(jobject Obj, jfieldID fieldId, TYPE Value, CheckedJniEnv Env = {}) \
		{ \
			Env->Set##PRIMITIVE_TYPE##Field(Obj, fieldId, Value); \
		} \
		static void SetStaticField(jclass Clazz, jfieldID fieldId, TYPE Value, CheckedJniEnv Env = {}) \
		{ \
			Env->SetStatic##PRIMITIVE_TYPE##Field(Clazz, fieldId, Value); \
		} \
	};

	HIENA_SETUP_FIELD_OPS(jboolean, Boolean)
	HIENA_SETUP_FIELD_OPS(jbyte, Byte)
	HIENA_SETUP_FIELD_OPS(jchar, Char)
	HIENA_SETUP_FIELD_OPS(jshort, Short)
	HIENA_SETUP_FIELD_OPS(jint, Int)
	HIENA_SETUP_FIELD_OPS(jlong, Long)
	HIENA_SETUP_FIELD_OPS(jfloat, Float)
	HIENA_SETUP_FIELD_OPS(jdouble, Double)
	HIENA_SETUP_FIELD_OPS(jobject, Object)

#undef HIENA_SETUP_FIELD_OPS
}