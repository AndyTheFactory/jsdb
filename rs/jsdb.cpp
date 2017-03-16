#include "rslib.h"
#include "jsdb.h"
#include "js/src/jscpucfg.h"  // file missing in tracemonkey
#include "rs/wrap_jsdb.h"
#include "js/src/jspubtd.h"
#ifdef XP_WIN
#include <conio.h>
#endif
#ifdef XP_UNIX
#include <sys/select.h>
extern "C" char * readline(char *prompt);
extern "C" void add_history(char  *line);
#endif

#include "js/src/jscntxt.h"

// needed for Coinitialize because of WIN32_LEAN_AND_MEAN=1 defined in tracemonkey
#ifdef XP_WIN
#include <Objbase.h>
#endif

class System
{
 public:
 TStr path;
 TParameterList modules;
#ifndef JSDB_MINIMAL
 ZipArchive* zip;
#endif
 System()
 {
#ifdef XP_WIN
 modules.CaseSensitive = false;
#else
 modules.CaseSensitive = true;
#endif
#ifndef JSDB_MINIMAL
  zip = 0;
#endif
 }
};

// wrap RS lib functions in JavaScript
TParameterList GlobalOptions;

void StartConsole(JSDBEnvironment*Env);
void ExecCommand(JSDBEnvironment* Env,const char * cmd,size_t len,uintN line);
void PrintDirectory(Stream& out, const char * filter = 0);

static JSBool GarbageCollect(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 JS_GC(cx);
 RETBOOL(true);
}

extern JSBool
Readln(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

static JSBool
Readbytes(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(system,read);
 if (!ISINT(0)) ERR_TYPE(system,read,1,Integer);

 GETENV;
 if (Env->in)
 {
  int32 length;
  TOINT(0,length);
  int pos = 0;
  if (length < 1) RETSTRW(L"");

  TStr line(length);
#ifdef XP_WIN
  DWORD mode;
  HANDLE h = 0;
  if (!strcmp(Env->in->filename(),"stdin"))
    h = GetStdHandle(STD_INPUT_HANDLE);
  if (h) if (!GetConsoleMode(h,&mode)) h=0;
  if (h) SetConsoleMode(h,mode & ~(ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_PROCESSED_OUTPUT));
#endif
  while (pos < length)
  {
   pos += Env->in->read(line + pos,length - pos);
  }
#ifdef XP_WIN
  if (h) SetConsoleMode(h,mode);
#endif
  RETSTRWC(line);
 }
 RETSTRW(L"");
}
//-------------------

static JSBool
Help(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    GETENV;
    WRITELN("JSDB " JSDB_VERSION " " JSDB_DATE "\n"
            JSDB_COPYRIGHT " See http://www.jsdb.org/\n");
    return JS_TRUE;
}

static JSBool
TestCompile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(jsdb,testCompile);
 if (!ISSTR(0)) ERR_TYPE(jsdb,testCompile,1,String);
 GETENV;

 JSString* str = JSVAL_TO_STRING(argv[0]);

 MemoryStream f;
 Stream* oldout = Env->out;
 Env->out = &f;
 JSStackFrame * fp = Env->cx->fp;
 Env->cx->fp = 0;
 JSScript * script = JS_CompileUCScript( Env->cx,
                                       Env->global,
                                       JS_GetStringChars(str),
                                       JS_GetStringLength(str),
                                       "",
                                       1);
 Env->out = oldout;
 Env->cx->fp = fp;
 RETSTR((char*)f);
 // garbage collection should take care of script

}

JSBool Mkdir(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc > 0)
 MakeDirectoryExist(UTF8STR(0));
 RETBOOL(true);
}

