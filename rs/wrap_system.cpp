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
JSBool Load)
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

NATIVE(IsSafe)
{
 GETENV;
 RETBOOL(Env->SafeMode);
}

NATIVE(SafeMode)
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

NATIVE(ShouldStop)
{
    GETENV;
    if (Env->checkInterrupt != NULL) Env->checkInterrupt(Env);
    JS_MaybeGC(Env->cx);

    //if (!Env->shouldStop) JS_GC(Env->cx);
    //garbage collection is always good now and again.

    RETBOOL(Env->shouldStop);
}


NATIVE(Restart)
{
   GETENV;
   Env->shouldStop = true;
   Env->restart = true;

   return JS_TRUE;
}

NATIVE(GoToSleep)
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

NATIVE(SetCWD)
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

NATIVE(GetEnv)
{
    if (argc == 0) ERR_COUNT(system,getenv);
    if (!ISSTR(0)) ERR_TYPE(system,getev,1,String);

    RETSTR(getenv(STR(0)));
}

NATIVE(Quit)
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
NATIVE(RegSetKey)
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
NATIVE(RegGetKey)
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

NATIVE(ShellBrowse)
{
 if (argc == 0) ERR_COUNT(JSDB,ShellBrowse);
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
#ifdef XP_MACOSX
 const char* b = "open";
#else
 const char* b = "firefox";
#endif

 if (argc > 1)
 {
     if (ISINT(1))
     switch(INT(1))
     {
#ifdef XP_MACOSX
         case 3: b = "open -a Opera";break;
         case 2: b = "open -a Safari";break;
         case 1: b = "open -a Seamonkey";
#else
         case 1: b = "seamonkey"; break;
         case 2: b = "kfmclient exec"; break;
         case 3: b = "opera";
#endif
      }
 else if (ISSTR(1))
     b = STR(1);
}

 TStr exeC(b," ",(char*)u0," & >/dev/null 2>&1");

 RETINT(system(exeC));
#endif
}

NATIVE(StartDebugger)
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

static JSBool directoryList(JSContext *cx, uintN argc, jsval *argv, jsval *rval,bool dirs)
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

NATIVE(listFiles)
{GETARGS;
 return directoryList(cx, argc, argv, RVAL,false);
}

NATIVE(listDirs)
{GETARGS;
 return directoryList(cx, argc, argv, RVAL,true);
}

NATIVE(jsCopyFile)
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

NATIVE(jsDeleteFile)
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

NATIVE(jsMoveFile)
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

NATIVE(FileExists)
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

NATIVE(FileStatus)
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

/* wait for streams to have input data, servers to have requests pending*/

int GetSelectable(JSContext* cx,JSObject* obj,size_t childCount = 0)
{
    if (JS_InstanceOf(cx,obj,Stream_Class(),0))
    {
//printf("Stream %x\n",obj);
        Stream* s = GETPRIVATE(Stream,obj);
        InternetStream * is = TYPESAFE_DOWNCAST(s,InternetStream);
        if (is) return is->s;
    }
    if (JS_InstanceOf(cx,obj,Server_Class(),0))
    {
//printf("Server %x\n",obj);
        InternetServer* is = GETPRIVATE(InternetServer,obj);
        if (is) return is->s;
    }
    jsval tso = 0;
    if (childCount < 8) //limit loop and recursion depth
    {
     if (JS_CallFunctionName(cx, obj, "toStream", 0, NULL, &tso))
    {
        JSObject* o = JSVAL_TO_OBJECT(tso);
//printf("toStream %x\n",o);
        if (JSVAL_IS_OBJECT(tso) && o != obj) return GetSelectable(cx,o,childCount+1);
    }
     if (JS_CallFunctionName(cx, obj, "toServer", 0, NULL, &tso))
    {
        JSObject* o = JSVAL_TO_OBJECT(tso);
//printf("toServer %x\n",o);
        if (JSVAL_IS_OBJECT(tso) && o != obj) return GetSelectable(cx,o,childCount+1);
    }
}
    return 0;
}

/*
build a list of sockets
test canRead for each object
if any ready, return the list immediately
if not, wait on those which are sockets
if any ready, return the list
if none, test canRead again
if any ready, return the list immediately
return NULL
*/

#ifndef FD_SETSIZE
#define FD_SETSIZE 64
#endif

//clears all socket values which are not readable
//returns the number of signaled (readable) sockets
int MultiSelect(int* sockets, bool* ready, unsigned count, unsigned length, int timeout)
{
    fd_set incoming;
    FD_ZERO(&incoming);

    int blocks = 1 + (count / FD_SETSIZE);

    timeval t;
    unsigned usec = timeout * 1000 / blocks;
    t.tv_sec = usec / 1000000;
    t.tv_usec = usec % 1000000;
    int s = 0;
    int result = 0;
    unsigned i=0, j=0, k=0, l=0;

    //test up to 64 sockets at a time
    for (; j<blocks; j++)
    {
        //remember where we started
        l=i;
        //find the next 64 sockets in the list
        for (; i<length; i++)
        {
         if (sockets[i])
         {
            if (sockets[i] > s) s = sockets[i];
            FD_SET(sockets[i],&incoming);
            if (++k > FD_SETSIZE)
            {
                k=0;
                break;
            }
         }
        }

        //test those sockets for incoming data
        if (select(s+1,&incoming,0,0,timeout<0?NULL:&t) == -1) return 0;

        //go back and clear the non-ready sockets
        for (; l < i; l++)
        {
         if (sockets[l])
         {
             if (FD_ISSET(sockets[l],&incoming))
             {
                  ready[l] = true;
                  t.tv_sec = t.tv_usec = 0;
                  result ++;
             }
         }
        }
    }
    return result;
}

