#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop


JSBool
BuildDate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    GETENV;
    WRITELN(__DATE__ " " __TIME__);
    return JS_TRUE;
}

JSBool
Version(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (argc > 0 && JSVAL_IS_INT(argv[0]))
    *rval = INT_TO_JSVAL(JS_SetVersion(cx, (JSVersion) JSVAL_TO_INT(argv[0])));
    else
    *rval = INT_TO_JSVAL(JS_GetVersion(cx));
    return JS_TRUE;
}

static struct {
    const wchar_t  *name;
    uint32      flag;
} js_options[] = {
    {L"strict",          JSOPTION_STRICT},
    {L"werror",          JSOPTION_WERROR},
#ifdef JSOPTION_ATLINE
    {L"atline",          JSOPTION_ATLINE},
#endif
#ifdef JSOPTION_XML
    {L"xml",          JSOPTION_XML},
#endif
#ifdef JSOPTION_JIT
    {L"jit",          JSOPTION_JIT},
#endif
     {0,                 0}
};

JSBool
Options(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    uint32 optset, flag;
    uintN i, j;
    JSString *str;
    const jschar *opt;
    MemoryStream ret;

   if (argc)
   {
    optset = 0;
    for (i = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return JS_FALSE;
        opt = JS_GetStringChars(str);
        for (j = 0; js_options[j].name; j++) {
            if (ucscmp(js_options[j].name, opt) == 0) {
                optset |= js_options[j].flag;
                break;
            }
        }
    }
    JS_ToggleOptions(cx, optset);
   }

   optset = JS_GetOptions(cx);
    bool any = false;

    for (j = 0; js_options[j].name; j++)
     {
        if (js_options[j].flag & optset)
        {
         if (any) ret << ",";
         any = true;
         ret << (char*)TStr((jschar*)js_options[j].name);
        }
     }

    str = JS_NewStringCopyZ(cx, ret);
    if (!str)
     return JS_FALSE;

    *rval = STRING_TO_JSVAL(str);
    return JS_TRUE;
}

#ifdef JSDB_MINIMAL

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);
/* Mark Adler's CRC function from ZLIB */
/* ========================================================================= */
unsigned long adler32(unsigned long adler, const unsigned char *buf,unsigned int  len)
{
    unsigned long s1 = adler & 0xffff;
    unsigned long s2 = (adler >> 16) & 0xffff;
    int k;

    if (buf == 0) return 1L;

    while (len > 0) {
        k = len < NMAX ? len : NMAX;
        len -= k;
        while (k >= 16) {
            DO16(buf);
        buf += 16;
            k -= 16;
        }
        if (k != 0) do {
            s1 += *buf++;
        s2 += s1;
        } while (--k);
        s1 %= BASE;
        s2 %= BASE;
    }
    return (s2 << 16) | s1;
}
#else
extern "C" {
unsigned long adler32(unsigned long adler, const unsigned char *buf,unsigned int  len);
}
// zlib/adler32.c
#endif
static JSBool
CRC32(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) ERR_COUNT(js,CRC32);
    long int start = 0xffffffff;

    if (ISINT(0))
    {
     if (argc < 2) ERR_COUNT(js,CRC32);
     if (!ISSTR(1)) ERR_TYPE(js,CRC32,2,String);
     start = INT(0);

     GETSTRN(1);
     RETINT(adler32(start,(uint8*)s1,l1));
    }
    else
    {
     if (!ISSTR(0)) ERR_TYPE(js,CRC32,1,String);
     if (argc > 1 && ISINT(1)) start = INT(1);

     GETSTRN(0);
     RETINT(adler32(start,(uint8*)s0,l0));
    }
//#ifdef JSDB_MINIMAL
//    RETINT(adler32(start,(uint8*)c,strlen(c)));
//#else
//  RETINT(CRC32(start,(uint8*)c,strlen(c)));
//#endif
}

static JSBool
SplitURL(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (!argc) ERR_COUNT(js,SplitURL);
 if (!ISSTR(0)) ERR_TYPE(js,SplitURL,1,String);
 ENTERNATIVE(cx);

 TStr data[6];
 jsval arr[6];

 char * s = STR(0);
 URLSplit(s,data[0],data[1],data[2],data[3],data[4],data[5]);
 //service, user, password, server, file, query
 for (int i =0 ; i < 6; i++)
  {
   arr[i] = STRING_TO_JSVAL(ROOT(JS_NewStringCopyZ(cx,data[i])));
  }

 JSObject * ret = JS_NewArrayObject(cx,6,arr);

 RETOBJ(ret);
}