JSBool Resource(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;

 if (argc < 1) RETOBJ(0);
 JSString *str;
 Stream* dt = 0;

     if (!ISSTR(0)) RETBOOL(false);
     const char * filename = STR(0);
     //look for the file in the EXE's ZIP archive
     System* sys = (System*)Env->System;
     if (sys)
     {
#ifndef JSDB_MINIMAL
      ZipArchive * zip = sys->zip;

      size_t index = NOT_FOUND;
      if (zip) index = zip->Find(filename);

      if (index != NOT_FOUND)
      {
       dt = new MemoryStream;
       if (zip->Extract(index,*dt))
        {
         dt->rewind();
        }
      } else
#endif
      //look in the path (never the local directory)
      if (!strchr(filename,'/') && !strchr(filename,'\\'))
      {
       TStr file(sys->path,filename);
       FixFilename(file);
       if (FileExists(file))
       {
       try
       {
        dt = new FileStream(file,Stream::OMBinary,Stream::ReadOnly);
       }catch(...) {}
       }
      }
     }

     if (dt)
      RETOBJ(Stream_Object(cx,dt,true,0));
     RETOBJ(0);
}

JSBool DiskLoadProgram(JSDBEnvironment* Env, System* sys, const char* file, jsval *rval)
{
     //then use the file name
     if (FileExists(file))
     {
       int line = 1;
       FileStream in(file,Stream::OMBinary,Stream::ReadOnly);
       if (rval) *rval = BOOLEAN_TO_JSVAL(true);

       return Env->ExecScript(0,0,in,in.filename(),line);
     }
     RETBOOL(false);
}

#ifndef JSDB_MINIMAL
JSBool ZipLoadProgram(JSDBEnvironment* Env, System* sys, const char* file, jsval *rval)
{
     if (sys)
     {
     //look for the file in the EXE's ZIP archive
      ZipArchive * zip = sys->zip;

      size_t index = NOT_FOUND;
      if (zip) index = zip->Find(file);

      if (index != NOT_FOUND)
      {
       int line = 1;
       MemoryStream m;
       if (zip->Extract(index,m))
        {
         m.rewind();
         RETBOOL(Env->ExecScript(0,0,m,file,line));
        }
      }
     }
     RETBOOL(false);
}
#endif

/*
JSBool Run(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;
 *rval = OBJECT_TO_JSVAL(0);

 if (Env->SafeMode) RETBOOL(false);
 if (argc < 1) RETBOOL(false);
 JSString *str;

   Stream* data;

   GETFILE(0,data);

   try
   {
    if (data) //execute from a stream
    {
     const char* filename = (argc > 1 && ISSTR(1)) ? STR(1) : data->filename();
     int line = (argc > 2 && ISINT(2)) ? INT(2) : 1;

     *rval = BOOLEAN_TO_JSVAL(true);
     return Env->ExecScript(*data,filename,line);
    }
    else //load a file
    {
     if (!ISSTR(0)) RETBOOL(false);
     const char * filename = STR(0);
     System* sys = (System*)Env->System;
     return LoadProgram(Env,sys,filename,rval);
    }
   }
   catch(xdb& x)
   {
    ERR_MSG(Load,"File open failure",TStr(x.why(), x.info()));
   }
}
*/

JSBool Run(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  GETENV;
  *rval = OBJECT_TO_JSVAL(0);

  if (Env->SafeMode) RETBOOL(false);
  if (argc < 1) RETBOOL(false);
  Stream* data;
  GETFILE(0,data);

  try
  {
     if (data) //execute from a stream
     {
        const char* filename = (argc > 1 && ISSTR(1)) ? STR(1) : data->filename();
        int32 line = 1;
        if (argc > 2 && ISINT(2))
         TOINT(2,line);
        *rval = BOOLEAN_TO_JSVAL(true);
        return Env->ExecScript(cx,obj,*data,filename,line);
     }

     if (!ISSTR(0)) RETBOOL(false);
     GETUTF8(0);
     TStr name(u0); //use the original when looking up in a Zip archive
     Replace(name, "\\", '/');

     System* sys = (System*)Env->System;
#ifndef JSDB_MINIMAL
     if (sys)
     {
      ZipArchive * zip = sys->zip;
      if (zip)
      {
       if (zip->Find(u0) != NOT_FOUND)
       return ZipLoadProgram(Env,sys,u0,rval);
      }
     }
#endif
     /* The local directory overrides the system path */

     FixFilename(name);

     if (sys)
     {
      TStr othername(sys->path,name);
      FixFilename(othername);

      if (!FileExists(name) && FileExists(othername))
        name = othername;
     }

     return DiskLoadProgram(Env,sys,name,rval);
  }
  catch(xdb& x)
  {
   ERR_MSG(Load,"File open failure",TStr(x.why(), x.info()));
  }
}