NATIVE(systemWait)
{
 JS_MaybeGC(cx);
 if (argc == 0 || !JSVAL_IS_OBJECT(ARGV(0))) RETOBJ(NULL);

 JSObject* w = JSVAL_TO_OBJECT(ARGV(0));
 if (!w) RETOBJ(NULL);

 jsuint i=0, j=0, k=0;
 jsuint length=1;
 bool isArray = JS_IsArrayObject(cx,w);
 if (isArray)
 {
  JS_GetArrayLength(cx,w,&length);
 }
//printf("wait %d %d objects\n",isArray,length);
 int timeout = -1; //10 seconds
 if (argc>1)
  TOINT(1,timeout);

 JSObject** objs = new JSObject*[length];
 JSObject* ret = 0;
 int* sockets = new int[length];
 bool* ready = new bool[length];
 int socketCount = 0;
 int readyCount = 0;

 //identify object types
 if (isArray)
 for (i=0; i<length; i++)
 {
     jsval val;
     JS_GetElement(cx,w,i,&val);
     objs[i] = JSVAL_IS_OBJECT(val) ? JSVAL_TO_OBJECT(val) : 0;
     sockets[i] = objs[i] ? GetSelectable(cx,objs[i]) : 0;
     if (sockets[i]) socketCount ++;
     ready[i] = 0;
//printf("%d %x %d %d\n",i,objs[i],sockets[i],ready[i]);
 }
 else
 {
//printf("wait not array\n",length);
     i=0;
     jsval val = ARGV(0);
     objs[i] = JSVAL_IS_OBJECT(val) ? JSVAL_TO_OBJECT(val) : 0;
     sockets[i] = objs[i] ? GetSelectable(cx,objs[i]) : 0;
     if (sockets[i]) socketCount ++;
     ready[i] = 0;
 }
 //look for a canRead function
 for (i=0; i<length; i++)
 {
     if (objs[i] && !sockets[i])
     {
       jsval val;
       if (JS_GetProperty(cx,objs[i],"canRead",&val))
        if (val == JSVAL_TRUE)
        {
         ready[i] = true;
         readyCount++;
//         printf("%d canRead\n",i);
        }
     }
 }

 if (readyCount) timeout = 0;

 int any = socketCount ? MultiSelect(sockets,ready,socketCount,length,timeout) : 0;
//printf("%d %d\n",readyCount,any);
 if (readyCount == 0 && any == 0)
 {//try again
      for (i=0; i<length; i++)
      {
          if (objs[i] && !sockets[i])
          {
            jsval val;
            //JSBool b=0;
            if (JS_GetProperty(cx,objs[i],"canRead",&val))
             //if (JS_ValueToBoolean(cx,val,&b)) if (b)
             if (val == JSVAL_TRUE)
             {
              ready[i] = true;
              readyCount++;
             }
          }
      }

 }
 readyCount+=any;
 if (readyCount) //return the list
 {
     jsval *array = new jsval[readyCount];

     int j=0;
     for (i=0; i<length && j<readyCount; i++)
         if (ready[i]) array[j++] = OBJECT_TO_JSVAL(objs[i]);

     ret = JS_NewArrayObject(cx, readyCount, array);
     delete[] array;
 }

 delete[] objs;
 delete[] sockets;
 RETOBJ(ret);
}
#ifdef XP_WIN
extern size_t CommandLineSize(jschar *s0, jschar* s1)
{
    return 2*(s0 ? wcslen((wchar_t*)s0) : 0) + (s1 ? wcslen((wchar_t*)s1) : 0) + 4;
}

extern void PrepareCommandLine(WStr& cmdline, jschar *s0, jschar* s1)
{
 wchar_t* c = cmdline;
 jschar q = s0[0];
 if (s0)
 {
     if (q != L'\"') *c++ = L'\"';
     while (*s0) *c++ = *s0++;
     if (q != L'\"') *c++ = L'\"';
     *c++ = L' ';
 }
 if (s1) wcscpy(c,(wchar_t*)s1);
}
#else
size_t CommandLineSize(char *s0, char* s1)
{
    return 2*(s0 ? strlen(s0) : 0) + (s1 ? strlen(s1) : 0) + 4;
}

