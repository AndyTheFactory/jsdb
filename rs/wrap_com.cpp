#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

#ifdef XP_WIN
#include <oleauto.h>
//#include "js/src/jsatom.h"
//#include "js/src/jsfun.h"

//#define DEBUG
//#define TRACE
//dword-word-word-byte[8]
//{85BBD920-42A0-1069-A2E4-08002B30309D}
//HRESULT CLSIDFromString(LPOLESTR lpsz,LPCLSID pclsid);
//S_OK == StringFromCLSID(REFCLSID rclsid,LPOLESTR * ppsz);

#if JS_VERSION >= 185
#else
extern "C" JSObject *
js_NewDateObject(JSContext* cx, int year, int mon, int mday,
                 int hour, int min, int sec);
#endif
#ifndef DISPID_PROPERTYPUT
#define DISPID_PROPERTYPUT (-3)
#endif

class ActiveX //takes ownership, calls Release() at the end
{public:
 IDispatch * dispatch;
 IUnknown * unknown;
 ITypeInfo * typeinfo;
 VARIANT variant;

 struct PropInfo
  {
   WStr name;
   bool Get;
   bool Put;
   bool PutRef;
   PropInfo(const uint16* n): name(n) {Get = Put = PutRef = false;}
  };

 TList<PropInfo> Properties;

 ActiveX();
 ActiveX(CLSID& clsid);
 ActiveX(IUnknown *obj,bool addref = false);
 ActiveX(IDispatch *obj,bool addref = false);
 ActiveX(VARIANTARG& var);
 TIntList properties;

 ~ActiveX();

 PropInfo* Find(const uint16* n);

// bool Id(size_t x,DISPID &dispid);

 bool Id(const uint16* name,DISPID &dispid);

 bool Set(DISPID dispid,JSContext *cx, uintN argc, jsval *argv, jsval* rval, bool ref);

 bool Get(DISPID dispid,JSContext *cx, uintN argc, jsval *argv, jsval* rval, bool exceptions = true);

 //throws an xdb exception on error
 bool Invoke(DISPID dispid, JSContext *cx, uintN argc, jsval *argv, jsval* rval);

 bool SetupMembers(JSContext* cx, JSObject* obj);
};

bool SetupValue(VARIANTARG& arg, JSContext* cx, jsval* rval);

void CheckReturn(JSContext* cx, jsval *rval);

bool RetrieveValue(VARIANTARG& arg, JSContext* cx, jsval* rval);

ActiveX::PropInfo* ActiveX::Find(const uint16* n)
{
#ifdef DEBUG
 TStr dn(n);
 printf("ActiveX::Find %s\n",(char*)dn);
#endif
 FOREACH(PropInfo*p,Properties)
  if (!ucscmp(p->name,n)) return p;
 DONEFOREACH
 return 0;
}

JSClass* ActiveX_Class();

JSObject* ActiveX_Object(JSContext *cx, ActiveX* t,bool autodelete,JSPointerBase* Parent);

extern "C" JSObject *
js_NewDateObject(JSContext* cx, int year, int mon, int mday,
                 int hour, int min, int sec);

#define FETCH(x) (ref? * (var.p ## x) : var.x)

