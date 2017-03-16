#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

void Record_JSFinalize(JSContext *cx, JSObject *obj)
{
 JSPointer<TNameValueList> * t =
   (JSPointer<TNameValueList>*)JS_GetPrivate(cx,obj);
 if (t) delete t;

 JS_SetPrivate(cx,obj,NULL);
}

void ListToObject(JSContext* cx, TNameValueList & t, JSObject* o)
{
 FOREACHITER(t)
   WStr n(t.Name(i));
   WStr v(t.Value(i));
   jsval ptr = STRING_TO_JSVAL(
                JS_NewUCStringCopyN(cx,(uint16*)v,v.bytes()/sizeof(uint16))
                );
   JS_SetUCProperty(cx, o,(uint16*)n,n.bytes()/sizeof(uint16),&ptr);
  DONEFOREACH
}

void ObjectToList(JSContext *cx, JSObject* o, TNameValueList& list)
{
  JSIdArray * props = JS_Enumerate(cx,o);
  if (props)
  {
   jsint index;
   jsint max = props->length;
   jsval np;
   jsval vp;
   for (index=0 ; index< max ; index++)
   {
     jsid id = props->vector[index];
     if (!JS_IdToValue(cx, id, &np))
       continue;
     JSString * name = JS_ValueToString(cx,np);

     TStr n( name ? JS_GetStringChars(name) : (jschar*)0,
             name ? JS_GetStringLength(name) : 0);

     if (JS_GetPropertyById(cx,o,id,&vp))
     {
      JSString * value = JS_ValueToString(cx,vp);
      TStr v( (value) ? JS_GetStringChars(value) : (jschar*)0,
             (value) ? JS_GetStringLength(value) : 0);
      list.Set(n,v);
     }
     else
      list.Set(n,0);
   }
   JS_DestroyIdArray(cx,props);
  }
}

JSBool Record_Record(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;
 if (!Env) return JS_FALSE;

 TParameterList * dt = NULL;
 JSBool b;

 try {
 if (argc == 1 && ISSTR(0))
 {
    GETUTF8(0);
    dt =  new TParameterList(u0,',');
 }
 else if (argc == 2 && ISSTR(0) && ISSTR(1))
 {
    GETUTF8(0);
    GETUTF8(1);
    dt =  new TParameterList(u0,u1[0]);
 }
 else if (argc == 1 && ISBOOL(0))
 {
  b=JSVAL_TO_BOOLEAN(argv[0]);
  dt = new TParameterList(b);
 }
 else if (argc == 1 && JSVAL_IS_OBJECT(argv[0]))
 {
  dt = new TParameterList;
  dt->CaseSensitive = true;
  JSObject *o = JSVAL_TO_OBJECT(argv[0]);
  ObjectToList(cx, o, *dt);
 }
 else if (argc == 0)
  dt = new TParameterList;
 } catch (...) {dt = NULL;}

 if (!dt)
 {
  if (Env->errorOnFailure) return JS_FALSE;
  ERR_MSG(Record,"Out of memory","");
 }

 if (dt)
 CONSTRUCTOR(TNameValueList,dt,true,NULL);
// JS_SetPrivate(cx,obj,dt);

 return JS_TRUE;
}
/*
JSBool Record_JSEnumerate(JSContext *cx, JSObject *obj,
                                     JSIterateOp enum_op,
                                     jsval *statep, jsid *idp)
{
 int32 * x, u;
 GETOBJ(Record,TNameValueList,t);

 switch (enum_op)
 {
  case JSENUMERATE_INIT:
    x = new int32;
    //allocate a new integer, because PRIVATE_TO_JSVAL drops the last bit
    *x = 0;
    *statep = PRIVATE_TO_JSVAL(x);
    if (idp) *idp = INT_TO_JSVAL(t->Count());
    break;
  case JSENUMERATE_NEXT:
    x = (int32*)JSVAL_TO_PRIVATE(*statep);
    u = *x;
    if (u < t->Count())
    {
     if (idp) *idp = INT_TO_JSVAL(u);
     *x = u + 1;
     break;
    }
    //else done -- cleanup.
  case JSENUMERATE_DESTROY:
    x = (int32*)JSVAL_TO_PRIVATE(*statep);
    delete x;
    *statep = JSVAL_NULL;
 };
 return JS_TRUE;
}
*/

JSBool
Record_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 GETOBJ(Record,TNameValueList,t);
 int x = JSVAL_TO_INT(id);

 TParameterList *pl = TYPESAFE_DOWNCAST(t,TParameterList);
 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: RETBOOL(pl ? pl->CaseSensitive : true);
   case 1: RETINT(t->Count());
   case 2: RETSTRW(L"Record");
   default: return JS_FALSE;
  }
 else return JS_FALSE;
}