void PrepareCommandLine(TStr& cmdline, char *s0, char* s1)
{
 char* c = (char*)cmdline;
 jschar q = s0[0];
 if (s0)
 {
     if (q != '\"') *c++ = '\"';
     while (*s0) *c++ = *s0++;
     if (q != '\"') *c++ = '\"';
     *c++ = ' ';
 }
 if (s1) strcpy(c,s1);
}
#endif


//program, command line, directory
NATIVE(ShellExecWait)
{
 JS_MaybeGC(cx);
 GETENV;
 if (argc == 0) ERR_COUNT(JSDB,ShellExec);
 if (!ISSTR(0)) ERR_TYPE(JSDB,ShellExec,1,String);
 if (!Env->AllowExec) RETBOOL(false);

#ifdef XP_WIN
 GETUCS2(0);
 GETUCS2(1);
 GETUCS2(2);

 WStr cmdline(CommandLineSize(s0,s1));
 PrepareCommandLine(cmdline,s0,s1);

//printf("execwait %s %s %s %s\n",(char*)TStr(s0),(char*)TStr(s1),(char*)TStr(s2),(char*)TStr(cmdline));
 PROCESS_INFORMATION pi;
 memset(&pi,0,sizeof(pi));
 STARTUPINFOW si;
 memset(&si,0,sizeof(si));
 si.cb=sizeof(STARTUPINFOW);
 si.dwFlags=STARTF_USESHOWWINDOW;
 si.wShowWindow=SW_SHOWNORMAL;
 if (CreateProcessW(NULL,(WCHAR*)cmdline,NULL,NULL,
                      FALSE,0,NULL,(WCHAR*)s2,&si,&pi))
   {  /* WARNING, if an error occurs, clean this up */
       CloseHandle(pi.hThread);

       WaitForSingleObject(pi.hProcess, INFINITE);
       DWORD ret=-1;
       GetExitCodeProcess(pi.hProcess,&ret);
       RETINT(ret);
   }
RETINT(-1);
#else
 GETUTF8(0);
 GETUTF8(1);
 GETUTF8(2);
 int a=0,ret=0;

 char dir[2048];
 if (*u2)
 {
     getcwd(dir,sizeof(dir));
     chdir(u2);
 }

 TStr cmdline(CommandLineSize(u0,u1));
 PrepareCommandLine(cmdline,u0,u1);
 ret = system(cmdline);
 if (*u2) chdir(dir);
 RETINT(ret);
#endif
}

NATIVE(ShellExec)
{
JS_MaybeGC(cx);
 GETENV;
 if (argc == 0) ERR_COUNT(JSDB,ShellExec);
 if (!ISSTR(0)) ERR_TYPE(JSDB,ShellExec,1,String);
 if (!Env->AllowExec) RETBOOL(false);

#ifdef XP_WIN
 GETUCS2(0);
 GETUCS2(1);
 GETUCS2(2);

 //WStr cmdline(CommandLineSize(s0,s1));
 //PrepareCommandLine(cmdline,s0,s1);
//printf("%s %s\n",(char*)TStr(s0),(char*)TStr(cmdline));
 HINSTANCE proc =
    ShellExecuteW(NULL,NULL,(WCHAR*)s0,(WCHAR*)s1,(WCHAR*)s2,SW_SHOWNORMAL);

 RETBOOL((int)proc > 32);
#else
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
 int ret = system(cmdline);
 if (*u2) chdir(dir);
 RETBOOL(ret != -1);
#endif
}

NATIVE(ExitCode)
{
     GETENV;

     if (argc > 0)
        TOINT(0,Env->ExitCode);
     RETINT(Env->ExitCode);
}

static JSBool shell_JSSet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 int x = JSVAL_TO_INT(id);
 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 1: return SetCWD(cx, obj, 1, rval, rval);
   case 3: return ExitCode(cx,obj,1,rval,rval);
  }
 return JS_FALSE;
}

static JSBool shell_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
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
   case 3: return ExitCode(cx,obj,0,0,rval);
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
    {"cwd",1, JSPROP_ENUMERATE|JSPROP_PERMANENT,shell_JSGet,shell_JSSet},
    {"pid",2, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,shell_JSGet},
    {"exitCode",3, JSPROP_ENUMERATE|JSPROP_PERMANENT,shell_JSGet,shell_JSSet},
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

NATIVE(Options);

NATIVE(BuildDate);

NATIVE(Version);

static JSFunctionSpec system_functions[] = {
    {"attributes",     FileStatus, 0},
    {"browse",     ShellBrowse,    1},
//    {"buildDate",       BuildDate,    0},
    {"copy",        jsCopyFile,       2},
    {"debug",         StartDebugger,           0},
    {"remove",       jsDeleteFile,           1},
    {"directories",       listDirs,           1},
    {"execute",        ShellExec,3},
    {"executeWait",        ShellExecWait,3},
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
    {"wait",           systemWait,      2},
    {"sleep",           GoToSleep,      1},
//    {"version",         Version,      0},
    {0}
};

JSObject* JSDBEnvironment::DefineSystemObject(JSFunctionSpec* extra, JSPropertySpec *props)
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
 if (props) JS_DefineProperties(cx,o,props);

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
