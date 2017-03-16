#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

#ifdef XP_UNIX

#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#endif

// hack because of WIN32_LEAN_AND_MEAN=1 defined in tracemonkey
#ifdef XP_WIN
#undef _INC_SHELLAPI
#include <shellapi.h>
#endif

//from wrap_system

#ifdef XP_WIN
extern size_t CommandLineSize(jschar *s0, jschar* s1);
extern void PrepareCommandLine(WStr& cmdline, jschar *s0, jschar* s1);
#else
extern size_t CommandLineSize(char *s0, char* s1);
extern void PrepareCommandLine(TStr& cmdline, char *s0, char* s1);
#endif

static JSClass* Process_Class();

#ifdef XP_WIN
BOOL CALLBACK closewindowsbypid_callback(HWND hwnd, LPARAM match)
{
   DWORD pid;
   GetWindowThreadProcessId(hwnd,&pid);
   if (pid==(DWORD)match)
      PostMessage(hwnd,WM_CLOSE,0,0);
   return 1;
}

class Process
{public:
 STARTUPINFOW si;
 PROCESS_INFORMATION pi;
 HANDLE handle;
 DWORD PID;

 bool isActive()
 {
  DWORD exitcode;
  return GetExitCodeProcess(handle,&exitcode) ? (exitcode==STILL_ACTIVE) : false;
 }

 int exitCode()
 {
  DWORD exitcode=0;
  GetExitCodeProcess(handle,&exitcode);
  return exitcode;
 }

 void Close();
 Process(uint16* cmdline);
 ~Process();
};

Process::Process(uint16* cmdline)
 {
  handle = INVALID_HANDLE_VALUE;
  PID = 0;

   memset(&si,0,sizeof(STARTUPINFOW));
   si.cb=sizeof(STARTUPINFOW);
   si.dwFlags=STARTF_USESHOWWINDOW;
   si.wShowWindow=SW_SHOWNORMAL;

   if (CreateProcessW(NULL,(WCHAR*)cmdline,NULL,NULL,
                      FALSE,DETACHED_PROCESS,NULL,NULL,&si,&pi))
   {  /* WARNING, if an error occurs, clean this up */
       handle = pi.hProcess;
      PID = pi.dwProcessId;
       CloseHandle(pi.hThread);
   }
 }

 void Process::Close()
 {
  if (isActive() && PID)
      EnumWindows(closewindowsbypid_callback,(LPARAM)PID);
 }

 Process::~Process()
 {
  CloseHandle(handle);
 }

void Process_JSFinalize(JSContext *cx, JSObject *obj)
{
 JSPointer<Process> * t =
   (JSPointer<Process>*)JS_GetPrivate(cx,obj);
 if (t) delete t;

 JS_SetPrivate(cx,obj,NULL);
}

WRAP(Process,Close)
{
 {
  GETOBJ(Process,Process,t);
  if (t) t->Close();
 }
 GETTHIS;
 CLOSEPRIVATE(Process,Process);
 RETBOOL(true);
}

static JSBool
Process_JSGet(JSContext *cx, JSObject *obj, jsid id, jsval *rval)
{
 GETOBJ2(Process,Process,t);

 int x = JSVAL_TO_INT(id);

 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 1: RETBOOL(t->isActive());
   case 2: RETINT(GetCurrentProcessId());
   case 3: RETSTRW(L"Process");
   case 4: RETINT(t->exitCode());
   default: return JS_FALSE;
  }
 else return JS_FALSE;
}

static JSPropertySpec Process_properties[] = {
    {"active",1, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Process_JSGet},
    {"id",2, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Process_JSGet},
    {"className",3, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Process_JSGet},
    {"exitCode",4, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Process_JSGet},
    {0}
};

static JSFunctionSpec Process_fnstatic[] = {
    {0}
};

static JSFunctionSpec Process_functions[] = {
    {"close", Process_Close,0},
    {0}
};

static JSClass Process_class = {
    "Process", JSCLASS_HAS_PRIVATE,         //Process_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,Process_JSFinalize
};

JSObject*
Process_Object(JSContext *cx, Process* t,bool autodelete,JSPointerBase* Parent)
{
 JSObject* obj;
 GETENV;
 ENTERNATIVE(cx);
 MAKENEW(Process);
 SETPRIVATE(obj,Process,t,autodelete,Parent);

 return obj;
}

JSClass* Process_Class() {return &Process_class;}