///RetrieveValue assumes that the caller will call VariantClear, so call AddRef on new objects
bool RetrieveValue(VARIANTARG& var, JSContext* cx, jsval* rval)
{
 ENTERNATIVE(cx);
 bool ref = false;
 int type = var.vt;
 if (type & VT_BYREF) {ref = true; type &= ~VT_BYREF;}
 ActiveX* x;
 IDispatch * dispatch;
 IUnknown * unknown;

try {
 switch (type)
 {
  case VT_ERROR: *rval = JSVAL_VOID; break;
  case VT_NULL:
  case VT_EMPTY: *rval = JSVAL_NULL; break;
  case VT_I1: *rval = INT_TO_JSVAL(FETCH(cVal)); break;
  case VT_I2: *rval = INT_TO_JSVAL(FETCH(iVal)); break;
#if JS_VERSION > 180
  case VT_INT:
  case VT_I4: *rval = INT_TO_JSVAL(FETCH(lVal)); break;
  case VT_R4: *rval = DOUBLE_TO_JSVAL(FETCH(fltVal)); break;
  case VT_R8: *rval = DOUBLE_TO_JSVAL(FETCH(dblVal)); break;
#else
  case VT_INT:
  case VT_I4:  if (INT_FITS_IN_JSVAL(FETCH(lVal)))
                  *rval = INT_TO_JSVAL(FETCH(lVal));
               else
                  *rval = DOUBLE_TO_JSVAL(ROOT(JS_NewDouble(cx,FETCH(lVal))));
               break;
  case VT_R4: *rval = DOUBLE_TO_JSVAL(ROOT(JS_NewDouble(cx,FETCH(fltVal)))); break;
  case VT_R8: *rval = DOUBLE_TO_JSVAL(ROOT(JS_NewDouble(cx,FETCH(dblVal)))); break;
#endif

  case VT_BOOL: *rval = BOOLEAN_TO_JSVAL(FETCH(boolVal)?1:0); break;

  case VT_UI1: *rval = INT_TO_JSVAL(FETCH(bVal)); break;
  case VT_UI2: *rval = INT_TO_JSVAL(FETCH(uiVal)); break;
  case VT_UINT:
#if JS_VERSION > 180
  case VT_UI4: *rval = UINT_TO_JSVAL(FETCH(ulVal)); break;
#else
  case VT_UI4: if (FETCH(ulVal) <= JSVAL_INT_MAX)
                  *rval = INT_TO_JSVAL(FETCH(ulVal));
               else
                  *rval = DOUBLE_TO_JSVAL(ROOT(JS_NewDouble(cx,FETCH(ulVal))));
               break;
#endif
  case VT_BSTR: *rval = STRING_TO_JSVAL(ROOT(JS_NewUCStringCopyN(cx,(jschar *)FETCH(bstrVal),SysStringLen(FETCH(bstrVal)))));
//              SysFreeString(FETCH(bstrVal));
//              var.vt = VT_EMPTY;
              break;

  case VT_DATE:
  {
   DATE d = FETCH(date);
   SYSTEMTIME time;
   VariantTimeToSystemTime(d,&time);
#if JS_VERSION >= 185
   *rval = OBJECT_TO_JSVAL(ROOT(JS_NewDateObject(cx,time.wYear,time.wMonth-1,time.wDay,
                time.wHour,time.wMinute, time.wSecond)));
#else
   *rval = OBJECT_TO_JSVAL(ROOT(js_NewDateObject(cx,time.wYear,time.wMonth-1,time.wDay,
                time.wHour,time.wMinute, time.wSecond)));
#endif
  break;
  }

  case VT_UNKNOWN:
  #ifdef TRACE
   printf( "VT_UNKNOWN\n");
  #endif
   if (!FETCH(punkVal)) {*rval = JSVAL_NULL; break;}
   x = new ActiveX(FETCH(punkVal),true);
   if (!x->unknown && !x->dispatch) {delete x; return false;}
   *rval = OBJECT_TO_JSVAL(ActiveX_Object(cx, x,true,NULL));
   break;

  case VT_DISPATCH:
  #ifdef TRACE
   printf( "VT_DISPATCH\n");
  #endif
   if (!FETCH(pdispVal)) {*rval = JSVAL_NULL; break;}
   x = new ActiveX(FETCH(pdispVal),true);
   if (!x->unknown && !x->dispatch) {delete x; return false;}
   *rval = OBJECT_TO_JSVAL(ActiveX_Object(cx, x,true,NULL));
   break;

  case VT_VARIANT: //traverse the indirection list?
   if (ref)
   {
    VARIANTARG* v = var.pvarVal;
    if (v)
      return RetrieveValue(*v,cx,rval);
   }
   break;

  default:
    if (type <= VT_CLSID)
    {
     x = new ActiveX(var);
    *rval = OBJECT_TO_JSVAL(ActiveX_Object(cx,x,true,NULL));
#ifdef DEBUG
    printf("VAR: %x\t%x\n",var.vt,type);
#endif
     return true;
    }
#ifdef DEBUG
    else
    {
     printf("ERR: %X\t%X\n",var.vt,type);
    }
#endif
   return false;
  //default: return false;
 }
 } catch(...)
  {
   return false;
  }
 return true;
}
#undef FETCH

void CheckReturn(JSContext* cx, jsval *rval)
{
 if (JSVAL_IS_OBJECT(*rval))
 {
  HRESULT hresult;
  JSObject* j0 = JSVAL_TO_OBJECT(*rval);
  if (j0 && JS_InstanceOf(cx, j0, ActiveX_Class(), 0))
   {
    ActiveX* x = GETPRIVATE(ActiveX,j0);
    if (x->unknown && !x->dispatch)
    {
     hresult = x->unknown->QueryInterface(IID_IDispatch, (void **)&x->dispatch);
     if (SUCCEEDED(hresult))
     {
       x->SetupMembers(cx, j0);
     }
     else
     {
       x->dispatch = 0;
     }
    }
   }
  }
}

bool SetupValue(VARIANTARG& arg, JSContext* cx, jsval *rval)
{
 VariantInit(&arg);
// arg.vt = VT_EMPTY;

 if (JSVAL_IS_OBJECT(*rval))
 {
   JSObject* j0 = JSVAL_TO_OBJECT(*rval);
   if (j0 && JS_InstanceOf(cx, j0, ActiveX_Class(), 0))
   {
    ActiveX* x = GETPRIVATE(ActiveX,j0);
    if (x->variant.vt != VT_EMPTY)
     {
      //1.7.2.3
      VariantCopyInd(&arg,&x->variant);
      //VariantCopy(&arg,&x->variant);
      //1.7.2.2 could address invalid memory if x is freed before arg
      // arg.vt = VT_VARIANT | VT_BYREF;
      // arg.pvarVal = &x->variant;
      return true;
     }
    if (x->dispatch)
     {
      arg.vt = VT_DISPATCH;
      arg.pdispVal = x->dispatch;
      x->dispatch->AddRef();
      return true;
     }
    else if (x->unknown)
     {
      arg.vt = VT_UNKNOWN;
      arg.punkVal = x->unknown;
      x->unknown->AddRef();
      return true;
     }
    else
     {
      arg.vt = VT_BYREF|VT_UNKNOWN;
      arg.ppunkVal = &x->unknown;
      return true;
     }
   }
 }

 if (JSVAL_IS_BOOLEAN(*rval))
 {
  arg.vt = VT_BOOL;
  arg.boolVal = JSVAL_TO_BOOLEAN(*rval) ? -1 : 0;
  return true;
 }

 if (JSVAL_IS_INT(*rval))
 {
  arg.vt = VT_I4;
  arg.lVal = JSVAL_TO_INT(*rval);
  return true;
 }

 if (JSVAL_IS_DOUBLE(*rval))
 {
  arg.vt = VT_R8;
#if JS_VERSION > 180
  arg.dblVal = JSVAL_TO_DOUBLE(*rval);
#else
  arg.dblVal = *JSVAL_TO_DOUBLE(*rval);
#endif
  return true;
 }

 if (JSVAL_IS_NULL(*rval))
 {
  arg.vt = VT_EMPTY;
  arg.scode = 0;
  return true;
 }

 if (JSVAL_IS_STRING(*rval))
 {
  arg.vt = VT_BSTR;
  arg.bstrVal = SysAllocString((WCHAR*)JS_GetStringChars(JSVAL_TO_STRING(*rval)));
  return true;
 }

 return false;
}

