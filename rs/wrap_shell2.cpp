#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

#ifndef XP_WIN
#include <fnmatch.h>
#endif
#include <time.h>
#include "js/src/jsdate.h"
//#include "js/jsconfig.h" // missing in Tracemonkey

// terrible hack because of WIN32_LEAN_AND_MEAN=1 defined in tracemonkey
#ifdef XP_WIN
#undef _INC_SHELLAPI
#include <shellapi.h>
#endif

/**
JSBool Load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;
 if (Env->SafeMode) RETBOOL(false);

 if (argc < 1) RETBOOL(false);
 JSString *str;
 JSBool ok = JS_FALSE;

   Stream* data;

   GETFILE(0,data);

   try
   {
    if (data)
    {
     int line = 1;
     if (argc > 1) if (ISSTR(1)) filename = STR(1);
     if (argc > 2) if (ISINT(2)) line = INT(2);
     return JSDB_ExecScript(Env,cx,*data,data->filename(),line);
    }
    else
    {
     if (!ISSTR(0)) RETBOOL(false);
     FileStream in(STR(0),Stream::OMBinary,Stream::ReadOnly);
     return JSDB_ExecScript(Env,cx,in,in.filename(),1);
    }
   }
   catch(xdb& x)
   {
    ERR_MSG(Load,"File open failure",TStr(x.why(), x.info()));
   }
}
*/
// ---

JSBool IsSafe(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;
 RETBOOL(Env->SafeMode);
}

static JSBool
SafeMode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;
 int key = 0;
 if (argc > 0 && ISINT(0)) key = INT(0);
 if (Env->SafeMode)
  {
   if (key == Env->SafeMode)
    Env->SafeMode = 0;
   RETINT(0);
  }

 Env->SafeMode = clock() ^ Env->Magic;
 if (Env->SafeMode == 0) Env->SafeMode = clock() ^ Env->Magic;
 Env->Magic = Env->SafeMode;
 RETINT(Env->SafeMode);
}

static JSBool
ShouldStop(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    GETENV;
    if (Env->checkInterrupt != NULL) Env->checkInterrupt(Env);
    JS_MaybeGC(Env->cx);

    //if (!Env->shouldStop) JS_GC(Env->cx);
    //garbage collection is always good now and again.

    RETBOOL(Env->shouldStop);
}


static JSBool

Restart(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
   GETENV;
   Env->shouldStop = true;
   Env->restart = true;

   return JS_TRUE;
}

static JSBool
GoToSleep(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
JS_MaybeGC(cx);
 GETENV;
 if (Env->shouldStop) return JS_TRUE;
 int32 ms = 0;
 if (argc) TOINT(0,ms);
 if (ms < 0) ms = 0;
 if (Env->checkInterrupt != NULL) Env->checkInterrupt(Env);
#ifdef XP_WIN
 if (!Env->shouldStop) SleepEx(ms,true);
#else
 if (!Env->shouldStop) usleep(ms*1000);
#endif
 return JS_TRUE;
}

static JSBool
SetCWD(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    GETENV;
    if (Env->SafeMode) RETSTRW(L"");
    #ifdef XP_WIN
    jschar path[1024];
    if (!GetCurrentDirectoryW(1023,(wchar_t*)path)) path[0]=0;
    path[1023]=0;

    if (argc > 0 && ISSTR(0))
     SetCurrentDirectoryW((const wchar_t*)WSTR(0));

    RETSTRW(path);
    #else
    char path[1024];
    if (!getcwd(path,1023)) path[0]=0;
    path[1023]=0;

    if (argc > 0 && ISSTR(0))
    {
     chdir(STR(0));
    }

    RETSTR(path);
    #endif
}

static JSBool
GetEnv(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (argc == 0) ERR_COUNT(system,getenv);
    if (!ISSTR(0)) ERR_TYPE(system,getev,1,String);

    RETSTR(getenv(STR(0)));
}