static JSBool
EncodeURL(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) ERR_COUNT(js,EncodeURL);
    if (!ISSTR(0)) ERR_TYPE(js,EncodeURL,1,String);
    GETUTF8(0);
//    WStr out;
    //char * s = STR(0);
//    URLEncode1(out,u0); //all 7-bit characters
    TStr out;
    URLEncodeURL(out,u0);
    RETSTR(out);
}

static JSBool
EncodeHTML(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) ERR_COUNT(js,EncodeURL);
    if (!ISSTR(0)) ERR_TYPE(js,EncodeURL,1,String);

    MemoryStream out;
    HTMLEscape(out,WSTR(0));
    RETSTR(out); //return a plain ASCII string
}

static JSBool
DecodeURL(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) ERR_COUNT(js,DecodeURL);
    if (!ISSTR(0)) ERR_TYPE(js,DecodeURL,1,String);

    TStr out(STR(0));
    URLDecode(out);
    RETSTRWC(out);
}

static JSBool
DecodeHTML(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) ERR_COUNT(js,DecodeHTML);
    if (!ISSTR(0)) ERR_TYPE(js,DecodeHTML,1,String);

    GETUTF8(0); //proper HTML is utf-8 or 7-bit
    HTMLUnEscape(u0);
    RETSTRWC(u0);
}

static JSBool
EncodeB64(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) RETSTRW(L"");

    Stream* in;
    Stream* out;
    GETFILE(0,in);
    GETFILE(1,out);

    MemoryStream out1;
    int i;

    if (!out) out = &out1;
    if (!in && argc && ISSTR(0))
    {
        GETSTRN(0);
        ByteStream in1(s0, l0);
        i = b64encode(in1,*out);
    }
    else
        i = b64encode(*in,*out);
    if (out1.size()) RETSTR(out1);
    RETINT(i);
}

static JSBool
DecodeB64(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) RETSTRW(L"");

    Stream* in;
    Stream* out;
    GETFILE(0,in);
    GETFILE(1,out);

    MemoryStream out1;
    int i;

    if (!out) out = &out1;
    if (!in && argc && ISSTR(0))
    {
        GETSTRN(0);
        ByteStream in1(s0, l0);
        i = b64decode(in1,*out);
    }
    else
        i = b64decode(*in,*out);
    if (out1.size()) RETSTR(out1);
    RETINT(i);
}

static JSBool
EncodeUTF8(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) ERR_COUNT(js,EncodeUTF8);

    GETUCS2(0);
    //JSString* j = JS_ValueToString(cx,argv[0]);
    if (!j0) ERR_TYPE(js,EncodeUTF8,1,String);;

    TStr t(s0);
    RETSTR(t);
}

static JSBool
DecodeUTF8(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) ERR_COUNT(js,DecodeUTF8);
    JSString* j0;
    const char* s0;
    GETSTRING(0);
    WStr t(s0); //JS_GetStringBytes(j));
    RETSTRW(t);
}

#ifndef XP_WIN //http://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1252.TXT
static int cp1252[256] =
{
 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
 0x20AC, 0x0020, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0020, 0x017D, 0x0020,
 0x0020, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0020, 0x017E, 0x0178, 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};
#endif

static JSBool
DecodeANSI(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (!argc) ERR_COUNT(js,DecodeANSI);
    if (!ISSTR(0)) ERR_TYPE(js,DecodeANSI,1,String);

#ifdef XP_WIN
 //Allocate enough space.
 char* x =STR(0);
 WStr data(MultiByteToWideChar(CP_ACP,0,x,strlen(x),NULL,0));
 //WStr data;
 //data.Resize(MultiByteToWideChar(CP_ACP,0,x,strlen(x),NULL,0));
 MultiByteToWideChar(CP_ACP,0,x,strlen(x),(wchar_t*)data,data.bytes()/sizeof(uint16));
 RETSTRW(data);
#else //http://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1252.TXT
 char* x =STR(0);
 size_t l = strlen(x);
 WStr data(l);
 uint16* d = (uint16*)data;
 for (size_t i=0; i<l; i++)
 {
  d[i] = cp1252[(unsigned)x[i]];
 }
 RETSTRW(d);
#endif
}