JSBool
Record_JSSet(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
 GETOBJ(Record,TNameValueList,t);
 JSBool b;
 TParameterList *pl = TYPESAFE_DOWNCAST(t,TParameterList);
 int x = JSVAL_TO_INT(id);
 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: if (!JS_ValueToBoolean(cx,*vp,&b)) return JS_FALSE;
           if (pl) pl->CaseSensitive = b;
           return JS_TRUE;
   default: return JS_FALSE;
  }
 else return JS_FALSE;
}

WRAP(Record,Clear)
{
 GETOBJ(Record,TNameValueList,t);
 TParameterList *pl = TYPESAFE_DOWNCAST(t,TParameterList);
 if (pl) pl->Clear();
 RETBOOL(pl != NULL);
}

I_WRAP_SS(Record,TNameValueList,ReadINIFileSection)

V_WRAP_SS(Record,TNameValueList,WriteINIFileSection)

B_WRAP_S(Record,TNameValueList,Has)

WRAP(Record,Unset)
{
 if (argc != 1) ERR_COUNT(type,name);
 if (!ISSTR(0)) ERR_TYPE(type,name,1,string);
 GETOBJ(Record,TNameValueList,t);
 TParameterList *pl = TYPESAFE_DOWNCAST(t,TParameterList);
 GETUTF8(0);
 if (pl) pl->Unset(u0);
 RETBOOL(pl);
}

S_WRAP_I(Record,TNameValueList,Name)
S_WRAP_I(Record,TNameValueList,Value)
WRAP_HELP(Record,
 "append(Record)\nclear()\nread(text,delim)\nwrite(text,delim)\n"
 "readINI(file,section)\nwriteINI(file,section)\nset(text,value)\n"
 "unSet(text)\nhas(text)\nname(int)\nvalue(int)\ncount()\n"
 "help()\n")


static JSBool
Record_Read(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 const char *s1 = NULL;
 JSString* j0, *j1;
 if (argc == 0 || argc > 2) ERR_COUNT(Record,Read);

 GETOBJ(Record,TNameValueList,t);

 GETUTF8(0);
 if (argc == 2) GETSTRING(1);

 RETINT(t->Read(u0, s1?s1[0] : '&'));
}


static JSBool
Record_Write(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 const char * s0 = NULL;
 JSString* j0;
 if (argc > 1) ERR_COUNT(Record,Read);

 GETOBJ(Record,TNameValueList,t);

 j0 = JS_ValueToString(cx,argv[0]);
 TStr r;
 if (j0)
 {
  TStr s0(JS_GetStringChars(j0));
  t->Write(r,s0);
 }
 else
  t->Write(r,"&");

 RETSTRWC(r);
}


static JSBool
Record_Get(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Record,TNameValueList,t);

 if (argc == 0 || argc > 2) ERR_COUNT(Record,Get);

 GETUTF8(0);

 if (argc == 2)
 {
  GETUTF8(1);
  if (t->Has(u0))
    RETSTRWC((*t)(u0));
  *rval = argv[1];
  return JS_TRUE;
 }
 else
 RETSTRWC((*t)(u0));
}

static JSBool
Record_GetI(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Record,TNameValueList,t);

 if (argc == 0 || argc > 2) ERR_COUNT(Record,Get);

 GETUTF8(0);
 int32 def = 0;
 if (argc == 2 && JS_ValueToInt32(cx, argv[1], &def))
 {
  RETINT(t->GetInt(u0,def));
 }
 else
 RETINT(t->GetInt(u0));
}

static JSBool
Record_GetN(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Record,TNameValueList,t);

 if (argc == 0 || argc > 2) ERR_COUNT(Record,Get);

 GETUTF8(0);
 double def = 0.0;
 if (argc == 2)
  JS_ValueToNumber(cx, argv[1], &def);

 *rval = DOUBLE_TO_JSVAL(JS_NewDouble(cx,t->GetDouble(u0,def)));
 return JS_TRUE;
}

#ifndef XP_WIN
#define GetGValue(rgb)   ((uint8) (((uint16) (rgb)) >> 8))
#define GetRValue(rgb)   ((uint8) (rgb))
#define GetBValue(rgb)   ((uint8) ((rgb) >> 16))
#endif

static JSBool
Record_GetColor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Record,TNameValueList,t);
 if (argc == 0 || argc > 1) ERR_COUNT(Record,GetColor);

 GETUTF8(0);
 int32 c = t->GetInt(u0); // COLORREFINT(0);
 jsval arr[3];

 arr[0] = INT_TO_JSVAL(GetRValue(c));
 arr[1] = INT_TO_JSVAL(GetGValue(c));
 arr[2] = INT_TO_JSVAL(GetBValue(c));

 JSObject * ret = JS_NewArrayObject(cx,3,arr);
  RETOBJ(ret);
}

