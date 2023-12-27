#pragma once

#include "Hiena/CheckException.hpp"
#include "Hiena/detail/JavaObjectBase.hpp"

namespace hiena::detail
{
	template <typename T>
	struct FieldOps
	{
		static_assert(AlwaysFalse<T>, "Unsupported type");
	};

	class FieldBase
	{
	public:
		FieldBase(){}
		FieldBase(const FieldBase& Other) {}
		FieldBase(FieldBase&& Other) { }
		FieldBase& operator=(const FieldBase& Other)
		{
			return *this;
		}
		FieldBase& operator=(FieldBase&& Other)
		{
			return *this;
		}

		void Setup(JavaObjectBase* InOwner, const char* InOwnerClassName, const char* InName)
		{
			Owner = InOwner;
			OwnerClassName = InOwnerClassName;
			FieldName = InName;
		}

	protected:
		jobject GetOwner()
		{
			return Owner->GetInstance();
		}

		jclass GetStaticOwnerClass(JNIEnv* Env)
		{
			if (Owner)
			{
				return Owner->GetOrInitClassInternal(Env);
			}
			return LowLevelFindClass(OwnerClassName, Env);
		}

		void ReleaseStaticOwnerClass(jclass Clazz, JNIEnv* Env)
		{
			if (!Owner)
			{
				Env->DeleteLocalRef(Clazz);
			}
		}


		jfieldID GetFieldId(const char* Signature, JNIEnv* Env)
		{
			if (FieldId != 0)
			{
				return FieldId;
			}
			if (CheckExceptionFast())
			{
				return {};
			}
			Env = GetEnv(Env);
			jclass Clazz = Owner->GetOrInitClassInternal(Env);
			FieldId = Env->GetFieldID(Clazz, FieldName, Signature);
			if (CheckException(Env))
			{
				return {};
			}
			return FieldId;
		}

		jfieldID GetStaticFieldId(jclass Clazz, const char* Signature, JNIEnv* Env)
		{
			if (FieldId != 0)
			{
				return FieldId;
			}
			if (CheckExceptionFast())
			{
				return {};
			}
			Env = GetEnv(Env);
			FieldId = Env->GetStaticFieldID(Clazz, FieldName, Signature);
			if (CheckException(Env))
			{
				return {};
			}
			return FieldId;
		}

		void TryCopyFieldId(const FieldBase& Other)
		{
			if(FieldId == 0 && Other.FieldId != 0)
			{
				if (std::string_view(FieldName) == std::string_view(Other.FieldName) )
				{
					JNIEnv* Env = GetEnv();
					jclass OwnerClass = Owner->GetOrInitClassInternal(Env);
					jclass OtherClass = Other.Owner->GetOrInitClassInternal(Env);
					if (OwnerClass == OtherClass ||
						Env->IsSameObject(OwnerClass,OtherClass))
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
		jfieldID FieldId = 0;
	};

#define HIENA_SETUP_FIELD_OPS(TYPE, PRIMITIVE_TYPE) \
	template <>\
	struct FieldOps<TYPE> \
	{ \
		static auto GetField(jobject Obj, jfieldID fieldId, JNIEnv* Env = nullptr) \
		{ \
			if (CheckExceptionFast()) \
			{ \
				return CreateDefault<TYPE>(); \
			} \
			Env = GetEnv(Env); \
			auto Ret = Env->Get##PRIMITIVE_TYPE##Field(Obj, fieldId); \
			CheckException(Env); \
			return Ret; \
		} \
		static auto GetStaticField(jclass Clazz, jfieldID fieldId, JNIEnv* Env = nullptr) \
		{ \
			if (CheckExceptionFast()) \
			{ \
				return CreateDefault<TYPE>(); \
			} \
			Env = GetEnv(Env); \
			auto Ret = Env->GetStatic##PRIMITIVE_TYPE##Field(Clazz, fieldId); \
			CheckException(Env); \
			return Ret; \
		} \
		static void SetField(jobject Obj, jfieldID fieldId, TYPE Value, JNIEnv* Env = nullptr) \
		{ \
			if (CheckExceptionFast()) \
			{ \
				return; \
			} \
			Env = GetEnv(Env); \
			Env->Set##PRIMITIVE_TYPE##Field(Obj, fieldId, Value); \
			CheckException(Env); \
			return; \
		} \
		static void SetStaticField(jclass Clazz, jfieldID fieldId, TYPE Value, JNIEnv* Env = nullptr) \
		{ \
			if (CheckExceptionFast()) \
			{ \
				return; \
			} \
			Env = GetEnv(Env); \
			Env->SetStatic##PRIMITIVE_TYPE##Field(Clazz, fieldId, Value); \
			CheckException(Env); \
			return; \
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