static JSBool
Quit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
   GETENV;
   Env->shouldStop = true;
   Env->restart = false;

   return JS_TRUE;
}

#ifdef XP_WIN

HKEY ResolveRoot(const char* root)
{
      if (!strncasecmp(root,"HKEY_LOCAL_MACHIN",17)) return HKEY_LOCAL_MACHINE;
 else if (!strncasecmp(root,"HKEY_CURRENT_USER",17)) return HKEY_CURRENT_USER;
 else if (!strncasecmp(root,"HKEY_CLASSES_ROOT",17)) return HKEY_CLASSES_ROOT;
 else return 0;
}

//regSetKey("hkey_local_machine\SOFTWARE\...","key","value");
static JSBool
RegSetKey(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 3) ERR_COUNT(JSDB,RegSetKey);
 if (!ISSTR(0)) ERR_TYPE(JSDB,RegSetKey,1,String);
 if (!ISSTR(1)) ERR_TYPE(JSDB,RegSetKey,2,String);

 GETENV;
 if (Env->SafeMode) RETBOOL(false);

 const char* root = STR(0);
 TStr section(root);
 Replace(section,'/','\\');
 char* code = strchr(section,'\\');
 if (code) code++;
 HKEY r = ResolveRoot(root);

 JSString* j2;
 char * s2;
 GETSTRING(2)

 RETBOOL(RegSetKey(code,STR(1),s2,r));
}


//regGetKey("hkey_local_machine\SOFTWARE\...","key");
static JSBool
RegGetKey(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc < 1) ERR_COUNT(JSDB,RegGetKey);
 if (!ISSTR(0)) ERR_TYPE(JSDB,RegGetKey,1,String);
 if (argc > 1 && !ISSTR(1)) ERR_TYPE(JSDB,RegGetKey,2,String);

 GETENV;
 if (Env->SafeMode) RETBOOL(false);

 const char* root = STR(0);
 TStr section(root);
 Replace(section,'/','\\');
 char* code = strchr(section,'\\');
 if (code) code++;
 HKEY r = ResolveRoot(root);

 TStr s;
 RegGetKey(code,argc > 1 ? STR(1) : 0,s,r);
 RETSTR(s);
}
#endif

static JSBool
ShellBrowse(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 1) ERR_COUNT(JSDB,ShellBrowse);
 if (!ISSTR(0)) ERR_TYPE(JSDB,ShellBrowse,1,String);

 GETENV;
 if (Env->SafeMode) RETBOOL(false);

 char * file = STR(0);

#ifdef XP_WIN
 GETUCS2(0);
 //TStr s();
 //RegGetKey("htmlfile\\shell\\open\\command","",s,HKEY_CLASSES_ROOT);
 int x = (int)ShellExecuteW(GetFocus(),L"open",(WCHAR*)s0,NULL,NULL,SW_SHOWNORMAL);

 RETINT(x);
#else
 GETUTF8(0);
 int browsr=0;
 const char* b = "firefox";

 if (ISINT(1))
     switch(INT(1))
     {
         case 1: b = "mozilla"; break;
         case 2: b = "kfmclient exec"; break;
         case 3: b = "opera";
     }
 else if (ISSTR(1))
     b = STR(1);

 char exeC[1024];
 *exeC=0;

 if (strlen(b) + strlen(u0) < 900)
   sprintf(exeC,"%s %s >/dev/null 2>&1",b,(char*)u0);
 else
    RETINT(-1);

 RETINT(system(exeC));
#endif
}

static JSBool StartDebugger(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
#ifdef JSDB_MINIMAL
 RETBOOL(false);
#else
JS_MaybeGC(cx);
 GETENV;
 if (argc == 0 || !ISSTR(0))
  {
   if (Env->Debugger)
   {
    JSDB_EndDebug(Env->rt,Env->Debugger);
    Env->Debugger = 0;
   }
   RETBOOL(false);
  }
 if (Env->SafeMode) RETBOOL(false);
 TStr address(STR(0));
 char* port = strchr(address,':');
 if (port) *port++ = 0;
 else port = (char*)"";
 if (Env->Debugger) JSDB_EndDebug(Env->rt,Env->Debugger);
 Env->Debugger = JSDB_StartDebug(Env->rt, address, atoi(port));
 RETBOOL(Env->Debugger != NULL);
#endif
}

