#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

static JSFunctionSpec math_static_methods[] = {
#if JS_HAS_TOSOURCE
    {js_toSource_str,   math_toSource,		0, 0, 0},
#endif
    {"abs",		math_abs,		1, 0, 0},
    {"acos",		math_acos,		1, 0, 0},
    {"asin",		math_asin,		1, 0, 0},
    {"atan",		math_atan,		1, 0, 0},
    {"atan2",		math_atan2,		2, 0, 0},
    {"ceil",		math_ceil,		1, 0, 0},
    {"cos",		math_cos,		1, 0, 0},
    {"exp",		math_exp,		1, 0, 0},
    {"floor",		math_floor,		1, 0, 0},
    {"log",		math_log,		1, 0, 0},
    {"max",		math_max,		2, 0, 0},
    {"min",		math_min,		2, 0, 0},
    {"pow",		math_pow,		2, 0, 0},
    {"random",		math_random,		0, 0, 0},
    {"round",		math_round,		1, 0, 0},
    {"sin",		math_sin,		1, 0, 0},
    {"sqrt",		math_sqrt,		1, 0, 0},
    {"tan",		math_tan,		1, 0, 0},
    {0,0,0,0,0}
};

JSObject *
js_InitMathClass(JSContext *cx, JSObject *obj)
{
    JSObject *Math;

    Math = JS_DefineObject(cx, obj, "Math", &math_class, NULL, 0);
    if (!Math)
        return NULL;
    if (!JS_DefineFunctions(cx, Math, math_static_methods))
        return NULL;
    if (!JS_DefineConstDoubles(cx, Math, math_constants))
        return NULL;
    return Math;
}