#pragma once

#define HIENA_CLASS_CONSTRUCTORS_BASE(CLASS, BASE, JNITYPE) \
        using SourceJniType = JNITYPE;\
		CLASS() {} \
		explicit CLASS(JNITYPE Instance) \
			:BASE((jobject)Instance) {} \
		CLASS(JNITYPE Instance, hiena::LocalOwnership_t Tag) \
			:BASE((jobject)Instance, Tag) {}

#define HIENA_CLASS_CONSTRUCTORS(CLASS, BASE, JNITYPE) \
		HIENA_CLASS_CONSTRUCTORS_BASE(CLASS, BASE, JNITYPE) \
		CLASS(const CLASS& Other) \
			:BASE(Other) {} \
		CLASS(CLASS&& Other) \
			:BASE(Other) {} \
		CLASS& operator=(CLASS&& Rhs) = default; \
		CLASS& operator=(const CLASS& Rhs) = default;


#define HIENA_JAVA_FIELDS(...) \
	struct HIENA_PP_CONCAT(Fields__, __LINE__) { \
		__VA_ARGS__ \
	} HIENA_PP_CONCAT(f__, __LINE__) = hiena::detail::InitFields(this, HIENA_PP_CONCAT(f__, __LINE__)); \
	auto operator->() const { return &HIENA_PP_CONCAT(f__, __LINE__); } \
	auto operator->() { return &HIENA_PP_CONCAT(f__, __LINE__); }