#ifdef __sun__
extern int isdir(const char* dir, const char* file)
{
     struct stat ms;
     int x = stat(TStr(dir,"/",file),&ms);
     return (ms.st_mode&S_IFMT) == S_IFDIR;
}
#endif

extern jsval* nameList(JSContext *cx,Strings& names, int& count);

static JSBool
directoryList(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval,bool dirs)
{
 GETENV;
 if (Env->SafeMode) RETBOOL(false);
 ENTERNATIVE(cx);

 TWORMList files(2048);
#ifdef XP_WIN
 const wchar_t* search = L"*.*";
 JSString* j0 = argc > 0 ? JS_ValueToString(cx,argv[0]) : 0;
 wchar_t* s0 = (j0) ? (wchar_t*)JS_GetStringChars(j0) : (wchar_t*)0;
 if (s0) search = s0;

 WIN32_FIND_DATAW FindData;
 int skip = FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_OFFLINE;

 HANDLE finder = FindFirstFileW(search,&FindData);
 if (finder != INVALID_HANDLE_VALUE)
 {
  do
  {
   if (FindData.cFileName[0] != L'.' &&
       (FindData.dwFileAttributes & skip) == 0)
       if ((dirs && ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)) ||
           (!dirs && ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) ))
          files.Add(TStr((jschar*)FindData.cFileName));
  } while (FindNextFileW(finder,&FindData));

  FindClose(finder);
 }
#else
 GETUTF8(0);
 const char* search = "*";
 const char* dirname = ".";
 if (*u0)
 {
     char* e = strrchr(u0,'/');
     if (e)// "/home/shanti/*.txt"
     {
       *e++ = 0;
       dirname = u0;
       if (*e) search = e;
     }
     else
       search = u0;
 }

 DIR *ff = opendir(dirname);
 if (ff == NULL)
   RETBOOL(false);

 struct dirent *dr;

 do {
  dr = readdir(ff);
  if (dr)
#ifdef __sun__
  if (dirs == isdir(search,dr->d_name))
#else
  if (dirs == (dr->d_type==4))
#endif
  if (fnmatch(search,dr->d_name,0) == 0)
   if (strcmp(dr->d_name,".") && strcmp(dr->d_name,".."))
    files.Add(dr->d_name);
 } while (dr!=NULL);

 closedir(ff);
#endif

 if (!files.Count())
  RETOBJ(JS_NewArrayObject(cx,0,0));

 int count = 0;
 jsval * arr = nameList(cx, files, count);

 JSObject * ret = JS_NewArrayObject(cx,files.Count(),arr);
 delete[] arr;
 RETOBJ(ret);
}

static JSBool
listFiles(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 return directoryList(cx, obj, argc, argv, rval,false);
}

static JSBool
listDirs(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 return directoryList(cx, obj, argc, argv, rval,true);
}

static JSBool
jsCopyFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 2) ERR_COUNT(JSDB,CopyFile);
 if (!ISSTR(0)) ERR_TYPE(JSDB,CopyFile,1,String);
 if (!ISSTR(1)) ERR_TYPE(JSDB,CopyFile,2,String);
 GETENV;
 if (Env->SafeMode) RETBOOL(false);

#ifdef XP_WIN
 GETUCS2(0);
 GETUCS2(1);
 WStr o(s0);
 WStr n(s1);
 Replace(o,'/','\\');
 Replace(n,'/','\\');
 RETBOOL(CopyFileW(o,n,false));
#else
 GETUTF8(0);
 GETUTF8(1);
 RETBOOL(CopyFile(u0, u1, false));