WRAP(ActiveX,as)
{
 if (argc < 1) ERR_COUNT(ActiveX,as);
 if (!ISSTR(0)) ERR_TYPE(ActiveX,as,1,String);
 GETENV;
 GETOBJ(ActiveX,ActiveX,t);

 HRESULT hresult;
 void* specific = NULL;
 CLSID clsid;
 jschar * type = JS_GetStringChars(JSVAL_TO_STRING(argv[0]));

 if (type[0] == L'{')
       hresult = CLSIDFromString((WCHAR*)type,&clsid);
     else
       hresult = CLSIDFromProgID((WCHAR*)type,&clsid);

 if (SUCCEEDED(hresult))
 {
	 IUnknown * unk;
	 hresult = t->unknown->QueryInterface(clsid,(void * *)&unk);
	 if (SUCCEEDED(hresult)) RETOBJ(ActiveX_Object(cx,new ActiveX(unk) ,true,NULL));

 }
 RETOBJ(NULL);
}

#ifdef EXPERIMENT_COM
IDispatch* Recast(IUnknown* unk, ITypeLib* typelib, WCHAR* type)
{
 IUnknown* result= NULL;
 IDispatch* dispatch = NULL;
 ITypeInfo* typeinfo = NULL;
 void* specific = NULL;
 HRESULT hresult;
 unsigned short found = 1;
 MEMBERID memb = 0;
 CLSID clsid;

 if (type[0] == L'{')
       hresult = CLSIDFromString((WCHAR*)type,&clsid);
     else
       hresult = CLSIDFromProgID((WCHAR*)type,&clsid);
 if (!SUCCEEDED(hresult))
      return NULL;

 hresult = typelib->GetTypeInfoOfGuid(clsid,&typeinfo);
 if (!SUCCEEDED(hresult) || !found)
    return NULL;

 hresult = unk->QueryInterface(clsid,&specific);
 if (!SUCCEEDED(hresult) || !specific)
     return NULL;

 hresult = CreateStdDispatch ( unk, specific, typeinfo, &result);

 if (!SUCCEEDED(hresult) || !result)
    return NULL;

 hresult = result->QueryInterface(IID_IDispatch, (void * *)&dispatch);

 if (!SUCCEEDED(hresult))
   dispatch = 0;

 return dispatch;
}

WRAP(ActiveX,typelib)
{
 if (argc < 1) ERR_COUNT(ActiveX,typelib);
 const jschar * library = WSTR(0);
 ITypeLib * typelib = NULL;

 HRESULT hresult = LoadTypeLib( (WCHAR*)library, &typelib);
 if (SUCCEEDED(hresult))
 {
//     MemoryStream s;
    WStr s;

     unsigned count = typelib->GetTypeInfoCount();
     for (unsigned i=0; i<count; i++)
     {
         BSTR name, docstring, helpfile;
         unsigned long helpcontext;

         typelib->GetDocumentation(i,&name,&docstring,&helpcontext,&helpfile);
         WStr w1((uint16*)name,SysStringLen(name));
         WStr w2((uint16*)docstring,SysStringLen(docstring));

         s << (w1) << (uint16*)L": " << (w2) << (uint16*)L"\n";

         SysFreeString(name);
         SysFreeString(docstring);
         SysFreeString(helpfile);
     }

     RETSTRW(s);
 }
 RETSTRW(L"");
}

WRAP(ActiveX,as)
{
 if (argc < 2) ERR_COUNT(ActiveX,as);
 if (!ISSTR(0) || !ISSTR(1)) ERR_TYPE(ActiveX,as,1,String);
 GETENV;
 GETOBJ(ActiveX,ActiveX,t);

 jschar * library = JS_GetStringChars(JSVAL_TO_STRING(argv[0]));
 jschar * name = JS_GetStringChars(JSVAL_TO_STRING(argv[1]));
 ITypeLib * typelib = NULL;

 HRESULT hresult = LoadTypeLib( (WCHAR*)library, &typelib);
 if (SUCCEEDED(hresult))
 {
  IDispatch* d = Recast(t->unknown, typelib, (WCHAR*)name);
  if (d)
  {
   if (t->dispatch) t->dispatch->Release();
   t->dispatch = d;
   t->SetupMembers(cx, obj);
  }

  typelib->Release();

  if (d)  RETOBJ(obj);
 }
 RETOBJ(NULL);
}
#endif

//shadow property 0 to 255
JSBool
ActiveX_JSGet(JSContext *cx, JSObject *obj, jsid id, jsval *rval)
{
 if (!JSVAL_IS_INT(id)) return JS_FALSE;
 int x = JSVAL_TO_INT(id);

 GETENV;
 GETOBJ2(ActiveX,ActiveX,t);

 switch (x)
  {
   case 255: RETSTRW(L"ActiveX");
  }

 ActiveX::PropInfo * p = t->Properties[x];
 DISPID dispid=0;
 if (p)
  if (t->Id((uint16*)p->name,dispid))
   return t->Get(dispid,cx,0,0,rval);

 return JS_FALSE;
}