NATIVE(Process_Process)
{
 if (argc == 0)
   ERR_COUNT(Process,Process);

 GETENV;
 if (!Env) return JS_FALSE;

 if (Env->SafeMode) ERR_MSG(Process,Process,"blocked by security settings");

 GETUCS2(0);
 GETUCS2(1);
 GETUCS2(2);

 WStr cmdline(CommandLineSize(s0,s1));
 PrepareCommandLine(cmdline,s0,s1);

 Process* dt = 0;
 if (!s0)
  ERR_COUNT(Process,Process)

  WCHAR dir[2050];
  if (s2)
  {
      GetCurrentDirectoryW(2048,dir);
      SetCurrentDirectoryW((WCHAR*)s2);
  }

 dt = new Process(cmdline);

/*  if (!dt || dt->handle == INVALID_HANDLE_VALUE && *s1)
  {
   if (dt) delete dt;
   uint16 cmd[1030];
   dt = 0;
   size_t l1 = wcslen((WCHAR*)s0);
   size_t l2 = wcslen((WCHAR*)s1);
   if ((l1+l2) < 1024)
   {
       memcpy(cmd,s0,l1);
       memcpy(&cmd[l1+1],s1,l2);
       cmd[l1]=L' ';
       cmd[l1+l2+1]=0;
       dt = new Process(NULL,cmd);
   }
  }*/
  if (!dt || dt->handle == INVALID_HANDLE_VALUE)
  {
   if (dt) delete dt;
   dt = 0;
   WStr s(wcslen((WCHAR*)s0)+1+MAXPATH);
   if ((int)FindExecutableW((WCHAR*)s0,0,(WCHAR*)s) > 32)
   {
    wcscat(s,L" ");
    wcscat(s,(WCHAR*)s0);
    dt = new Process(s);
   }
  }
  if (!dt || dt->handle == INVALID_HANDLE_VALUE)
  {
   if (dt) delete dt;
   dt = 0;
   if (wcsstr((WCHAR*)s0,L"://")) //URLs are always UTF-8, URL-encoded
   {
       TStr url(s0);
       TStr s;
       *strstr(url,"://") = 0;
       RegGetKey(TStr(url,"\\shell\\open\\command"),"",s,HKEY_CLASSES_ROOT);

    if (*s)
    {
      s.replace("%1",TStr(s0));
      dt = new Process(WStr(s));
    }
   }
  }
  if (!dt || dt->handle == INVALID_HANDLE_VALUE)
  {
   if (dt) delete dt;
   dt = 0;
  }

  if (s2) SetCurrentDirectoryW(dir);

 if (dt)
  CONSTRUCTOR(Process,dt,true,NULL);

 return JS_TRUE;
}

void Process_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(Process);
}

#endif

#ifdef XP_UNIX
class Process
{public:
 int pid;
 TStr cmd;

 Process(const char* cmdline);
 int Close();
 ~Process();
};

int Process::Close()
{
 kill(pid,SIGTERM);
}

Process::Process(const char* cmdline) : cmd(cmdline)
 {
  pid = fork();
  if (pid == -1) throw xdb("unable to fork",cmd);

  if (!pid) //child
  {
    char* args[4];
    args[0] = (char*)"sh";
    args[1] = (char*)"-c";
    args[2] = cmd;
    args[3] = 0;
    execv("/bin/sh",(char*const*)args);
    //execve(cmd,environ); //splitting a command line isn't easy, so let the shell do it.
    _exit(0);
  }
}

Process::~Process()
 {

 }

void Process_JSFinalize(JSContext *cx, JSObject *obj)
{
 JSPointer<Process> * t =
   (JSPointer<Process>*)JS_GetPrivate(cx,obj);
 if (t) delete t;

 JS_SetPrivate(cx,obj,NULL);
}

WRAP(Process,Close)
{
 {
  GETOBJ(Process,Process,t);
  if (t) t->Close();
 }
 GETTHIS;
 CLOSEPRIVATE(Process,Process);
 RETBOOL(true);
}

WRAP(Process,pfork)
{
// GETENV;
 //GETOBJ(Process,Process,p);
 RETINT(fork());
}

WRAP(Process,pwait)
{
 //GETOBJ(Process,Process,p);
 RETBOOL(wait(0) != -1);
}

WRAP(Process,pgetpid)
{
 //GETOBJ(Process,Process,p);
 RETINT(getpid());
}

static JSBool
Process_JSGet(JSContext *cx, JSObject *obj, jsid id, jsval *rval)
{
 GETOBJ2(Process,Process,t);

 int x = JSVAL_TO_INT(id);
 int status;

 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 1: RETBOOL(waitpid(t->pid, &status, WNOHANG) != t->pid);
   case 2: RETINT(t->pid);
   case 3: RETSTRW(L"Process");
   case 4:
        waitpid(t->pid, &status, WNOHANG);
        RETINT(WEXITSTATUS(status));
  }
return JS_FALSE;
}

static JSPropertySpec Process_properties[] = {
    {"active",1, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Process_JSGet},
    {"id",2, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Process_JSGet},
    {"className",3, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Process_JSGet},
    {"exitCode",4, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Process_JSGet},
    {0}
};

static JSFunctionSpec Process_fnstatic[] = {
    {"fork",     Process_pfork,      0},
    {"wait",     Process_pwait,      0},
    {"getpid",     Process_pgetpid,      0},
    {0}
};

static JSFunctionSpec Process_functions[] = {
    {"close", Process_Close,0},
    {0}
};

static JSClass Process_class = {
    "Process", JSCLASS_HAS_PRIVATE,         //Process_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,Process_JSFinalize
};

JSObject*
Process_Object(JSContext *cx, Process* t,bool autodelete,JSPointerBase* Parent)
{
 JSObject* obj;
 GETENV;
 ENTERNATIVE(cx);
 MAKENEW(Process);
 SETPRIVATE(obj,Process,t,autodelete,Parent);

 return obj;
}

NATIVE(Process_Process)
{
 GETENV;
 if (!Env) return JS_FALSE;

 if (Env->SafeMode) ERR_MSG(Process,Process,"blocked by security settings");
 if (argc > 0)
    if (!ISSTR(0)) ERR_TYPE(js,Process,1,String);

  GETUTF8(0);
  GETUTF8(1);
  GETUTF8(2);

  char dir[2048];
  if (*u2)
  {
      getcwd(dir,sizeof(dir));
      chdir(u2);
  }

  TStr cmdline(CommandLineSize(u0,u1));
  PrepareCommandLine(cmdline,u0,u1);

  Process* dt = 0;
  try{
   dt = new Process(cmdline);
   }
  catch (xdb& x)
  {
    ERR_XDB(Process,x);
  }
  if (*u2) chdir(dir);

  if (dt)
  CONSTRUCTOR(Process,dt,true,NULL);
  return JS_TRUE;
}

JSClass* Process_Class() {return &Process_class;}

void Process_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(Process);
}
#endif