#endif
}

static JSBool
jsDeleteFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc < 1) ERR_COUNT(JSDB,Remove);
 if (!ISSTR(0)) ERR_TYPE(JSDB,Remove,1,String);
 GETENV;
 if (Env->SafeMode) RETBOOL(false);
#ifdef XP_WIN
 GETUCS2(0);
 WStr o(s0);
 Replace(o,'/','\\');
 WIN32_FILE_ATTRIBUTE_DATA attr;
 if (!GetFileAttributesExW(o,GetFileExInfoStandard,&attr)) RETBOOL(false);
 if (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
 {
  RETBOOL(RemoveDirectoryW(o));
 }
 else
 {
   RETBOOL(DeleteFileW(o));
 }
#else
 GETUTF8(0);
 RETBOOL((remove(u0)==0));
#endif
}

static JSBool
jsMoveFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc < 2) ERR_COUNT(JSDB,MoveFile);
 if (!ISSTR(0)) ERR_TYPE(JSDB,MoveFile,1,String);
 if (!ISSTR(1)) ERR_TYPE(JSDB,MoveFile,2,String);
 GETENV;
 if (Env->SafeMode) RETBOOL(false);

#ifdef XP_WIN
 JSBool replace = 0;
 if (argc > 2) TOBOOL(2,replace);

 GETUCS2(0);
 GETUCS2(1);
 WStr o(s0);
 WStr n(s1);
 Replace(o,'/','\\');
 Replace(n,'/','\\');
 DWORD opts = MOVEFILE_COPY_ALLOWED;
 if (replace) opts |= MOVEFILE_REPLACE_EXISTING;

 RETBOOL(MoveFileExW(o,n,opts));
#else
 GETUTF8(0);
 GETUTF8(1);
 RETBOOL((rename(u0,u1)==0));
#endif
}

static JSBool
FileExists(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 1) ERR_COUNT(JSDB,FileExists);
 if (!ISSTR(0)) ERR_TYPE(JSDB,FileExists,1,String);
 GETENV;

 GETUTF8(0);
#ifdef XP_WIN
 Replace(u0,'/','\\');
#endif
 RETBOOL(FileExists(u0));
}


/** returns an object with file information:
  attributes:
  date: last modified date
  creation: creation date, if available
  attributes:
  size
*/

static JSBool
FileStatus(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 1) ERR_COUNT(JSDB,FileStatus);
 if (!ISSTR(0)) ERR_TYPE(JSDB,FileStatus,1,String);
 GETENV;

#ifdef XP_WIN
 GETUCS2(0);
 if (!s0 || !*s0) RETOBJ(NULL);
 WStr o(s0);
 Replace(o,'/','\\');

 WIN32_FILE_ATTRIBUTE_DATA attr;
 if (!GetFileAttributesExW(o,GetFileExInfoStandard,&attr)) RETOBJ(NULL);

 JSObject* r = JS_NewObject(cx,NULL,NULL,NULL);
 JS_AddRoot(cx, &r);
 MemoryStream attrs;

 if (attr.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) attrs << "archive,";
 if (attr.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) attrs << "compressed,";
 if (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) attrs << "directory,";
 if (attr.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) attrs << "hidden,";
 if (attr.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) attrs << "offline,";
 if (attr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) attrs << "readonly,";
 if (attr.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) attrs << "system,";
 if (attr.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) attrs << "temporary,";
#else

 GETUTF8(0);
 JSObject* r = JS_NewObject(cx,NULL,NULL,NULL);
 JS_AddRoot(cx, &r);
 MemoryStream attrs;

 struct stat ms;
 int x = stat(u0,&ms);
 if (x == -1)
   RETOBJ(NULL);
 if (S_ISDIR(ms.st_mode)) attrs << "directory,";
 if (S_ISLNK(ms.st_mode)) attrs << "symlink,";
 if (S_ISREG(ms.st_mode)) attrs << "regular,";
 if (S_ISCHR(ms.st_mode)) attrs << "chardev,";
 if (S_ISBLK(ms.st_mode)) attrs << "blockdev,";
 if (S_ISFIFO(ms.st_mode)) attrs << "fifo,";
 if (S_ISSOCK(ms.st_mode)) attrs << "socket,";