JSBool
ActiveX_JSSet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 if (!JSVAL_IS_INT(id)) return JS_FALSE;
 int x = JSVAL_TO_INT(id);

 GETENV;
 GETOBJ2(ActiveX,ActiveX,t);

// DISPID dispid=0;
// if (t->Id(id,dispid))

 ActiveX::PropInfo * p = t->Properties[x];
 DISPID dispid=0;
 if (p)
  if (t->Id(p->name,dispid))
   return t->Set(dispid,cx,0,0,rval,p->PutRef);

 return JS_FALSE;
}
/*
char * DeflateString(const jschar*chars, size_t length)
{
    size_t i, size;
    char *bytes;

    size = (length + 1) * sizeof(char);
    bytes = (char *) malloc(size);
    if (!bytes) return NULL;
    for (i = 0; i < length; i++)
        bytes[i] = (char) chars[i];
    bytes[i] = 0;
    return bytes;
}
*/
void ActiveXError(HRESULT hresult, EXCEPINFO& exception, UINT& argerr, JSContext* cx)
{
  char errmsg[1024];
//#define ReportError1(msg,arg)
// sprintf(errmsg,"ActiveX:  msg,arg);
// JS_ReportError(cx,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,errmsg )));

#define ReportError1(msg,arg) \
 {sprintf(errmsg,msg,arg); \
 JS_SetPendingException(cx,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,TStr("ActiveX:", errmsg))));}

#define ReportError(msg) \
 JS_SetPendingException(cx,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,TStr("ActiveX:", msg))))

//  #define ReportError1(msg,arg) JS_ReportError(cx,"ActiveX: " msg,arg);
//#define ReportError(msg) JS_ReportError(cx,"ActiveX: " msg);
// JS_SetPendingException(cx,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,errmsg )));

  switch(hresult)
  {
  case DISP_E_BADPARAMCOUNT: ReportError("Wrong number of parameters"); break;
  case DISP_E_BADVARTYPE: ReportError1("Bad variable type %d",argerr); break;
  case DISP_E_EXCEPTION: if (exception.bstrDescription)
                         {
                          WStr w1((uint16*)exception.bstrDescription,SysStringLen(exception.bstrDescription));
//                          TStr d(w1);

                          WStr w2((uint16*)exception.bstrSource,SysStringLen(exception.bstrSource));
//                          TStr s(w2);
                          WStr w(w1.length() + w2.length() + 20);
                          swprintf(w,L"ActiveX: (%s) %s",(wchar_t*)w2,(wchar_t*)w1);
//                          TStr err(w);
//                          printf("%s\n",(char*)err);

                          JS_SetPendingException(cx,STRING_TO_JSVAL(JS_NewUCStringCopyN(cx,w,w.length())));
//                          char* err = DeflateString((jschar*)exception.bstrDescription,SysStringLen(exception.bstrDescription));
//                          ReportError(err);
//                          ReportError(cx,"%s",TStr("Activex: ",err));
//                          JS_ReportError(cx,"%s",(char*)TStr(err));
//                          free(err);
                         }
                         else
                         {
                           ReportError1("Error code %d",exception.scode);
                         }
                         SysFreeString(exception.bstrSource);
                         SysFreeString(exception.bstrDescription);
                         SysFreeString(exception.bstrHelpFile);
                         break;
  case DISP_E_MEMBERNOTFOUND: ReportError("Function not found"); break;
  case DISP_E_OVERFLOW: ReportError1("Can not convert variable %d",argerr); break;
  case DISP_E_PARAMNOTFOUND: ReportError1("Parameter %d not found",argerr); break;
  case DISP_E_TYPEMISMATCH: ReportError1("Parameter %d type mismatch",argerr); break;
  case DISP_E_UNKNOWNINTERFACE: ReportError("Unknown interface"); break;
  case DISP_E_UNKNOWNLCID: ReportError("Unknown LCID"); break;
  case DISP_E_PARAMNOTOPTIONAL: ReportError1("Parameter %d is required",argerr);
  }
}