static JSBool
Record_Set(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Record,TNameValueList,t);

 if (argc != 2) ERR_COUNT(Record,Set);

 if (ISINT(0))
 {
     GETUTF8(1);
     int32 i;
     TOINT(0,i);
     RETBOOL(t->Set(i,u1));
  //TNameValuePair*x = (*t)[INT(0)];
  //GETUTF8(1);
  //if (x)
  // x->Value =  s1;
  //RETBOOL(true);
 }
 else
 {
  GETUTF8(0);
  GETUTF8(1);
  RETBOOL(t->Set(u0,u1));
 }
}

static JSBool
Record_ToObject(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 ENTERNATIVE(cx);
 GETOBJ(Record,TNameValueList,t);
 JSObject * o = ROOT(JS_NewObject(cx, NULL, NULL, NULL));
 ListToObject(cx,*t,o);
 RETOBJ(o);
}

static JSBool
Record_ToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Record,TNameValueList,t);
 TStr out;
 t->Write(out,"\n");
 RETSTRW(WStr(out));
}

WRAP(Record,Close)
{
 CLOSEPRIVATE(Record,TNameValueList);
 RETBOOL(true);
}


/*
 * This function type is used for callbacks that enumerate the properties of
 * a JSObject.  The behavior depends on the value of enum_op:
 *
 *  JSENUMERATE_INIT
 *    A new, opaque iterator state should be allocated and stored in *statep.
 *    (You can use PRIVATE_TO_JSVAL() to tag the pointer to be stored).
 *
 *    The number of properties that will be enumerated should be returned as
 *    an integer jsval in *idp, if idp is non-null, and provided the number of
 *    enumerable properties is known.  If idp is non-null and the number of
 *    enumerable properties can't be computed in advance, *idp should be set
 *    to JSVAL_ZERO.
 *
 *  JSENUMERATE_NEXT
 *    A previously allocated opaque iterator state is passed in via statep.
 *    Return the next jsid in the iteration using *idp.  The opaque iterator
 *    state pointed at by statep is destroyed and *statep is set to JSVAL_NULL
 *    if there are no properties left to enumerate.
 *
 *  JSENUMERATE_DESTROY
 *    Destroy the opaque iterator state previously allocated in *statep by a
 *    call to this function when enum_op was JSENUMERATE_INIT.
 *
 * The return value is used to indicate success, with a value of JS_FALSE
 * indicating failure.
 *
 * You shouldn't ever return JS_FALSE.
 * idp is null on the first call when running a for .. in loop.
 * JSENUMERATE_DESTROY never happens when running a for .. in loop.
 */
/*JSBool Record_JSEnumerate(JSContext *cx, JSObject *obj,
                                     JSIterateOp enum_op,
                                     jsval *statep, jsid *idp)
{
 int32 * x, u;
 TParamterList * n = (TParamterList*)JS_GetPrivate(cx, obj);
 if (!n) return JS_TRUE;

 switch (enum_op)
 {
  case JSENUMERATE_INIT:
    x = new int32;
    //allocate a new integer, because PRIVATE_TO_JSVAL drops the last bit
    *x = 0;
    *statep = PRIVATE_TO_JSVAL(x);
    if (idp) *idp = INT_TO_JSVAL(n->Count());
    break;
  case JSENUMERATE_NEXT:
    x = (int32*)JSVAL_TO_PRIVATE(*statep);
    u = *x;
    if (u < n->Count())
    {
     if (idp) JS_ValueToId(cx, STRING_TO_JSVAL(JS_NewStringCopyZ(cx,t->Name(u))), idp);
     *x = u + 1;
     break;
    }
    //else done -- cleanup.
  case JSENUMERATE_DESTROY:
    x = (int32*)JSVAL_TO_PRIVATE(*statep);
    delete x;
    *statep = JSVAL_NULL;
 };
 return JS_TRUE;
}
*/
/*
 * Like JSResolveOp, but flags provide contextual information as follows:
 *
 *  JSRESOLVE_QUALIFIED   a qualified property id: obj.id or obj[id], not id
 *  JSRESOLVE_ASSIGNING   obj[id] is on the left-hand side of an assignment
 *
 * The *objp out parameter, on success, should be null to indicate that id
 * was not resolved; and non-null, referring to obj or one of its prototypes,
 * if id was resolved.
 *
 * note: objp is null when first called, but don't depend on that
 */
