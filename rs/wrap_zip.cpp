#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#include "zlib/zlib.h"
#pragma hdrstop

JSBool
Archive_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 GETOBJ(Archive,ZipArchive,t);

 int x = JSVAL_TO_INT(id);

 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: RETSTRW(L"Archive");
   case 1: RETINT(t->Count());
   default: return JS_FALSE;
  }
 else return JS_FALSE;
}

JSBool
Archive_Archive(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
    jsval *rval)
{
 GETENV;
 if (!Env) return JS_FALSE;

 if (argc == 0) ERR_COUNT(Zip,Zip);

 if (ISSTR(0))
 {
  try {
  FileStream* in = new FileStream(STR(0),Stream::OMBinary,Stream::ReadOnly);
  ZipArchive* archive = new ZipArchive(in,true);
  SETPRIVATE(obj,ZipArchive,archive,true,NULL);
  return JS_TRUE;
  } catch(...)
  {  if (Env->errorOnFailure)
       return JS_FALSE;
     ERR_MSG(Archive,"File not found",STR(0));
  }
 }
 else if (ISOBJ(0))
 {
  JSObject* jx = JSVAL_TO_OBJECT(argv[0]);
  if (!JS_InstanceOf(cx,jx,Stream_Class(),0))
   return JS_FALSE;

  JSPointer<Stream> * jsps = (JSPointer<Stream> *)JS_GetPrivate(cx,jx);
  if (!jsps)
   return JS_FALSE;

  jsps->AddChild(GETPOINTER);
  ZipArchive* archive = new ZipArchive(jsps->P,false);
  CONSTRUCTOR(ZipArchive,archive,true,NULL);
  return JS_TRUE;
 }
 return JS_FALSE;
}

void Archive_JSFinalize(JSContext *cx, JSObject *obj)
{
 DELPRIVATE(ZipArchive);
}

I_WRAP_S(Archive,ZipArchive,Find);

static JSBool
Archive_Name(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 1) ERR_COUNT(Archive,Name);
 if (!ISINT(0)) ERR_TYPE(Archive,Name,1,Number);

 GETOBJ(Archive,ZipArchive,t);

 const char*c = (*t)[INT(0)];
 RETSTR(c?c:"");
}

static JSBool
Archive_Has(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 1) ERR_COUNT(Archive,Size);

 GETOBJ(Archive,ZipArchive,t);

 if (ISINT(0)) RETBOOL(t->Items[INT(0)] != 0);
 if (ISSTR(0)) RETBOOL(t->Find(STR(0)) != -1);
 ERR_TYPE(Archive,Has,1,Number|String);
}

static JSBool
Archive_Size(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 1) ERR_COUNT(Archive,Size);

 int i;

 GETOBJ(Archive,ZipArchive,t);

 if (ISINT(0))
  i = INT(0);
 else if (ISSTR(0))
  i = t->Find(STR(0));
 else ERR_TYPE(Archive,Size,1,Number|String);

 ZipFile* f = t->Items[i];
 RETINT(f ? f->uncompressedSize : 0);
}

WRAP(Archive,Close)
{
 CLOSEPRIVATE(Archive,ZipArchive);
 RETBOOL(true);
}

WRAP(Archive,Compress)
{
    if (argc == 0) ERR_COUNT(Archive,Compress);
    if (!ISSTR(0)) ERR_TYPE(Archive,Compress,1,String);

    GETUCS2(0);
    size_t data_len = JS_GetStringLength(j0);
    TChars source(data_len);
    jschar *chars=JS_GetStringChars(j0);
    char *c = source.buf;
    for (size_t i=0; i<data_len; i++)
      *c++ = *chars++;

    unsigned long dest_len = data_len+(data_len/100)+16;
    TChars dest(dest_len);

    z_stream stream;
    int status;

    stream.next_in = (Bytef*)source.buf;
    stream.avail_in = (uInt)source.size;
    stream.next_out = (Bytef*)dest.buf;
    stream.avail_out = (uInt)dest.size;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    dest_len = 0;
    status = deflateInit(&stream, Z_BEST_COMPRESSION);
    if (status == Z_OK)
    {
     status = deflate(&stream, Z_FINISH);
     if (status == Z_STREAM_END)
         dest_len = stream.total_out;
     status = deflateEnd(&stream);
    }

    if (dest_len)
    {
        RETSTRN((char *)dest.buf ,dest_len);
    }
    else
    {
        JS_ReportError(cx,"Compression error");
        return JS_FALSE;
    }
}