// if (mystat.st_mode & S_IFMT == S_IWRITE) return 1; //read only
// return JS_FALSE; //read and write
#endif
 JSString * s = JS_NewStringCopyZ(cx,attrs);
 jsval val = STRING_TO_JSVAL(s);
 JS_SetProperty(cx,r,"attributes",&val);

#ifdef XP_WIN
 if (attr.nFileSizeHigh == 0 && INT_FITS_IN_JSVAL(attr.nFileSizeLow))
   val = INT_TO_JSVAL(attr.nFileSizeLow);
 else
 {
  double d = (long)1 << 16;
  d = d * d * (double)(unsigned long)(attr.nFileSizeHigh) + (double)(unsigned long)attr.nFileSizeLow;
  //val = DOUBLE_TO_JSVAL(d);
  JS_NewNumberValue(cx,d,&val);
 }
#else
   val = INT_TO_JSVAL(ms.st_size);
#endif
 JS_SetProperty(cx,r,"size",&val);

#ifdef XP_WIN
 SYSTEMTIME utc;
 SYSTEMTIME t;
 FileTimeToSystemTime(&attr.ftLastWriteTime,&utc);
 SystemTimeToTzSpecificLocalTime(NULL,&utc,&t);
 JSObject* d = js_NewDateObject(cx,t.wYear,t.wMonth-1,t.wDay,t.wHour,t.wMinute,t.wSecond);
 val = OBJECT_TO_JSVAL(d);
 JS_SetProperty(cx,r,"date",&val);

 FileTimeToSystemTime(&attr.ftCreationTime,&utc);
 SystemTimeToTzSpecificLocalTime(NULL,&utc,&t);
 d = js_NewDateObject(cx,t.wYear,t.wMonth-1,t.wDay,t.wHour,t.wMinute,t.wSecond);
 val = OBJECT_TO_JSVAL(d);
 JS_SetProperty(cx,r,"creation",&val);
#else
 struct tm t;

 t=*localtime(&ms.st_mtime);
 JSObject* d = js_NewDateObject(cx,t.tm_year+1900,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
 val = OBJECT_TO_JSVAL(d);
 JS_SetProperty(cx,r,"date",&val);

 t=*localtime(&ms.st_ctime);
 d = js_NewDateObject(cx,t.tm_year+1900,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
 val = OBJECT_TO_JSVAL(d);
 JS_SetProperty(cx,r,"creation",&val);
#endif


 JS_RemoveRoot(cx,&r);
 RETOBJ(r);
}

static JSBool
ShellExec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
JS_MaybeGC(cx);
 GETENV;
 if (argc == 0) ERR_COUNT(JSDB,ShellExec);
 if (!ISSTR(0)) ERR_TYPE(JSDB,ShellExec,1,String);
 if (!Env->AllowExec) RETBOOL(false);

#ifdef XP_WIN
 char* params = 0;
 GETUCS2(0);
 GETUCS2(1);
 GETUCS2(2);

 HINSTANCE proc =
    ShellExecuteW(NULL,NULL,(WCHAR*)s0,(WCHAR*)s1,(WCHAR*)s2,SW_SHOWNORMAL);

 RETBOOL((int)proc > 32);
#else
 GETUTF8(0);
 RETBOOL(system(u0) != -1);
#endif
}

static JSBool
shell_JSSet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 int x = JSVAL_TO_INT(id);
 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 1: return SetCWD(cx, obj, 1, rval, rval);
  }
 return JS_FALSE;
}