#ifdef RSLIB_FORMAT
static JSBool
PrintReport(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
if (argc != 2) ERR_COUNT(Shell,PrintReport);
 const char * s0="";
 JSString* j0=0;
 Stream* ins=0;
 Stream* out=0;
 TNameValueList *record=0;
 GETFILE(0,ins);
 TPointer<TParameterList> recordAutoDelete;
  GETREC(1,record);
 if (argc > 2) GETFILE(2,out);
 if (!ins) GETSTRING(0);
 if (!s0 && !ins) ERR_TYPE(Shell,PrintReport,1,StringStream);
 if (!record) ERR_TYPE(Shell,PrintReport,2,Record);

 MemoryStream memstr;
 ByteStream bs(s0);

 Stream* outstr = out;
 Stream* instr = ins;

 if (!outstr) outstr = &memstr;

 if (!instr) instr = &bs;

 FormatText(*record,*instr,*outstr);

 if (!out) RETSTR((char*)memstr);
 *rval = argv[2];
 return JS_TRUE;
}
#endif
//important -- obj and rval are null when called by jsdb.cpp
extern JSBool
Write(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *);
extern JSBool
Writeln(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *);

JSBool
Write(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
{
  JSString *str;
  GETENV;
  if (!Env->out) return JS_TRUE;

#ifdef XP_WIN
  FileStream* fs = TYPESAFE_DOWNCAST(Env->out,FileStream);
  HANDLE h = fs ? fs->File : 0;
  DWORD mode = 0;
  if (h && h == GetStdHandle(STD_OUTPUT_HANDLE)
       && GetFileType(h) == FILE_TYPE_CHAR && GetConsoleMode(h,&mode))
  {
  for (uintN i = 0; i < argc; i++)
  {
    str = JS_ValueToString(cx, argv[i]);
    if (!str) return JS_TRUE;
    WriteConsoleW(h,JS_GetStringChars(str),JS_GetStringLength(str),&mode,NULL);
  }
  }
  else
  {
#endif
  for (uintN i = 0; i < argc; i++)
  {
    str = JS_ValueToString(cx, argv[i]);
    if (!str) return JS_TRUE;
    Env->out->writestr(TStr(JS_GetStringChars(str),JS_GetStringLength(str)));
  }
#ifdef XP_WIN
  }
#endif
  return JS_TRUE;
}

JSBool
Writeln(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 Write(cx,obj,argc,argv,rval);
 GETENV;
 if (Env->out)
   Env->out->writestr("\n");
 return JS_TRUE;
}
//extern JSBool
//Readln(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool
Readln(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETENV;
 if (Env->in)
 {
  MemoryStream line;
  const  char * end = "\n";
  if (argv && ISSTR(0)) end = STR(0);
     Env->in->ReadUntilWord(end,&line);
  char* c = line;
  if (*c && c[strlen(c)-1] == '\r')
   c[strlen(c)-1] = 0;
  RETSTRWC(line);
 }
 RETSTRW(L"");
}

static JSFunctionSpec shell_functions[] = {
   // {"stripWhitespace",  Whitespace,    1},
    {"println",        Writeln,         8},
    {"print",        Write,             8},
    {"writeln",         Writeln,        8},
    {"write",           Write,          8},
    {"readln",         Readln,        8},
    {"readLine",       Readln,          1},
    {"jsVersion",         Version,      0},
    {"options",         Options,      2},
    {"jsOptions",         Options,      2},
    {"jsBuildDate",       BuildDate,    0},
#ifdef RSLIB_FORMAT
{"printReport",        PrintReport,         4},
#endif
{"splitURL",        SplitURL,       1},
    {"encodeURL",       EncodeURL,      1},
    {"decodeURL",       DecodeURL,      1},
    {"encodeUTF8",       EncodeUTF8,      1},
    {"decodeUTF8",       DecodeUTF8,      1},
    {"decodeANSI",       DecodeANSI,      1},
    {"encodeB64",       EncodeB64,      1},
    {"decodeB64",       DecodeB64,      1},
    {"encodeHTML",       EncodeHTML,      1},
    {"decodeHTML",       DecodeHTML,      1},
    {"crc32",         CRC32,        1},

    {0}
};

static JSClass global_class = {
    "global", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,   JS_FinalizeStub
};

void JSDBEnvironment::InitGlobal(void*v)
{
 System = v;
 global = JS_NewObject(cx, &global_class, NULL, NULL);
// JS_AddRoot(cx,global);
 JS_DefineFunctions(cx, global, shell_functions);
 JS_SetErrorReporter(cx, rs_ErrorReporter);
 JS_SetPrivate(cx,global,(void*)this);
 JS_SetContextPrivate(cx, (void*)this);
 JS_SetOptions(cx,JSOPTION_VAROBJFIX);
}

void JSDBEnvironment::Cleanup()
{
}

