#pragma once

#define HIENA_CLASS_CONSTRUCTORS_EX(CLASS, BASE, TYPE, BTYPE) \
        using SourceJniType = TYPE;\
		CLASS() {} \
		explicit CLASS(TYPE Instance) \
			:BASE((BTYPE)Instance) {} \
		CLASS(TYPE Instance, hiena::LocalOwnership_t Tag) \
			:BASE((BTYPE)Instance, Tag) {} \
		CLASS(const CLASS& Other) \
			:BASE(Other) {} \
		CLASS(CLASS&& Other) \
			:BASE(Other) {} \
		CLASS& operator=(CLASS&& Rhs) = default; \
		CLASS& operator=(const CLASS& Rhs) = default; \
		friend TYPE ToJniArgument(const CLASS& Obj, hiena::CheckedJniEnv Env) { return (TYPE)ToJniArgument((JavaObjectBase&)Obj, Env); }

#define HIENA_CLASS_CONSTRUCTORS(CLASS, BASE, TYPE) HIENA_CLASS_CONSTRUCTORS_EX(CLASS, BASE, TYPE, jobject)
#define HIENA_CLASS_CONSTRUCTORS_ARRAY(CLASS, BASE, TYPE) HIENA_CLASS_CONSTRUCTORS_EX(CLASS, BASE, TYPE, TYPE)