WRAP(Archive,Uncompress)
{
    if (argc == 0) ERR_COUNT(Archive,Uncompress);
    if (!ISSTR(0)) ERR_TYPE(Archive,Uncompress,1,String);

    //make a local copy of the low bytes
    GETUCS2(0);
    size_t data_len = JS_GetStringLength(j0);
    TChars cin(data_len);
    jschar *chars=JS_GetStringChars(j0);
    char *c = cin.buf;
    for (size_t i=0; i<data_len; i++)
      *c++ = *chars++;

    MemoryStream out;
    TChars cout(4096);
     //decompress on the heap, not the stack, to reduce overflow risks

    z_stream zlib;
    zlib.total_out = 0;
    zlib.zalloc    = 0;
    zlib.zfree     = 0;
    zlib.opaque   = 0;

    inflateInit(&zlib);
    zlib.avail_in  = 0;

    zlib.next_in = (Bytef*)cin.buf;
    zlib.avail_in = cin.size;
    int crc = 0;

    count_t bytesin = 0;
    int status = Z_OK;

    while (status == Z_OK && zlib.avail_in)
     {
      zlib.next_out = (Bytef*)cout.buf;
      zlib.avail_out = cout.size;
      status = inflate(&zlib,Z_SYNC_FLUSH);
      out.write(cout.buf,cout.size - zlib.avail_out);
     }

    inflateEnd(&zlib);
    RETSTRN(out,out.size());
}

static JSBool
Archive_Extract(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(Archive,Extract);
 if (!ISINT(0) && !ISSTR(0)) ERR_TYPE(Archive,Extract,1,Number|String);

 GETOBJ(Archive,ZipArchive,t);

 int i;

 if (ISINT(0))
  i = INT(0);
 else
  i = t->Find(STR(0));

 ZipFile* f = t->Items[i];

 if (!f)
  {
   if (argc > 1) RETBOOL(false);
   RETOBJ(NULL);
  }

 if (argc > 1)
 {
  Stream* out = 0;
  GETFILE(1,out);
  if (out)
  {
   t->Extract(i,*out);
   RETBOOL(true);
  }
  RETBOOL(false);
 }

 MemoryStream* out = new MemoryStream;
 t->Extract(i,*out);
 out->rewind();
 RETOBJ(Stream_Object(cx,out,true,0));
}

WRAP_HELP(Archive,
 "name(index)\nextract(index)\nextract(index,string)\nsize(index)\n"
 "close()\nArchive.compress(string)\nArchive.uncompress(string)\n")

static JSPropertySpec Archive_properties[] = {
    {"className",0, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT, Archive_JSGet},
    {"count",1, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT, Archive_JSGet},
    {0}
};

static JSFunctionSpec Archive_functions[] = {
    {"name",     Archive_Name,      1},
    {"extract",    Archive_Extract, 1},
    {"size", Archive_Size,0},
    {"close", Archive_Close,0},
    {"find",ZipArchive_Find,1},
    {"has",Archive_Has,1},
    {0}
};

static JSFunctionSpec Archive_fnstatic[] = {
    {"help",  Archive_HELP,    0},
    {"compress",  Archive_Compress,    0},
    {"uncompress",  Archive_Uncompress,    0},
    {0}
};

static JSClass Archive_class = {
    "Archive", JSCLASS_HAS_PRIVATE,         //Archive_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,Archive_JSFinalize
};

JSObject*
Archive_Object(JSContext *cx, ZipArchive* t,bool autodelete,JSPointerBase* Parent)
{
 GETENV;
 JSObject* obj;
 ENTERNATIVE(cx);
 MAKENEW(Archive);
 SETPRIVATE(obj,ZipArchive,t,autodelete,Parent);
 return obj;
}

void Archive_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(Archive);
}

JSClass* Archive_Class() {return &Archive_class;}