JSBool Load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;
 *rval = OBJECT_TO_JSVAL(0);

 if (Env->SafeMode) RETBOOL(false);
 if (argc < 1) RETBOOL(false);

   try
   {
     if (!ISSTR(0)) RETBOOL(false);
     GETUTF8(0);
     int filetime = 0;
     TStr name(u0);
     Replace(name, "\\", '/');

     System* sys = (System*)Env->System;
#ifndef JSDB_MINIMAL
     if (sys)
     {
      ZipArchive * zip = sys->zip;
      if (zip)
      {
       if (zip->Find(u0) != NOT_FOUND)
       {
        if (sys->modules.Has(u0)) RETBOOL(true);
        JSBool ret = ZipLoadProgram(Env,sys,u0,rval);
        if (ret) sys->modules.Set(u0,1);
        return ret;
       }
      }
     }
#endif
     /* The local directory overrides the system path */

     FixFilename(name);

     if (sys)
     {
      TStr othername(sys->path,name);
      FixFilename(othername);

      if (!FileExists(name) && FileExists(othername))
        name = othername;

#ifdef XP_WIN
     WIN32_FILE_ATTRIBUTE_DATA attr;
     if (GetFileAttributesExW(WStr(name),GetFileExInfoStandard,&attr))
      filetime = attr.ftLastWriteTime.dwLowDateTime;
#else
     struct stat ms;
     int x = stat(name,&ms);
     if (x == 0) filetime =  ms.st_mtime;
#endif
     }

     if (sys && filetime)
     {
      if (sys->modules.GetInt(name) == filetime)
       RETBOOL(true);
     }

     JSBool ret = DiskLoadProgram(Env,sys,name,rval);
     if (ret && sys) sys->modules.Set(name,filetime);
     return ret;
   }
   catch(xdb& x)
   {
    ERR_MSG(Load,"File open failure",TStr(x.why(), x.info()));
   }
}
/*
JSBool OldLoad(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;
 *rval = OBJECT_TO_JSVAL(0);

 if (Env->SafeMode) RETBOOL(false);
 if (argc < 1) RETBOOL(false);

   try
   {
     if (!ISSTR(0)) RETBOOL(false);
     System* sys = (System*)Env->System;
     GETUTF8(0);
     int filetime = 0;
#ifndef JSDB_MINIMAL
     ZipArchive * zip = sys->zip;
     if (zip)
     {
      if (zip->Find(s0) != NOT_FOUND)
      filetime = 1;
      JSBool ret = ZipLoadProgram(Env,sys,s0,rval);
      if (ret && sys) sys->modules.Set(s0,filetime);
      return ret;
     }
     else
#endif
     {
      TStr othername(sys->path,s0);
      FixFilename(othername);

      if (!FileExists(s0) && FileExists(othername))
        s0 = othername;
#ifdef XP_WIN
     WIN32_FILE_ATTRIBUTE_DATA attr;
     if (GetFileAttributesExW(WStr(s0),GetFileExInfoStandard,&attr))
      filetime = attr.ftLastWriteTime.dwLowDateTime;
#else
     struct stat ms;
     int x = stat(s0,&ms);
     if (x > 0) filetime =  ms.st_mtime;
#endif
     }
     if (sys && filetime)
     {
      if (sys->modules.GetInt(s0) == filetime)
       RETBOOL(true);
     }
     JSBool ret = DiskLoadProgram(Env,sys,s0,rval);
     if (ret && sys) sys->modules.Set(s0,filetime);
     return ret;
   }
   catch(xdb& x)
   {
    ERR_MSG(Load,"File open failure",TStr(x.why(), x.info()));
   }
}
*/

