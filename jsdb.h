#ifndef _RSLIB_H
#include "rslib.h"
#endif
#ifndef __JSDB_H
#define __JSDB_H
#ifdef __sun__
#else
#include "js/src/jsversion.h"
#if JS_VERSION >= 180
#pragma pack(push,8) //version 1.85 switched to 8-byte packing for a 64-bit jsval
#else
#pragma pack(push,4)
#endif
#endif
#define EXPORT_JS_API
#ifndef _declspec
#define _declspec __declspec
#endif
#include "jsapi.h"

#ifdef XP_UNIX
#include <sys/types.h>
#include <dirent.h>
#endif

#define JSDB_VERSION "1.8.0.7"
#define JSDB_DATE "May 17 2012"
#define JSDB_COPYRIGHT "Copyright 2003-2011 by Shanti Rao and others."

#ifdef XP_WIN
#define JSDB_PLATFORM "Windows"
#elif defined XP_MACOSX
#define JSDB_PLATFORM "OSX"
#elif defined __sun__
#define JSDB_PLATFORM "Sun"
#elif defined __linux__
#define JSDB_PLATFORM "LINUX"
#else
#define JSDB_PLATFORM "UNIX"
#endif

class JSDBEnvironment
 {
  public:
   Stream* out;
   Stream* in;
   Stream* err;
   void* System;
   void* Debugger;
   JSRuntime *rt;
   JSContext *cx;
   JSObject *global;
   //exported classes
   JSObject *oStream, *oRecord, *oProcess;
#ifndef JSDB_MINIMAL
   JSObject *oTable, *oIndex, *oMail, *oServer, *oImage, *oArchive;
#if JS_VERSION < 180
   JSObject *oForm;
#endif
#ifndef TBL_NO_SQL
   JSObject *oODBC;
#endif
#ifndef TBL_NO_SQLITE
   JSObject *oSQLite;
#endif
#ifdef XP_WIN
   JSObject *oActiveX;
#endif
   TBLEnv* TableEnv;
#endif
   bool reportWarnings;
   bool errorOnFailure;
   bool restart;
   bool shouldStop;
   bool (*checkInterrupt)(JSDBEnvironment* Env);
   int SafeMode, GCTimer;
   int Magic; //uninitialized
   int ExitCode;
   bool AllowExec;
   bool AutoDeleteRuntime,AutoDeleteContext;

   SYSTEMTIME startTime;

   JSDBEnvironment(TBLEnv* table)
    {
#ifndef JSDB_MINIMAL
     TableEnv = table;
#endif
     out = in = err = NULL;
     System = NULL;
     Debugger = NULL;
     AutoDeleteRuntime = AutoDeleteContext = false;
     reportWarnings = errorOnFailure = false;
     shouldStop = false;
     AllowExec = false;
     restart = false;
     rt = NULL;
     cx = NULL;
     global = NULL;
     oStream = oRecord = NULL ;
#ifndef JSDB_MINIMAL
     oTable = oMail = oServer = NULL ;
     oIndex = oArchive = oImage = NULL;
 #if JS_VERSION < 180
     oForm = NULL;
 #endif
 #ifndef TBL_NO_SQL
     oODBC = NULL;
 #endif

#ifdef XP_WIN
     oActiveX = NULL;
#endif
#endif
     ExitCode = SafeMode = GCTimer = 0;
     checkInterrupt = NULL;
    }
    ~JSDBEnvironment();

    JSBool ExecScript(void* cx, void* obj, ::Stream &d, const char* filename, int line);
    JSBool ExecScript(void* cx, void* obj, const char* c, size_t length, const char* filename, int line);
    JSBool ExecScript(void* cx, void* obj, const char* c, const char* filename, int line)
    {
     return ExecScript(cx,obj,c,strlen(c),filename,line);
    }

    bool Startup(int memSize, int stackSize,void* global_private); /*!creates a runtime and context */

    void InitGlobal(void* global_private);
    void InitClasses();

    /*! defines the system object. We recommend you use these instead of the global functions */
    JSObject* DefineSystemObject(JSFunctionSpec* extra=0, JSPropertySpec* props=0);

    /*! unroots the global object */
    void Cleanup();

    /*! defines load(), run(), sleep(), exit(), etc... */
    void DefineProcessFunctions();

    /*! defines copyFile(), openBrowser(), fileExists(), regSetKey, regGetKey */
    void DefineShellFunctions();
 };