/*
JSBool Num_JSResolve(JSContext *cx, JSObject *obj, jsval id,
              uintN flags, JSObject **objp)
{
  NumberArray * n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n) return JS_TRUE;

  if (!JSVAL_IS_INT(id)) return JS_ResolveStub(cx,obj,id);

  int32 x = JSVAL_TO_INT(id);

  if (flags & JSRESOLVE_ASSIGNING) //Num_Set will expand if necessary
  {
    if (x < 0)
      return JS_TRUE;

    // Define the indexed property
    JS_DefineProperty(cx, obj, (char*)x, INT_TO_JSVAL(x), NULL, NULL,
              JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_INDEX);
    *objp = obj;
  }
  else if (flags & JSRESOLVE_DETECTING || flags & JSRESOLVE_QUALIFIED) //valid property exists with this id?
  {
    if (x >= n->length)
      return JS_TRUE;

    JS_DefineProperty(cx, obj, (char *) x, INT_TO_JSVAL(x), NULL, NULL,
              JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_INDEX);
    *objp = obj;
  }

  return JS_TRUE;
}
*/

static JSBool
Record_Append(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

static JSPropertySpec Record_properties[] = {
    {"caseSensitive",      0,   JSPROP_ENUMERATE|JSPROP_PERMANENT,Record_JSGet,Record_JSSet},
    {"count",      1,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Record_JSGet},
    {"length",      1,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Record_JSGet},
    {"className",2, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Record_JSGet},
        {0}
};

static JSFunctionSpec Record_functions[] = {
    {"append",     Record_Append,     1},
    {"clear",     Record_Clear,       0},
    {"read",     Record_Read,          1},
    {"write",    Record_Write,         1},
    {"readINI",    TNameValueList_ReadINIFileSection,         2},
    {"writeINI",    TNameValueList_WriteINIFileSection,          2},
    {"set",  Record_Set,    2},
    {"unSet",  Record_Unset,    1},
    {"unset",  Record_Unset,    1},
    {"get",  Record_Get,    2},
    {"getN",  Record_GetN,    2},
    {"getI",  Record_GetI,    2},
    {"getColor",  Record_GetColor,    1},
    {"has",  TNameValueList_Has,    1},
    {"name",  TNameValueList_Name,    1},
    {"value",  TNameValueList_Value,    1},
    {"toString", Record_ToString,0},
    {"toObject", Record_ToObject,0},
    {"close", Record_Close,0},
    {0}
};

static JSFunctionSpec Record_fnstatic[] = {
    {"help",  Record_HELP,    0},
    {0}
};

static JSBool
Record_Append(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;
 GETOBJ(Record,TNameValueList,t);

 for (uintN i = 0; i < argc; i++)
 {
  if (!JSVAL_IS_OBJECT(argv[i]))
    ERR_TYPE(Record,Append,i,Object);

  JSObject* j0 = JSVAL_TO_OBJECT(argv[i]);
  if (!j0) return false;

  TNameValueList* r;
  TPointer<TParameterList> rAutoDelete;
  GETREC(0,r);

  if (r)
   {
    t->Append(*r);
   }
/*  else
   {
    jsval last;
    int32 max;
    jsval n,v;
    TChars c(256);

    if (!JS_GetProperty(cx,j0,"count",&last)) goto error;

    if (JSVAL_IS_VOID(last) || JSVAL_IS_NULL(last)) goto error;

    if (!JS_ValueToInt32(cx,last,&max)) goto error;

    for (int32 i=0; i<max; i++)
        {
         JSErrorReporter older = JS_SetErrorReporter(cx,NULL);

         sprintf(c,"name(%d)",i);
         if (!JS_EvaluateScript(cx,j0,c,strlen(c),0,0,&n))
          goto error;

         sprintf(c,"value(%d)",i);
         if (!JS_EvaluateScript(cx,j0,c,strlen(c),0,0,&v))
          goto error;

         JS_SetErrorReporter(cx,older);

         JSString *s0, *s1;
         s0 = JS_ValueToString(cx,n);
         s1 = JS_ValueToString(cx,v);

         if (!s1 || !s0) goto error;

         t->Set(JS_GetStringBytes(s0),JS_GetStringBytes(s1));
        }
   }*/
 }
 RETBOOL(true);

// error:
// JS_ReportError(cx,"Arguments to record.append() must have a "
// "count property and name() and value() functions.");
 return JS_FALSE;
}

JSObject*
Record_Object(JSContext *cx, TNameValueList* t,bool autodelete,JSPointerBase* Parent)
{
 JSObject* obj;
 GETENV;
 MAKENEW(Record);
 //JSPointer<TParameterList> * p =new JSPointer<TParameterList>(t,Parent == NULL);
 //JS_SetPrivate(cx,obj,p);
 SETPRIVATE(obj,TNameValueList,t,autodelete,Parent);
 return obj;
}

void
Record_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(Record);
}

static JSClass Record_class = {
    "Record", JSCLASS_HAS_PRIVATE, // | JSCLASS_NEW_ENUMERATE | JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
//    (JSEnumerateOp)Record_JSEnumerate, (JSResolveOp)Record_JSResolve,
        JS_EnumerateStub, JS_ResolveStub,
    JS_ConvertStub,Record_JSFinalize
};

JSClass* Record_Class() {return &Record_class;}