static JSBool
shell_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 int x = JSVAL_TO_INT(id);
 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 1: return SetCWD(cx, obj, 0,0,rval);

   case 2:
   #ifdef XP_WIN
   RETINT(GetCurrentProcessId());
   #else
   RETINT(getpid());
   #endif
  }
 return JS_FALSE;
}

static JSFunctionSpec shell_functions2[] = {
    {"sleep",           GoToSleep,      0},
    {"quit",            Quit,           0},
    {"exit",            Quit,           0},
    {"jsShouldStop",      ShouldStop,     0},
    {"jsRestart",         Restart,        0},
    {"jsSafeMode",      SafeMode, 0},
    {"jsIsSafe",        IsSafe,0},
    {0}
};

static JSPropertySpec shell_properties[] = {
    {"cwd",1, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,shell_JSGet,shell_JSSet},
    {"pid",2, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,shell_JSGet},
    {0}
};

static JSFunctionSpec shell_functions3[] = {
    {"copyFile",        jsCopyFile,       2},
    {"moveFile",        jsMoveFile,       2},
    {"openBrowser",     ShellBrowse,    1},
    {"fileExists",      FileExists,     1},
#ifdef XP_WIN
    {"regGetKey",       RegGetKey,     2},
    {"regSetKey",       RegSetKey,     3},
#endif
    {"listFiles",       listFiles,           1},
    {"listFolders",       listDirs,           1},
    {"listDirectories",       listDirs,           1},
    {"jsShellExec",        ShellExec,3},
    {"jsDebug",         StartDebugger,           0},
    {0}
};

extern JSBool
Options(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

extern JSBool
BuildDate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

extern JSBool
Version(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

static JSFunctionSpec system_functions[] = {
    {"attributes",     FileStatus, 0},
    {"browse",     ShellBrowse,    1},
//    {"buildDate",       BuildDate,    0},
    {"copy",        jsCopyFile,       2},
    {"debug",         StartDebugger,           0},
    {"remove",       jsDeleteFile,           1},
    {"directories",       listDirs,           1},
    {"execute",        ShellExec,3},
    {"exists",      FileExists,     1},
    {"exit",            Quit,           0},
    {"files",       listFiles,           1},
    {"folders",       listDirs,           1},
    {"getenv",          GetEnv, 1},
    {"setcwd",          SetCWD, 1},
    {"getcwd",          SetCWD, 0},
#ifdef XP_WIN
    {"getKey",       RegGetKey,     2},
    {"setKey",       RegSetKey,     3},
#endif
    {"isSafe",        IsSafe,0},
    {"move",        jsMoveFile,       2},
    {"options",         Options,      2},
    {"quit",            Quit,           0},
    {"restart",         Restart,        0},
    {"safeMode",      SafeMode, 0},
    {"shouldStop",      ShouldStop,     0},
    {"sleep",           GoToSleep,      0},
//    {"version",         Version,      0},
    {0}
};

JSObject* JSDBEnvironment::DefineSystemObject(JSFunctionSpec* extra)
{
 JSObject * o= JS_NewObject(cx, NULL, NULL, NULL);
 jsval val = OBJECT_TO_JSVAL(o);
 JSBool foundp=0;
 JS_DefineFunctions(cx, o, system_functions);
 if (extra) JS_DefineFunctions(cx, o, extra);
 JS_SetProperty(cx, global, "system", &val);
 JS_SetPropertyAttributes(cx, global,"system",JSPROP_PERMANENT, &foundp);
 jsval ptr = INT_TO_JSVAL(JS_VERSION);
 JS_SetProperty(cx, o,"version",&ptr);
 ptr = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,__DATE__));
 JS_SetProperty(cx, o,"buildDate",&ptr);

 JS_DefineProperties(cx,o,shell_properties);

 return o;
}

void JSDBEnvironment::DefineProcessFunctions()
{
 JS_DefineFunctions(cx, global, shell_functions2);
}

void JSDBEnvironment::DefineShellFunctions()
{
 JS_DefineFunctions(cx, global, shell_functions3);
}