/** void* ptr = JSDB_StartDebug(rt);
 ...
 JSDB_EndDebug(rt,ptr);
*/

#ifndef JSDB_MINIMAL
void* JSDB_StartDebug(JSRuntime* rt, const char* address, int port);
void JSDB_EndDebug(JSRuntime* rt,void* callerdata);

void DebugScriptSource(void* Debugger,const char* filename, int start, const char* data,size_t length);
#endif
/**
Base class for the JSPointer smart pointer system. Smart pointers are good,
because they implement garbage collection outside of JS. InvalidateChildren()
deletes all its child objects before deallocating its own memory, ensuring that
pointers get deallocated in their proper order. This adds around 32 bytes of
overhead to each object.
*/

class JSPointerBase
{public:
 virtual void Close() = 0;
 TList<JSPointerBase> Children; /// does not autodelete
 JSPointerBase* Parent;
 void AddChild(JSPointerBase *c);
 void RemoveChild(JSPointerBase *c);
 void InvalidateChildren();

 JSPointerBase(JSPointerBase* p);
 ~JSPointerBase();
};

template<class T> class JSPointer :public JSPointerBase
{public: T* P; bool AutoDelete;
   JSPointer(JSPointerBase* parent,T*p, bool ad);
   ~JSPointer();
   operator T*() {return P;}

   /** Notifies the object that it has been orphaned.
       Calls Parent->RemoveChild() to remove the object from the child list.
       Deletes the pointer and invalidates the object.
   */
   void Close();
};

template<class T>
void JSPointer<T>::Close()
{
 if (P && AutoDelete) delete P;
 P=0;
 if (Parent)
  Parent->RemoveChild(this); // Parent->RemoveChild will set Parent=0 if successful
}

template<class T> JSPointer<T>::JSPointer(JSPointerBase* parent,T*p, bool ad) :
  JSPointerBase(parent), P(p), AutoDelete(ad)
{
}

template<class T> JSPointer<T>::~JSPointer()
{
 InvalidateChildren();
 if (Parent)
  Parent->RemoveChild(this);

 if (AutoDelete && P)
  delete P;
 P=0;
}

#define DECLARE_CLASS(name,basedon) \
JSClass* name ## _Class(); \
void name ## _InitClass(JSContext *cx, JSObject *obj); \
void name ## _JSFinalize(JSContext *cx, JSObject *obj); \
JSBool name ## _ ## name \
(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);\
JSBool name ## _JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval);\
JSObject* name ## _Object(JSContext *cx, basedon* t,bool autodelete,JSPointerBase* Parent)

DECLARE_CLASS(Stream,Stream);
DECLARE_CLASS(Record,TNameValueList);
#ifndef JSDB_MINIMAL
 #if JS_VERSION < 180
DECLARE_CLASS(Form,EZFForm);
 #endif
extern void Num_InitClass(JSContext *cx, JSObject *obj);
#ifndef TBL_NO_SQL
DECLARE_CLASS(ODBC,DBPointer);
#endif
#ifndef TBL_NO_SQLITE
extern void SQLite_InitClass(JSContext *cx, JSObject *obj);
#endif
DECLARE_CLASS(Image,GIFImage);
DECLARE_CLASS(Archive,ZipArchive);
DECLARE_CLASS(Index,DataIndex);
DECLARE_CLASS(Table,DataTable);
DECLARE_CLASS(Mail,MailLibrary);
DECLARE_CLASS(Server,InternetServer);
void Process_InitClass(JSContext *cx, JSObject *obj);

#ifdef XP_WIN
void ActiveX_InitClass(JSContext *cx, JSObject *obj);
#endif
#endif
/* Usage:
CoInitialize(NULL);
ActiveX_InitClass(context,global)
Run()...
CoFreeUnusedLibraries();
CoUninitialize();
*/

/** standard error reporter sends error messages to JSEnvironment->out
*/
void rs_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report);

#ifdef __sun__
#else
#pragma pack(pop)
#endif
#endif

