#pragma once

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
		FieldBase() = default;
		FieldBase(const FieldBase& Other)
		:FieldBase()
		{
			*this = Other;
		}
		FieldBase(FieldBase&& Other)
		:FieldBase()
		{
			*this = std::move(Other);
		}
		FieldBase& operator=(const FieldBase& Other)
		{
			if(!Owner)
			{
				Owner = Other.Owner;
				FieldName = Other.FieldName;
				FieldId = Other.FieldId;
			}
			else
			{
				TryCopyFieldId(Other);
			}
			return *this;
		}
		FieldBase& operator=(FieldBase&& Other)
		{
			if(!Owner)
			{
				Owner = std::exchange(Other.Owner, nullptr);
				FieldName = std::exchange(Other.FieldName, nullptr);
				FieldId = std::exchange(Other.FieldId, nullptr);
			}
			else
			{
				TryCopyFieldId(Other);
			}
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

		jfieldID GetFieldId(const char* Signature, JNIEnv* Env)
		{
			if (FieldId != 0)
			{
				return FieldId;
			}
			jclass Clazz = Owner->GetOrInitClassInternal(Env);
			FieldId = Env->GetFieldID(Clazz, FieldName, Signature);
			// CheckException
			return FieldId;
		}

		jfieldID GetStaticFieldId(jclass Clazz, const char* Signature, JNIEnv* Env)
		{
			if (FieldId != 0)
			{
				return FieldId;
			}
			FieldId = Env->GetStaticFieldID(Clazz, FieldName, Signature);
			// CheckException
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
					jclass OtherClass = Owner->GetOrInitClassInternal(Env);
					if (OwnerClass == OtherClass ||
						Env->IsSameObject(OwnerClass,OtherClass))
					{
						FieldId = Other.FieldId;
					}
				}
			}
		}
	private:
		JavaObjectBase* Owner = nullptr;
		const char* FieldName = nullptr;
		const char* OwnerClassName = nullptr;
		jfieldID FieldId = 0;
	};

#define HIENA_SETUP_FIELD_OPS(TYPE, PRIMITIVE_TYPE) \
	template <>\
	struct FieldOps<TYPE> \
	{ \
		static auto GetField(jobject Obj, jfieldID fieldId, JNIEnv* Env = nullptr) \
		{ \
			auto Ret = GetEnv(Env)->Get##PRIMITIVE_TYPE##Field(Obj, fieldId); \
			/*CheckException*/ \
			return Ret; \
		} \
		static auto GetStaticField(jclass Clazz, jfieldID fieldId, JNIEnv* Env = nullptr) \
		{ \
			auto Ret = GetEnv(Env)->GetStatic##PRIMITIVE_TYPE##Field(Clazz, fieldId); \
			/*CheckException*/ \
			return Ret; \
		} \
		static void SetField(jobject Obj, jfieldID fieldId, TYPE Value, JNIEnv* Env = nullptr) \
		{ \
			GetEnv(Env)->Set##PRIMITIVE_TYPE##Field(Obj, fieldId, Value); \
			/*CheckException*/ \
			return; \
		} \
		static void SetStaticField(jclass Clazz, jfieldID fieldId, TYPE Value, JNIEnv* Env = nullptr) \
		{ \
			GetEnv(Env)->SetStatic##PRIMITIVE_TYPE##Field(Clazz, fieldId, Value); \
			/*CheckException*/ \
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