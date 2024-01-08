#pragma once

#include "Hiena/JavaInvoker.hpp"

#define HIENA_IMPLEMENT_METHOD_IMPL(RETTYPE, FUNCNAME, TYPEARGS, ARGS) \
	RETTYPE FUNCNAME(TYPEARGS) { return ::hiena::JavaInvoker<&FUNCNAME>::Invoke(this, HIENA_PP_UNPACK ARGS); }

#define HIENA_IMPLEMENT_STATIC_METHOD_IMPL(RETTYPE, FUNCNAME, TYPEARGS, ARGS) \
	RETTYPE FUNCNAME(TYPEARGS)  { return ::hiena::JavaInvoker<&FUNCNAME>::StaticInvoke(HIENA_PP_UNPACK ARGS); }

#define HIENA_IMPLEMENT_METHOD(RETTYPE, FUNCNAME, TYPES) \
    HIENA_IMPLEMENT_METHOD_IMPL(RETTYPE, FUNCNAME, HIENA_PP_ZIP(TYPES, (,), HIENA_PP_NAMED_FUNC_ARGS(TYPES, a)), HIENA_PP_NAMED_FUNC_ARGS(TYPES, a))

#define HIENA_IMPLEMENT_METHOD_NOARG(RETTYPE, FUNCNAME) \
    RETTYPE FUNCNAME() { return ::hiena::JavaInvoker<&FUNCNAME>::Invoke(this); }

#define HIENA_IMPLEMENT_STATIC_METHOD(RETTYPE, FUNCNAME, TYPES) \
    HIENA_IMPLEMENT_STATIC_METHOD_IMPL(RETTYPE, FUNCNAME, HIENA_PP_ZIP(TYPES, (,), HIENA_PP_NAMED_FUNC_ARGS(TYPES, a)), HIENA_PP_NAMED_FUNC_ARGS(TYPES, a))

#define HIENA_IMPLEMENT_STATIC_METHOD_NOARG(RETTYPE, FUNCNAME) \
    RETTYPE FUNCNAME() { return ::hiena::JavaInvoker<&FUNCNAME>::StaticInvoke(); }