JSBool KBHit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
#ifdef XP_WIN
    static INPUT_RECORD pinp[16];
    DWORD nread, nevents, j;
    HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
    GetNumberOfConsoleInputEvents(hstdin, &nevents);
    if (nevents == 0 || nevents > 16)
        RETBOOL(false);
    PeekConsoleInput(hstdin, pinp, nevents, &nread);
    for (j = 0;j<nevents;j++)
        if ((pinp[j].EventType & KEY_EVENT) != 0)
            if (pinp[j].Event.KeyEvent.bKeyDown != 0)
            {
                if ((pinp[j].Event.KeyEvent.wVirtualKeyCode == VK_SHIFT)   ||
                    (pinp[j].Event.KeyEvent.wVirtualKeyCode == VK_CONTROL) ||
                    (pinp[j].Event.KeyEvent.wVirtualKeyCode == VK_MENU))
                      continue;
                else
                      RETBOOL(true);
            }
   RETBOOL(false);
#else
  struct timeval tv;
  fd_set read_fd;
  tv.tv_sec=0;
  tv.tv_usec=0;
  FD_ZERO(&read_fd);
  FD_SET(0,&read_fd);
  if (select(1, &read_fd, NULL, NULL, &tv) == -1)
   RETBOOL(false);
  if(FD_ISSET(0,&read_fd))
    RETBOOL(true);
  RETBOOL(false);
#endif
}

inline bool isCGI()
{
 const char* c = getenv("GATEWAY_INTERFACE");
 return c && (c[0] == 'C' && c[1] == 'G' && c[2] == 'I');
}