bool ActiveX::Id(const uint16 * name,DISPID& dispid)
{
 #ifdef TRACE
 printf( "%d\t%x %x\n",__LINE__,this,dispatch);
#endif
if (!dispatch) return false;

 dispid = 0;
 HRESULT hresult;

 if (!name || name[0] == L'0') {dispid=0; return true;}
#ifdef DEBUG
{
 TStr sn(name);
 printf("ActiveX::Id %s\n",(char*)sn);
}
#endif
 hresult = dispatch->GetIDsOfNames(IID_NULL, (WCHAR**)&name, 1, LOCALE_USER_DEFAULT, &dispid);
#ifdef DEBUG
{
     TStr n(name);
 printf("ID %s %n %x\n",(char*)n,dispid,hresult);
}
#endif
 if (!SUCCEEDED(hresult))
{  hresult = dispatch->GetIDsOfNames(IID_NULL, (WCHAR**)&name, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
#ifdef DEBUG
TStr n(name);
printf("ID %s %n %x\n",(char*)n,dispid,hresult);
#endif
}
 return SUCCEEDED(hresult);
}

bool ActiveX::Invoke(DISPID dispid, JSContext *cx, uintN argc, jsval *argv, jsval* rval)
{
 #ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,dispatch);
#endif
if (!dispatch) return false;
#ifdef DEBUG
 printf("ActiveX::Invoke %d\n",dispid);
#endif

 VARIANT VarResult;
 VARIANTARG * args;
 DISPPARAMS dispparams = {NULL,NULL,0,0};
 HRESULT hresult;
 EXCEPINFO exception={0};
 UINT argerr=0;

 if (argc)
 {
   args = new VARIANTARG[argc];
   dispparams.rgvarg = args;
   dispparams.cArgs = argc;
   for (size_t i=0; i<argc; i++)
   {
    if (!SetupValue(args[argc-i-1],cx,argv+i))
     {
      args[argc-i-1].vt=VT_ERROR;
      args[argc-i-1].scode = 0;
     }
   }
 }

 VariantInit(&VarResult);

 // don't use DispInvoke, because we don't know the TypeInfo
 hresult = dispatch->Invoke(
      dispid,
      IID_NULL,
      LOCALE_USER_DEFAULT,
      DISPATCH_METHOD,
      &dispparams, &VarResult, &exception, &argerr);

 for (size_t i=0; i<argc; i++)
 {
    CheckReturn(cx,argv+i); //in case any empty ActiveX objects were filled in by Invoke()
    VariantClear(&args[i]); //decrement AddRefs() done in SetupValue
 }
 if (argc) delete[] args;

 if (!SUCCEEDED(hresult))
 {
  VariantClear(&VarResult);
  *rval = JSVAL_NULL;
  ActiveXError(hresult,exception,argerr,cx);
  return false;
 }

 if (!RetrieveValue(VarResult, cx, rval))
  {
#ifdef DEBUG
   printf("Unknown result value\n");
#endif
   *rval = JSVAL_NULL;
  }

 VariantClear(&VarResult);
 return true;
}

bool ActiveX::Set(DISPID dispid, JSContext *cx, uintN argc, jsval *argv, jsval *rval,bool byref)
{
 #ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,dispatch);
#endif
if (!dispatch) return false;
VARIANTARG * args = new VARIANTARG[argc+1];
 DISPID dispput = DISPID_PROPERTYPUT;
 DISPPARAMS dispparams = {args,&dispput,argc+1,1};
 HRESULT hresult;
 EXCEPINFO exception={0};
 UINT argerr=0;

 //the set value
 if (!SetupValue(args[0],cx,rval)) return false;

 //the index values, in reverse order
 if (argc)
 {
//   dispparams.rgvarg = args; //initialized in the declaration
//   dispparams.cArgs = argc+1;
   for (size_t i=0; i<argc; i++)
   {
    if (!SetupValue(args[argc-i],cx,argv+i))
     {
      args[argc-i].vt=VT_ERROR;
      args[argc-i].scode = 0;
     }
   }
 }

 DWORD flag = DISPATCH_PROPERTYPUT;
 if (byref && (args[0].vt & VT_DISPATCH || args[0].vt & VT_UNKNOWN))
 {//must be passed by name
  flag = DISPATCH_PROPERTYPUTREF;
 }

 hresult = dispatch->Invoke(
      dispid,
      IID_NULL,
      LOCALE_USER_DEFAULT,
      flag,
      &dispparams, NULL, &exception, &argerr); //no result

 for (size_t i=0; i<argc; i++)
 {
    CheckReturn(cx,argv+i);
    VariantClear(&args[i]);
 }

 if (argc) delete [] args;

  if (!SUCCEEDED(hresult))
  {
     ActiveXError(hresult,exception,argerr,cx);
     return false;
  }

  return true;
}

bool ActiveX::Get(DISPID dispid, JSContext *cx, uintN argc, jsval *argv,  jsval *rval, bool exceptions)
{
 #ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,dispatch);
#endif
if (!dispatch) return false;
#ifdef DEBUG
 printf("ActiveX::Get\n");
#endif

 VARIANT VarResult;
 VARIANTARG * args;
 DISPPARAMS dispparams = {NULL,NULL,0,0};
 HRESULT hresult;
 EXCEPINFO exception={0};
 UINT argerr=0;

 if (argc)
 {
   args = new VARIANTARG[argc];
   dispparams.rgvarg = args;
   dispparams.cArgs = argc;
   for (size_t i=0; i<argc; i++)
   {
    if (!SetupValue(args[argc-i-1],cx,argv+i))
     {
      args[argc-i-1].vt=VT_ERROR;
      args[argc-i-1].scode = 0;
     }
   }
 }

 VariantInit(&VarResult);

 hresult = dispatch->Invoke(
      dispid,
      IID_NULL,
      LOCALE_USER_DEFAULT,
      DISPATCH_PROPERTYGET,
      &dispparams, &VarResult, &exception, &argerr);

 for (size_t i=0; i<argc; i++)
 {
    CheckReturn(cx,argv+i);
    VariantClear(&args[i]);
 }

 if (argc) delete[] args;

 if (!SUCCEEDED(hresult))
 {
  *rval = JSVAL_NULL;
  if (exceptions) ActiveXError(hresult,exception,argerr,cx);
  return false;
 }
 else if (!RetrieveValue(VarResult, cx, rval))
  {
   *rval = JSVAL_NULL;
  }
 VariantClear(&VarResult);

 return true;
}

ActiveX::ActiveX(VARIANTARG& var)
{
#ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,dispatch);
#endif
 unknown = NULL;
 typeinfo = NULL;
 dispatch = NULL;
 VariantInit(&variant);
 VariantCopyInd(&variant,&var);
}

ActiveX::ActiveX()
{
#ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,0);
#endif
 unknown = NULL;
 typeinfo = NULL;
 dispatch = NULL;
 memset(&variant,0,sizeof(variant));
}

ActiveX::ActiveX(IDispatch *obj, bool addref)
{
#ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,obj);
#endif
 unknown = NULL;
 typeinfo = NULL;
 memset(&variant,0,sizeof(variant));
 dispatch = obj;

 if (!dispatch) return;
 if (addref) dispatch->AddRef();
}

ActiveX::ActiveX(IUnknown* obj,bool addref)
{
 dispatch = NULL;
 typeinfo = NULL;
 memset(&variant,0,sizeof(variant));

 unknown = obj;
 if (!unknown) return;

 if (addref) unknown->AddRef();

 HRESULT hresult;

 hresult = unknown->QueryInterface(IID_IDispatch, (void * *)&dispatch);

 if (!SUCCEEDED(hresult))
  dispatch = 0;
#ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,dispatch);
#endif

// else  QueryInterface calls AddRef() for you
  //  dispatch->AddRef();
}

ActiveX::ActiveX(CLSID& clsid)
{
 HRESULT hresult;
 unknown = NULL;
 dispatch = NULL;
 typeinfo = NULL;
 memset(&variant,0,sizeof(variant));

 hresult = CoCreateInstance(clsid, NULL, CLSCTX_SERVER|CLSCTX_INPROC_HANDLER,
            IID_IUnknown, (void **)&unknown);

 if (!SUCCEEDED(hresult)) {unknown = 0; return;} //throw xdb("CoCreateInstance Failure");

 hresult = unknown->QueryInterface(IID_IDispatch, (void * *)&dispatch);

#ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,dispatch);
#endif
 //maybe I don't know what to do with it, but it might get passed to
 //another COM function
 if (!SUCCEEDED(hresult))
 {dispatch = NULL;
  //unknown->Release();
  //unknown=NULL;
  //throw xdb("IDispatch interface not found");
 }
}

ActiveX::~ActiveX()
{
#ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,dispatch);
#endif
 if (dispatch) dispatch->Release();
 if (unknown) unknown->Release();
 if (typeinfo) typeinfo->Release();
 if (variant.vt)
 {
   VariantClear(&variant);
 }
 CoFreeUnusedLibraries();
}

WRAP(ActiveX,Run)
{
 GETENV;
 GETARGS;
 GETOBJ(ActiveX,ActiveX,t);
#ifdef TRACE
 printf("%d\t\t%x %x\n",__LINE__ ,obj,t);
#endif
 JSString* s = JS_GetFunctionId(JS_ValueToFunction(cx, argv[-2]));
 if (s)
  { const uint16* name = (uint16*)JS_GetStringChars(s);
    if (!name)
    {
     ERR_MSG(ActiveX,Exec,"No function name");
    }
#ifdef DEBUG
TStr n(name);
printf("CALL %s\n",(char*)n);
#endif
    DISPID dispid;
    if (!t->Id((uint16*)name,dispid))
    {
      ERR_MSG(ActiveX,"This object does not have that function",TStr(name));
    }
    if (!t->Invoke(dispid, cx, argc, argv, RVAL))
    {
     return JS_FALSE;
//      RETOBJ(0);
      //JavaScript handles the exception with SetPendingException
      ERR_MSG(ActiveX,"IDispatch->Invoke failed",TStr((uint16*)name));
    }
  }
  return JS_TRUE;
}


WRAP(ActiveX,Exec)
{
 GETENV;
 GETOBJ(ActiveX,ActiveX,t);
#ifdef TRACE
 printf("%d\t\t%x %x\n",__LINE__,obj,t);
#endif
 if (argc < 1) ERR_COUNT(ActiveX,Exec);
 if (!ISSTR(0)) ERR_TYPE(ActiveX,Exec,1,string);
 JSString* s = JSVAL_TO_STRING(ARGV(0));
 if (s)
  { jschar* name = JS_GetStringChars(s);
    if (!name)
    {
      ERR_MSG(ActiveX,Exec,"No function name");
    }
    DISPID dispid;
    if (!t->Id(name,dispid))
    {
      ERR_MSG(ActiveX,"This object does not have that function",TStr(name));
    }
    if (!t->Invoke(dispid, cx, argc-1, argv+1, rval))
    {
      return JS_FALSE;
//      ERR_MSG(ActiveX,"IDispatch->Invoke failed",TStr(name));
    }
  }
  return JS_TRUE;
}

///Get("property","index","index")
WRAP(ActiveX,ToString)
{
 GETARGS;
 GETENV;
 GETOBJ(ActiveX,ActiveX,t);
#ifdef TRACE
 printf("%d\t\t%x %x\n",__LINE__,obj,t);
#endif

 if (t->variant.vt)
   RETSTRW(L"variant");

 DISPID dispid = 0;
 if (t->Id((uint16*)L"toString",dispid))
 {
    t->Invoke(dispid, cx, argc, argv, rval);
    return JS_TRUE;
 }

 //if (!t->Get(dispid, cx, 0, 0, rval, false))
   RETSTRW(L"");

// return JS_TRUE;
}

WRAP(ActiveX,Get)
{
 if (argc == 0) ERR_COUNT(ActiveX,Get);
 if (!ISSTR(0) && !ISINT(0)) ERR_TYPE(ActiveX,Get,1,String);

 GETENV;
 GETOBJ(ActiveX,ActiveX,t);
#ifdef TRACE
 printf("%d\t\t%x %x\n",__LINE__,obj,t);
#endif

 DISPID dispid = 0;
 if (ISSTR(0))
  {
  JSString* s = JSVAL_TO_STRING(argv[0]);

 if (s)
  { jschar* name = JS_GetStringChars(s);
    if (!name)
    {
      ERR_MSG(ActiveX,Exec,"No property name");
    }
    if (!t->Id(name,dispid))
    {
      ERR_MSG(ActiveX,"This object does not have that property",TStr(name));
  } }
  }

  if (!t->Get(dispid, cx, argc-1, argv+1, rval))
  {
  //    ERR_MSG(ActiveX,"IDispatch->Invoke failed","");
    return JS_FALSE;
  }
  return JS_TRUE;
}

///Set("property","index","index","value")
WRAP(ActiveX,Set)
{
 if (argc < 2) ERR_COUNT(ActiveX,Set);
 if (!ISSTR(0)) ERR_TYPE(ActiveX,Set,1,String);

 GETENV;
 GETOBJ2(ActiveX,ActiveX,t);
#ifdef TRACE
 printf("%d\t\t%x %x\n",__LINE__,obj,t);
#endif

 JSString* s = JSVAL_TO_STRING(argv[0]);
 if (s)
  { jschar* name = JS_GetStringChars(s);
    if (!name)
    {
      ERR_MSG(ActiveX,Exec,"No property name");
    }

    DISPID dispid;
    if (!t->Id(name,dispid))
    {
      ERR_MSG(ActiveX,"This object does not have that property",TStr(name));
    }

    ActiveX::PropInfo *p = t->Find(name);
    RETBOOL(t->Set(dispid, cx, argc-2, argv+1, argv+argc-1, p?p->PutRef:false));
  }
 RETOBJ(0);
}

bool ActiveX::SetupMembers(JSContext* cx, JSObject* obj)
{
 #ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,dispatch);
#endif
 HRESULT hresult;
 if (unknown && !dispatch)
  hresult = unknown->QueryInterface(IID_IDispatch, (void * *)&dispatch);
 if (!dispatch) return false;
 ENTERNATIVE(cx);

 JSObject * doc = ROOT(JS_NewObject(cx,NULL,NULL,obj));
 JS_DefineProperty(cx, obj,"members",OBJECT_TO_JSVAL(doc),0,0,JSPROP_ENUMERATE);
 ROOT(obj);
 ROOT(doc);

 if (!typeinfo)
 {
  unsigned ctinfo;
  hresult = dispatch->GetTypeInfoCount(&ctinfo);
#ifdef DEBUG
 printf("TYPEINFO %x %d\n",hresult,ctinfo);
#endif
  if (SUCCEEDED(hresult))
   if (ctinfo)
    dispatch->GetTypeInfo(0,0,&typeinfo);
 }

 if (!typeinfo) return false;

 size_t i;
 VARDESC * vardesc;
 for (i=0; typeinfo->GetVarDesc(i, &vardesc) == S_OK && i < 255; i++)
 {
  BSTR name = NULL;
  BSTR desc = NULL;
  if (typeinfo->GetDocumentation(vardesc->memid, &name, &desc, NULL, NULL) == S_OK)
   {
    PropInfo * p = Find((uint16*)name);
    if (!p) p = new ActiveX::PropInfo((uint16*)name);
    p->Get = p->Put = true;
    unsigned prop = Properties.Add(p);

    JS_DefineUCPropertyWithTinyId(cx,obj,(jschar *)name,
            SysStringLen(name), (int8)(prop), OBJECT_TO_JSVAL(NULL),
            ActiveX_JSGet, ActiveX_JSSet, JSPROP_ENUMERATE);
#ifdef DEBUG
{
 TStr n((jschar*)name);
 TStr d((jschar*)desc);
 printf("MEMBER %d %s %s\n",prop,(char*)n,(char*)d);
}
#endif
    if (doc)
     {
      jsval d = desc ? STRING_TO_JSVAL(ROOT(JS_NewUCStringCopyN(cx,(jschar*)desc, SysStringLen(desc)))) : OBJECT_TO_JSVAL(NULL);
      JS_DefineUCProperty(cx, doc, (jschar*)name, SysStringLen(name),d,NULL,NULL,JSPROP_ENUMERATE);
     }
    SysFreeString(name);
    SysFreeString(desc);
   }
  typeinfo->ReleaseVarDesc(vardesc);
 }

 FUNCDESC * funcdesc;
 for (i=0; typeinfo->GetFuncDesc(i, &funcdesc) == S_OK; i++)
 {
  BSTR name = NULL;
  BSTR desc = NULL;

  if (typeinfo->GetDocumentation(funcdesc->memid, &name, &desc, NULL, NULL) == S_OK)
   {
//    char* fname = DeflateString((jschar*)name,SysStringLen(name));

    if (funcdesc->invkind == INVOKE_FUNC)
    {
//       JS_DefineFunction(cx,obj,fname,*ActiveX_Run,funcdesc->cParams,0);
       JS_DefineUCFunction(cx,obj,(jschar*)name,SysStringLen(name),
                           *ActiveX_Run,funcdesc->cParams,0);
#ifdef DEBUG
{
 TStr n((uint16*)name);
 TStr d((uint16*)desc);
 DISPID dispid = 0;
 WStr nn(n);
 dispatch->GetIDsOfNames(IID_NULL, (WCHAR**)&nn, 1, LOCALE_USER_DEFAULT, &dispid);
 printf("FUNCTION %x %x %s %s\n",dispid, funcdesc->memid,(char*)n,(char*)d);
}
#endif
    }
    else
    {
      PropInfo * p = Find((uint16*)name);

      if (!p)
      {
       p = new PropInfo((uint16*)name);
       unsigned prop = Properties.Add(p);
       JS_DefineUCPropertyWithTinyId(cx,obj,(jschar *)name,
                                     SysStringLen(name), (int8)(prop), OBJECT_TO_JSVAL(NULL),
                                     ActiveX_JSGet, ActiveX_JSSet, JSPROP_ENUMERATE);
#ifdef DEBUG
{
 TStr n((jschar*)name);
 TStr d((jschar*)desc);
 printf("PROPERTY %d %s %s\n",prop,(char*)n,(char*)d);
}
#endif
      }

      if (funcdesc->invkind & INVOKE_PROPERTYGET)
        p->Get = true;
      if (funcdesc->invkind & INVOKE_PROPERTYPUT)
        p->Put = true;
      if (funcdesc->invkind & INVOKE_PROPERTYPUTREF)
        p->PutRef = true;
    }
//    if (fname) free(fname);

    if (doc)
     {
      jsval d = desc && *desc ? STRING_TO_JSVAL(ROOT(JS_NewUCStringCopyN(cx,(jschar*)desc, SysStringLen(desc)))) : JSVAL_NULL;
      JS_DefineUCProperty(cx, doc, (jschar*)name, SysStringLen(name),
       d,NULL,NULL,JSPROP_ENUMERATE);
     }

    SysFreeString(name);
    SysFreeString(desc);
   }
  typeinfo->ReleaseFuncDesc(funcdesc);
 }

#ifdef DEBUG
{
 printf("DONE SetupMembers\n");
}
#endif
#ifdef TRACE
 printf("%d\t%x %x\n",__LINE__ ,this,dispatch);
#endif
 return true;
}

WRAP(ActiveX,Close)
{
 #ifdef TRACE
 printf( "CLOSE\n");
#endif
CLOSEPRIVATE(ActiveX,ActiveX);
 RETBOOL(true);
}

void ActiveX_JSFinalize(JSContext *cx, JSObject *obj)
{
 #ifdef TRACE
 printf( "FINALIZE\n");
#endif
DELPRIVATE(ActiveX);
}

WRAP_HELP(ActiveX,
 "name(index)\nextract(index)\nextract(index,string)\nsize(index)\n"
 "close()\n")

static JSPropertySpec ActiveX_properties[] = {
    {"className",255, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,ActiveX_JSGet},
    {0}
};

static JSFunctionSpec ActiveX_functions[] = {
    {"get",     ActiveX_Get,      1},
    {"set",    ActiveX_Set, 2},
    {"exec",    ActiveX_Exec, 2},
    {"at",    ActiveX_Exec, 2},
//#ifdef EXPERIMENT_COM
    {"as",    ActiveX_as, 2},
//#endif
    {"close",ActiveX_Close,0},
    {"toString",ActiveX_ToString,0},
    {0}
};

static JSFunctionSpec ActiveX_fnstatic[] = {
    {"help",  ActiveX_HELP,    0},
#ifdef EXPERIMENT_COM
    {"typelib",  ActiveX_typelib,    1},
#endif
    {0}
};

static JSClass ActiveX_class = {
    "ActiveX", JSCLASS_HAS_PRIVATE,         //ActiveX_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, ActiveX_JSFinalize
};

JSObject*
ActiveX_Object(JSContext *cx, ActiveX* t,bool autodelete,JSPointerBase* Parent)
{
 GETENV;
 JSObject* obj;
 ENTERNATIVE(cx);
 MAKENEW(ActiveX);
 /*obj = JS_NewObject(cx, ActiveX_Class(),Env->ActiveX, NULL);
 JS_DefineFunctions(cx,obj,ActiveX_functions);
 JS_DefineProperties(cx,obj,ActiveX_properties);   */
 if (t)
 {
  SETPRIVATE(obj,ActiveX,t,autodelete,Parent);
  t->SetupMembers(cx,obj);
 }
 return obj;
}

NATIVE(ActiveX_ActiveX)
{
 GETENV;
 if (Env->SafeMode) ERR_MSG(ActiveX,ActiveX,"blocked by security settings");

 if (argc)
  if (!ISSTR(0)) ERR_TYPE(ActiveX,ActiveX,1,String);
 //ENTERNATIVE(cx);

 //argc > 0 if clsid is valid
 CLSID clsid;
 HRESULT hresult;
 jschar * name;

 if (argc)
 {
     name = JS_GetStringChars(JSVAL_TO_STRING(ARGV(0)));
     if (name[0] == L'{')
       hresult = CLSIDFromString((WCHAR*)name,&clsid);
     else
       hresult = CLSIDFromProgID((WCHAR*)name,&clsid);

     if (!SUCCEEDED(hresult))
     {
      ERR_MSG(ActiveX,ActiveX,"invalid CLSID");
     }
 }
 ActiveX* t = NULL;

 if (argc == 0)
 {
  t = new ActiveX();
 }
 else
 {
  IUnknown* unk = NULL ;
  if (argc == 1)
  {
   hresult = GetActiveObject(clsid,NULL,&unk);
  }

  if (SUCCEEDED(hresult) && unk)
  {
    t = new ActiveX(unk);
    if (!t->unknown)
    {
     delete t;
     t=0;
     ERR_MSG(ActiveX,"Can't create ActiveX object",TStr(name));
    }
  }
 }

 if (!t)
 {
   t = new ActiveX(clsid);
   if (!t->unknown)
   {
    delete t;
    ERR_MSG(ActiveX,"Can't create ActiveX object",TStr(name));
   }
 }
 if (t)
 {
  CONSTRUCTOR(ActiveX,t,true,NULL);
#if JS_VERSION > 180
#else
  t->SetupMembers(cx,obj);
#endif
 }
 return JS_TRUE;
}

JSClass* ActiveX_Class() {return &ActiveX_class;}

void ActiveX_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(ActiveX);
}

/*
CoInitialize(NULL);
ActiveX_InitClass(cx,obj);
CoFreeUnusedLibraries();
CoUninitialize();
*/

#endif
