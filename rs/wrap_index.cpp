#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

void DataIndex_JSFinalize(JSContext *cx, JSObject *obj)
{
 JSPointer<DataIndex> * t =
   (JSPointer<DataIndex>*)JS_GetPrivate(cx,obj);
 if (t) delete t;

 JS_SetPrivate(cx,obj,NULL);
}

JSBool
Index_Index(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
    jsval *rval)
{
 GETENV;
 if (!Env) return JS_FALSE;

 TNameValueList * r = 0;
 TPointer<TParameterList> rAutoDelete;
 GETREC(0,r);

 DataIndex * dt = 0;
 if (r) dt = new DataIndex(*r);
 else dt = new DataIndex();

 if (!r && argv && JSVAL_IS_OBJECT(argv[0]))
 {
  jsval p = JSVAL_VOID;
  JSObject * ob = JSVAL_TO_OBJECT(argv[0]);
  jsint i = 0;
  while (1)
  {
   JS_GetElement(cx,ob,i++,&p);
   if (p == JSVAL_VOID) break;

   JSString* str = JS_ValueToString(cx, p);
   dt->AddKey(TStr(JS_GetStringChars(str),JS_GetStringLength(str)));
  }
 }

 if (dt)
  SETPRIVATE(obj,DataIndex,dt,true,NULL);

 return JS_TRUE;
}

JSBool
Index_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 GETOBJ(Index,DataIndex,t);

 int x = JSVAL_TO_INT(id);

 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: RETBOOL(t->CaseSensitive);
   case 3: RETSTRW(L"Index");
   default: return JS_FALSE;
  }
 else return JS_FALSE;
}

static JSBool
Index_JSPut(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 GETOBJ(Index,DataIndex,t);

 int x = JSVAL_TO_INT(id);

 JSBool b;
 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: if (JS_ValueToBoolean(cx,*rval,&b)) t->CaseSensitive = b;
           return JS_TRUE;
   default: return JS_FALSE;
  }
 else return JS_FALSE;
}

WRAP(Index,Rebuild)
{
 GETOBJ(Index,DataIndex,t);
 RETBOOL(t->BuildIndex(0,0));
}

WRAP(Index,Find)
{
 if (argc == 0)
   ERR_COUNT(Index,Find);

 GETOBJ(Index,DataIndex,t);

 //JSString*j0;
 //char* s0;
 TNameValueList * r = 0;
 TPointer<TParameterList> rAutoDelete;
 GETREC(0,r);
 if (r)
  RETINT(t->FindRecord(*r));

 GETUTF8(0);

 if (u0) if (*u0)
 {
   size_t pos = NOT_FOUND;
   if (t->FindPosition(u0,pos))
     RETINT(pos);
 }
 RETINT(-1);
}

WRAP(Index,Add)
{
 if (argc == 0)
   ERR_COUNT(Index,Add);

 GETOBJ(Index,DataIndex,t);
 //JSString*j0;
 //char* s0;

 TNameValueList * r;
 TPointer<TParameterList> rAutoDelete;
 GETREC(0,r);
 if (r)
  t->AddRecord(*r);
 else
 {
  GETUTF8(0);
  if (u0) if (*u0)
    t->AddKey(u0);
 }
 RETOBJ(0);
}

void Index_JSFinalize(JSContext *cx, JSObject *obj)
{
 DELPRIVATE(DataIndex);
}

static JSPropertySpec Index_properties[] = {
    {"caseSensitive",0, JSPROP_ENUMERATE|JSPROP_PERMANENT,Index_JSGet,Index_JSPut},
    {"className",3, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Index_JSGet},
    {0}
};

static JSFunctionSpec Index_fnstatic[] = {
    {0}
};

static JSFunctionSpec Index_functions[] = {
    {"rebuild",     Index_Rebuild,      0},
    {"find",        Index_Find,      1},
    {"add",         Index_Add,      1},
    {"addRow",         Index_Add,      1},
    {0}
};

static JSClass Index_class = {
    "Index", JSCLASS_HAS_PRIVATE,         //Index_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,Index_JSFinalize
};

JSObject*
Index_Object(JSContext *cx, DataIndex* t,bool autodelete,JSPointerBase* Parent)
{
 JSObject* obj;
 GETENV;
 ENTERNATIVE(cx);

 obj = JS_NewObject(cx, JS_GetClass(Env->oIndex),Env->oIndex, NULL);
JS_DefineFunctions(cx,obj,Index_functions);
JS_DefineProperties(cx,obj,Index_properties) ;
// MAKENEW(Index);
 SETPRIVATE(obj,DataIndex,t,autodelete,Parent);

 return obj;
}

void Index_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(Index);
/* Env->Index = JS_InitClass(cx, obj, NULL, Index_Class(),
                           0, 0,
                           Index_properties, Index_functions,
                           NULL,NULL);
*/
}

JSClass* Index_Class() {return &Index_class;}