void CGIErrorMessage(Stream& out)
{
  if (isCGI()) out << "Content-Type: text/plain\nWarn: 500 Script Error\n\n";
}
int main(int argc, char **argv)
{
 /*set up global JS variables, including global and custom objects */
 /* can't trust argv[0] on gcc. */
 #ifdef _Windows
  char argv0[MAXPATH];
  GetModuleFileName(0,argv0,MAXPATH);
 #else
  char* argv0 = argv[0];
 #if defined(XP_MACOSX) || defined(__sun__)
  char selfpath[PATH_MAX];
  if (realpath(argv[0],selfpath));
   argv0 = selfpath;
 #else
  char selfpath[PATH_MAX];
  if (readlink( "/proc/self/exe", selfpath, PATH_MAX) > 0)
   argv0 = selfpath;
 #endif
 #endif
 TBLEnv Tables;
 TPointer<JSDBEnvironment> Env(new JSDBEnvironment(&Tables));
 Env->reportWarnings = false;
 Env->shouldStop = false;

 Env->rt = JS_NewRuntime(8L * 1024L * 1024L);

 if (!Env->rt)
  {
   return 1;
  }

 Env->cx = JS_NewContext(Env->rt, 8192);

 JS_SetVersion(Env->cx,JSVERSION_1_7);

 if (!Env->cx)
  {
   JS_DestroyRuntime(Env->rt);
   return 2;
  }

#ifdef XP_WIN
 CoInitialize(NULL);
#endif

 Env->in = new FileStream("stdin",Stream::OMBinary);
 Env->out = new FileStream("stdout",Stream::OMBinary);

 JSFunctionSpec extra_functions[] = {
    {"load",            Load,           1},
    {"run",             Run,           1},
    {"loadResource",    Resource,       1},
    {"help",            Help,           0},
    {"jsTestCompile",   TestCompile,    1},
    {"jsGC",            GarbageCollect, 0},
    {0}};

 JSFunctionSpec extra_functions2[] = {
    {"load",            Load,           1},
    {"run",             Load,           1},
    {"resource",    Resource,           1},
    {"help",            Help,           0},
    {"compile",   TestCompile,          1},
    {"gc",     GarbageCollect,          0},
    {"read",        Readbytes,          1},
    {"readln",         Readln,          1},
    {"readLine",       Readln,          1},
    {"mkdir",       Mkdir,          1},
    {"kbhit",   KBHit,                  1},
    {0}};

 jsval rval;
 Env->InitGlobal(0);
 Env->AllowExec = true;

 JS_InitStandardClasses(Env->cx, Env->global);
 Env->InitClasses();
 Env->DefineProcessFunctions();
 Env->DefineShellFunctions();
 JSObject* sys = Env->DefineSystemObject(extra_functions2);
 JS_DefineFunctions(Env->cx, Env->global, extra_functions);
 #ifdef JSOPTION_XML
 JS_SetOptions(Env->cx,JS_GetOptions(Env->cx) | JSOPTION_XML);
 #endif
 #ifdef JSOPTION_JIT
 JS_SetOptions(Env->cx,JS_GetOptions(Env->cx) | JSOPTION_JIT);
 #endif

 {
    JSObject* ret = Stream_Object(Env->cx,Env->in,false,0);
    jsval ptr = OBJECT_TO_JSVAL(ret);
    JSBool foundp = 0;
    JS_SetProperty(Env->cx, sys,"stdin",&ptr);
    JS_SetPropertyAttributes(Env->cx, sys,"stdin",JSPROP_PERMANENT|JSPROP_READONLY, &foundp);
    ret = Stream_Object(Env->cx,Env->out,false,0);
    ptr = OBJECT_TO_JSVAL(ret);
    JS_SetProperty(Env->cx, sys,"stdout",&ptr);
    JS_SetPropertyAttributes(Env->cx, sys,"stdout",JSPROP_PERMANENT|JSPROP_READONLY, &foundp);
    ptr = STRING_TO_JSVAL(JS_NewStringCopyZ(Env->cx,JSDB_VERSION));
    JS_SetProperty(Env->cx, sys,"release",&ptr);
    ptr = STRING_TO_JSVAL(JS_NewStringCopyZ(Env->cx,argv0));
    JS_SetProperty(Env->cx, sys,"program",&ptr);

#ifdef XP_WIN
    char* hn = getenv("COMPUTERNAME");
#else
    char hn[1024];
    hn[0]=0;
    gethostname(hn,1024);
#endif
    ptr = STRING_TO_JSVAL(JS_NewStringCopyZ(Env->cx,hn));
    JS_SetProperty(Env->cx, sys,"name",&ptr);

    ptr = STRING_TO_JSVAL(JS_NewStringCopyZ(Env->cx,""));
    JS_SetProperty(Env->cx, sys,"script",&ptr);

    ptr = OBJECT_TO_JSVAL(JS_NewArrayObject(Env->cx,0,0));
    JS_SetProperty(Env->cx, sys,"arguments",&ptr);
 }

 System system;
#ifdef XP_WIN
 GetDirectory(argv0,system.path);
#endif
#ifdef XP_UNIX
 GetDirectory(argv0,system.path);
// system.path="/etc/jsdb/";
#endif

 Env->System = &system;

/*
 process flags
 -load file
 -strict
 -werror
 -debug server:port
*/
bool runFromZip = false;
bool runConsole = true;
const char* execCode = 0;
int error = 0;

#ifndef JSDB_MINIMAL
if (FileExists(argv0))
{
try {
  FileStream * Program = new FileStream(argv0,Stream::OMBinary,Stream::ReadOnly);
  ZipArchive * Archive = new ZipArchive(Program,true);
  if (Archive->Count())
  {
   system.zip = Archive;
   if (system.zip->Find("main.js")!=NOT_FOUND || system.zip->Find("run.js")!=NOT_FOUND )
      runFromZip = true;
  }
  else delete Archive;
 } catch(...) {system.zip = 0;}
} else system.zip = 0;
#endif

 while (argc > 1)
 {
  char* c = argv[1];
  if (!strcasecmp(c,"-help") || !strcasecmp(c,"/help") || !strcasecmp(c,"/?"))
  {
    runConsole =  false;
    argv ++;
    argc --;
    *Env->out << "JSDB " JSDB_VERSION " " JSDB_DATE "\n"
                 JSDB_COPYRIGHT " See http://www.jsdb.org/\n";
    *Env->out << "\n"
         "JSDB [-strict] [-werror] [-load file.js] [-debug server] [lib.zip] [program.js]\n"
         "     [-path directory] [-exec code]\n"
         "\n"
         "  -strict             Strict syntax mode\n"
         "  -werror             Treat warnings as errors\n"
         "  -noxml              Disable the ECMAScript For XML extensions\n"
#ifdef JSOPTION_JIT
         "  -jit                Enable the JIT compiler (tracemonkey)\n"
         "  -nojit              Disable the JIT compiler (tracemonkey)\n"
#endif
         "  -path directory     Set the search path to directory\n"
         "  -load file.js       Load and run file.js before program.js\n"
         "  -debug server:port  Connect to a debugger server\n"
         "  -exec \"code\"        Evaluates code and exits\n"
         "  lib.zip             Set lib.zip as the default location for library files\n"
         "                      Executes main.js if it exists in lib.zip\n"
         "  program.js          Runs a JavaScript program\n"
         "                      JSDB runs console mode if no program is specified\n"
         "\n";
  }
  else if ((!strcasecmp(c,"-strict") || !strcasecmp(c,"/strict")))
  {
    JS_SetOptions(Env->cx,JS_GetOptions(Env->cx) | JSOPTION_STRICT);
    argv ++;
    argc --;
  }
  else if ((!strcasecmp(c,"-noxml") || !strcasecmp(c,"/noxml")))
  {
    JS_SetOptions(Env->cx,JS_GetOptions(Env->cx) & ~JSOPTION_XML);
    argv ++;
    argc --;
  }
  else if ((!strcasecmp(c,"-werror") || !strcasecmp(c,"/werror")))
  {
    JS_SetOptions(Env->cx,JS_GetOptions(Env->cx) | JSOPTION_WERROR);
    argv ++;
    argc --;
  }
#ifdef JSOPTION_JIT
  else if ((!strcasecmp(c,"-jit") || !strcasecmp(c,"/jit")))
  {
    JS_SetOptions(Env->cx,JS_GetOptions(Env->cx) | JSOPTION_JIT);
    argv ++;
    argc --;
  }
  else if ((!strcasecmp(c,"-nojit") || !strcasecmp(c,"/nojit")))
  {
    JS_SetOptions(Env->cx,JS_GetOptions(Env->cx) & ~JSOPTION_JIT);
    argv ++;
    argc --;
  }
#endif
  else if ((!strcasecmp(c,"-exec") || !strcasecmp(c,"/exec")))
  {
    argv ++;
    argc --;
    if (argc)
    {
     execCode = argv[1];
     argv ++;
     argc --;
    }
  }
  else if ((!strcasecmp(c,"-path") || !strcasecmp(c,"/path")))
  {
    system.path = argv[2];
    argv += 2;
    argc -= 2;
    AddBackslash(system.path);
    FixFilename(system.path);
  }
#ifndef JSDB_MINIMAL
  else if ((!strcasecmp(c,"-debug") || !strcasecmp(c,"/debug")))
  {
   TStr address(argv[2]);
   char* port = strchr(address,':');
   if (port) *port++ = 0;
   else port = (char*)"";
   Env->Debugger = JSDB_StartDebug(Env->rt, address, atoi(port));
   JS_SetOptions(Env->cx,JS_GetOptions(Env->cx) | JSOPTION_WERROR);
   argv += 2;
   argc -= 2;
  }
#endif
  else if (argc > 2 && (!strcasecmp(c,"-load") || !strcasecmp(c,"/load")))
  {
   TStr fn(argv[2]);
   argv += 2;
   argc -= 2;
   FixFilename(fn);
   if (!FileExists(fn)) fn = TStr(system.path,fn);
   if (!FileExists(fn))
   {
      error = 1;
      CGIErrorMessage(*Env->out);
      *Env->out << "Error: Can not find " << fn << "\n";
   }
   else
   {
       try {
        FileStream in(fn,Stream::OMBinary,Stream::ReadOnly);
        Env->ExecScript(0,0,in,fn,1);
       } catch(...)
       {
        error = 2;
        CGIErrorMessage(*Env->out);
        *Env->out << "Error: Unable to open " << fn << "\n";
        break;
       }
   }
   continue;
  }
#ifndef JSDB_MINIMAL
  else if (system.zip == 0 && (stristr(c,".zip")||stristr(c,".jar")))
  {
    try {
    FileStream * Program = new FileStream(c,Stream::OMBinary,Stream::ReadOnly);
    ZipArchive * Archive = new ZipArchive(Program,true);
    system.zip = Archive;
    if (system.zip->Find("main.js")!=NOT_FOUND || system.zip->Find("run.js")!=NOT_FOUND )
      runFromZip = true;

    argv ++;
    argc --;
   } catch(...) {system.zip=0;}
  }
#endif
 else break;
 }

TStr filename;
if (runFromZip == false && argc > 1)
{
  filename = argv[1];
  FixFilename(filename);
  argc --;
  argv ++;
}

jsval * arr;
if (argc > 1)
{
    arr = new jsval[argc-1];
    for (int i=1; i< argc; i++)
    {
        arr[i-1] = STRING_TO_JSVAL(JS_NewStringCopyZ(Env->cx,argv[i]));
    }
    JSObject * ret = (JS_NewArrayObject(Env->cx,argc-1,arr));
    jsval ptr = OBJECT_TO_JSVAL(ret);
    JSBool foundp = 0;
    JS_SetProperty(Env->cx, Env->global,"jsArguments",&ptr);
    JS_SetPropertyAttributes(Env->cx, Env->global,"jsArguments",JSPROP_PERMANENT, &foundp);
    JS_SetProperty(Env->cx, sys,"arguments",&ptr);
    JS_SetPropertyAttributes(Env->cx, sys,"arguments",JSPROP_PERMANENT, &foundp);
    if (arr) delete [] arr;
}
else
{
  jsval ptr = OBJECT_TO_JSVAL(JS_NewArrayObject(Env->cx,0,0));
  JS_SetProperty(Env->cx, Env->global,"jsArguments",&ptr);
  JS_SetProperty(Env->cx, sys,"arguments",&ptr);
}

#ifndef JSDB_MINIMAL
 if (runFromZip)
 {
  MemoryStream in;
  const char * name = "main.js";
  if (!system.zip->Extract(system.zip->Find(name),in))
  {
       name = "run.js";
       system.zip->Extract(system.zip->Find(name),in);
  }
  in.rewind();

  jsval ptr = STRING_TO_JSVAL(JS_NewStringCopyZ(Env->cx,name));
  JS_SetProperty(Env->cx, sys,"script",&ptr);
  Env->ExecScript(0,0,in,name,1);
 }
 else
#endif
 if (*filename)
 {
  if (FileExists(filename))
  { // ExecScript skips the shebang #!
  try {
    FileStream in(filename,Stream::OMBinary,Stream::ReadOnly);
    jsval ptr = STRING_TO_JSVAL(JS_NewStringCopyZ(Env->cx,filename));
    JS_SetProperty(Env->cx, sys,"script",&ptr);
    Env->ExecScript(0,0,in,filename,1);
   } catch(...)
   {
       error = 2;
       CGIErrorMessage(*Env->out);
       *Env->out << "Error: Unable to open " << filename << "\n";
   }
  }
  else
  {
      error = 1;
      CGIErrorMessage(*Env->out);
      *Env->out << "Error: Can not find " << filename << "\n";
  }
 }
 else if (!error && runConsole)
 {
  StartConsole(Env);
 }

// Cleanup!
 JS_GC(Env->cx);
 JS_DestroyContext(Env->cx);
 JS_DestroyRuntime(Env->rt);

 if (Env->in != Env->out)
   delete Env->in;

 delete Env->out;
 Env = 0; //Env is a smart pointer, assigning NULL forces cleanup

#ifdef XP_WIN
 CoFreeUnusedLibraries();
 CoUninitialize();
#endif
 JS_ShutDown();
 return error;
}
//wrapper function macros

void ExecCommand(JSDBEnvironment* Env,const char * cmd,size_t len,uintN line)
{
#ifndef JSDB_MINIMAL
  if (Env->Debugger)
   DebugScriptSource(Env->Debugger,"console",line,cmd,len);
#endif

  jsval result;
  JSString* str;
  JSBool ok = JS_EvaluateScript(Env->cx, Env->global, cmd, len, "console", line, &result);

  if (ok && result != JSVAL_VOID && result != 0)
  { /* Suppress error reports from JS_ValueToString(). */
    JSErrorReporter older;
    older = JS_SetErrorReporter(Env->cx, NULL);
    str = JS_ValueToString(Env->cx, result);
    JS_SetErrorReporter(Env->cx, older);

    if (str)
    {
     const char * c = JS_GetStringBytes(str);
     if (*c)
     {
      Env->out->writestr(c);
      if (c[strlen(c)-1] != '\n')
       Env->out->writestr("\n");
     }
    }
    else Env->out->writestr("\n");
  }
}

#ifdef __sun__
/* wrap_shell2.cpp */
extern int isdir(const char* dir, const char* file);
#endif

void PrintDirectory(Stream& out, const char * filter)
{
#ifdef XP_WIN
 WIN32_FIND_DATA ff;
 if (!filter || !*filter) filter = "*.*";
 HANDLE start = FindFirstFile(filter,&ff);
 if (start == INVALID_HANDLE_VALUE) return;
 char space[80];
 memset(space,' ',80);
 int i =0;
 do {
  if (ff.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) continue;
  if (ff.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) continue;
  if (ff.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) continue;

  out << ff.cFileName;
  i++;
  size_t len = strlen(ff.cFileName);
  if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
   {out << "/"; len++;}

 if (len < 40) out.write(space,40-len);
 else if (len % 40) out.writestr("\n");
//  if ((i%2)==0) out << "\n";

 } while (FindNextFile(start,&ff));

 if (i % 2) out.writestr("\n");
 FindClose(start);
#endif
#ifdef XP_UNIX
DIR *ff;
 if (!filter || !*filter) filter = ".";
 ff=opendir(filter);
 if (ff==NULL) { out << "Can't open " << filter << "\n"; return;}

 struct dirent *dr;

 char space[80];
 memset(space,' ',80);
 int i =0;
 do {
  dr = readdir(ff);
  if (dr == NULL) { break; }
  if (dr->d_name == NULL) { break; }
  out << dr->d_name;
#ifdef __sun__
  if (isdir(filter,dr->d_name))
#else
  if(dr->d_type == 4)
#endif
   { out << "/"; }
  out << "\n";
 } while (dr != NULL);

 closedir(ff);
#endif
}

void StartConsole(JSDBEnvironment*Env)
{
 MemoryStream code;
#ifdef XP_WIN
 char line[1024];
#endif
#ifdef XP_UNIX
 TStr line;
 char pText[32];
#endif
 uintN startline = 1;
 bool prompt = true;

 while (!Env->shouldStop)
 {
   if (prompt)
   {
#ifdef XP_WIN
     *Env->out << "js>";
#endif
#ifdef XP_UNIX
     strcpy(pText,"js>");
#endif
   }
   else
   {
#ifdef XP_WIN
     *Env->out << startline << ": ";
#endif
#ifdef XP_UNIX
    sprintf(pText,"%d: ",startline);
#endif
   }

#ifdef XP_WIN
   fgets(line,1024,stdin);
#endif
#ifdef XP_UNIX
   fflush(stdout);
    char* l = readline(pText);
    line = l;
    add_history(l);
    free(l);
#endif
   size_t len = strlen(line);

   if (len == 0) continue;

   if (line[len-1] == '\n') len --;
   if (line[len-1] == '\r') len --;

   if (!len) continue;

   if (prompt)
   {
    if ((len==3 && !strncasecmp(line,"dir",3)) || (len==2 && !strncasecmp(line,"ls",2)) ||
        !strncasecmp(line,"dir ",4) || !strncasecmp(line,"ls ",3) )
     {
      char * filter = strchr(line,' ');
      if (filter) filter ++;
      line[len] = 0;
      PrintDirectory(*Env->out,filter);
      continue;
     }

    if (!strncasecmp(line,"cd ",3))
     {
      char * filter = strchr(line,' ');
      if (filter) filter ++;
      line[len] = 0;
      ChangeDirectory(filter);
      continue;
     }
   if (len == 4 && !strncasecmp(line,"quit",4) || !strncasecmp(line,"exit",4))
    {
     Env->shouldStop = true;
     continue;
    }

   }

   if (!prompt) code.write("\r\n",2);
   code.write(line,len);

   if (JS_BufferIsCompilableUnit(Env->cx, Env->global, code, code.size()))
     {
      ExecCommand(Env,code,code.size(),startline);
      code.Clear();
      prompt = true;
      if (!Env->Debugger) startline = 1;
     }
    else
     {
      startline ++;
      prompt = false;
     }
 }
}

