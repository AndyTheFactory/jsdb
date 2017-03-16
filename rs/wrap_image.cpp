#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop
#include <math.h>

void Image_JSFinalize(JSContext *cx, JSObject *obj)
{
 DELPRIVATE(GIFImage);
}

JSBool
Image_Image(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
    jsval *rval)
{
 GETENV;
 if (!Env) return JS_FALSE;

// exit(EXIT_FAILURE);
 if (argc != 2) ERR_COUNT(Image,new)

 int32 w;
 int32 h;

 TOINT(0,w);
 TOINT(1,h);

 if (w < 1 || w > 4096 || h < 1 || h > 4096)
 {
  if (Env->errorOnFailure)
   return JS_FALSE;

  char err[128];
  sprintf(err,"Image(%d, %d) must be between 1x1 and 4096x4096",w,h);

  ERR_MSG(Image,new,err);
 }

 try {
 GIFImage *t = new GIFImage(w,h);
 CONSTRUCTOR(GIFImage,t,true,NULL);
 } catch(...)
 {
   if (Env->errorOnFailure)
   return JS_FALSE;

  char err[128];
  sprintf(err,"Image(%d, %d) must be between 1x1 and 4096x4096",w,h);

  ERR_MSG(Image,new,err);
 }

 return JS_TRUE;
}


JSBool
Image_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 GETOBJ(Image,GIFImage,t);
 int x = JSVAL_TO_INT(id);

 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: RETINT(t->iw);
   case 1: RETINT(t->ih);
   case 2: RETINT(t->nextcolor);
   case 3: RETINT(t->Size());
   case 4: RETSTRW(L"Image");
   default: return JS_FALSE;
  }
 else return JS_FALSE;
}

static JSPropertySpec Image_properties[] = {
    {"width", 0,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Image_JSGet},
    {"height", 1,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Image_JSGet},
    {"colors", 2,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Image_JSGet},
    {"size", 3,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Image_JSGet},
    {"className",4, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Image_JSGet},
    {0}
};

static JSBool Image_BGColor(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(Image,setBGColor);
 int32 v1; if (!TOINT(0,v1)) ERR_TYPE(Image,setBGColor,1,integer);
 int32 v2; if (!TOINT(1,v2)) ERR_TYPE(Image,setBGColor,2,integer);
 int32 v3; if (!TOINT(2,v3)) ERR_TYPE(Image,setBGColor,3,integer);
 GETOBJ(Image,GIFImage,t);
 t->setBGColor(v1,v2,v3);
 RETINT(0);
}

static JSBool Image_Color(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(Image,Color);
 int32 v1; if (!TOINT(0,v1)) ERR_TYPE(Image,Color,1,integer);
 int32 v2; if (!TOINT(1,v2)) ERR_TYPE(Image,Color,2,integer);
 int32 v3; if (!TOINT(2,v3)) ERR_TYPE(Image,Color,3,integer);
 GETOBJ(Image,GIFImage,t);
 RETINT( t->Color(v1,v2,v3));
}


static JSBool Image_Line(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(Image,Line);
 int32 v1; if (!TOINT(0,v1)) ERR_TYPE(Image,Line,1,integer);
 int32 v2; if (!TOINT(1,v2)) ERR_TYPE(Image,Line,2,integer);
 int32 v3; if (!TOINT(2,v3)) ERR_TYPE(Image,Line,3,integer);
 int32 v4; if (!TOINT(3,v4)) ERR_TYPE(Image,Line,4,integer);
 int32 c; if (!TOINT(4,c)) ERR_TYPE(Image,Line,5,integer);
 GETOBJ(Image,GIFImage,t);
 t->Line(v1,v2,v3-v1,v4-v2,c);
 RETBOOL(true);
}

extern GIFFont Font_Helvetica9;

static JSBool Image_Print(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(Image,Print);
 JSString* j3;
 char * s3 = 0;

 GETSTRING(3);
 if (!s3 || !*s3) RETBOOL(true);

 int32 x; if (!TOINT(0,x)) ERR_TYPE(Image,Print,1,integer);
 int32 y; if (!TOINT(1,y)) ERR_TYPE(Image,Print,2,integer);
 int32 c; if (!TOINT(2,c)) ERR_TYPE(Image,Print,3,integer);

 int32 bg=0; if (argc > 4) TOINT(4,bg);
 GETOBJ(Image,GIFImage,t);
 t->Print(Font_Helvetica9,x,y,c,s3,bg);
 RETBOOL(true);
}

static JSBool Image_Arc(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(Image,Arc);
 int32 x; if (!TOINT(0,x)) ERR_TYPE(Image,Arc,1,integer);
 int32 y; if (!TOINT(1,y)) ERR_TYPE(Image,Arc,2,integer);
 double r; if (!TODBL(2,r)) ERR_TYPE(Image,Arc,3,number);
 double start; if (!TODBL(3,start)) ERR_TYPE(Image,Arc,4,number);
 double finish; if (!TODBL(4,finish)) ERR_TYPE(Image,Arc,5,number);
 int32 c; if (!TOINT(5,c)) ERR_TYPE(Image,Arc,6,integer);
 GETOBJ(Image,GIFImage,t);
 t->Arc(r,x,y,fabs(finish-start),finish < start ? finish : start,c);
 RETBOOL(true);
}

static JSBool Image_Fill(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(Image,Fill);
 int32 v1; if (!TOINT(0,v1)) ERR_TYPE(Image,Fill,1,integer);
 int32 v2; if (!TOINT(1,v2)) ERR_TYPE(Image,Fill,2,integer);
 int32 v3; if (!TOINT(2,v3)) ERR_TYPE(Image,Fill,3,integer);
 GETOBJ(Image,GIFImage,t);
 t->Fill(v1,v2,v3);
 RETBOOL(true);
}

static JSBool Image_PieSlice(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(Image,PieSlice);
 int32 x; if (!TOINT(0,x)) ERR_TYPE(Image,PieSlice,1,integer);
 int32 y; if (!TOINT(1,y)) ERR_TYPE(Image,PieSlice,2,integer);
 double r; if (!TODBL(2,r)) ERR_TYPE(Image,PieSlice,3,number);
 double start; if (!TODBL(3,start)) ERR_TYPE(Image,PieSlice,4,number);
 double finish; if (!TODBL(4,finish)) ERR_TYPE(Image,PieSlice,5,number);
 int32 c; if (!TOINT(5,c)) ERR_TYPE(Image,PieSlice,6,integer);
 GETOBJ(Image,GIFImage,t);
 t->PieSlice(x,y,start,finish,r,c);
 RETBOOL(true);
}

static JSBool Image_Setp(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 3) ERR_COUNT(Image,Set);
 GETOBJ(Image,GIFImage,t);
 int32 a,b,c;
 TOINT(0,a);
 TOINT(1,b);
 TOINT(2,c);
 t->Pixel(a,b)=(uint8)c;
 RETINT(c);
}

static JSBool Image_Getp(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 2) ERR_COUNT(Image,Get);
 GETOBJ(Image,GIFImage,t);
 int32 a,b;
 TOINT(0,a);
 TOINT(1,b);
 RETINT(t->Pixel(a,b));
}


static JSBool Image_Interlace(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Image,GIFImage,t);
 t->Interlace();
 RETBOOL(true);
}
static JSBool Image_Write(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc == 0) ERR_COUNT(Image,Write);
 GETOBJ(Image,GIFImage,t);
 Stream* out;
 GETFILE(0,out)
 if (!out) ERR_TYPE(Image,Write,1,stream);
 if (t->iw > 16 && t->ih > 16)
   t->WriteRLE(*out);
 else
   t->Write(*out); //uncompressed small images.
 RETBOOL(true);
}

WRAP(Image,Close)
{
 CLOSEPRIVATE(Image,GIFImage);
 RETBOOL(true);
}

static JSFunctionSpec Image_functions[] = {
    {"setBGColor",     Image_BGColor, 3},
    {"color",     Image_Color, 3},
    {"line", Image_Line,3},
    {"arc",    Image_Arc, 6},
    {"fill",    Image_Fill, 3},
    {"slice",    Image_PieSlice, 6},
    {"write",    Image_Write,1},
    {"getp",    Image_Getp,2},
    {"setp",    Image_Setp,3},
    {"print",  Image_Print,4},
    {"interlace", Image_Interlace,0},
    {"close", Image_Close,0},
    {0}
};

WRAP(Image,HELP)
{
 RETSTRW(L"line(x1,y1,x2,y2,color)\n"
 L"arc(x,y,r,start,finish,color)\n"
 L"fill(x,y,color)\n"
 L"slice(x,y,r,start,finish,color)\n"
 L"setBGColor(r,g,b)\n"
 L"color(r,g,b) returns an index or 0 on error\n"
 L"setp(x,y,color)\n"
 L"getp(x,y)\n"
 L"print(x,y,color,text,background)\n"
 L"interlace()\n"
 L"");
}

static JSFunctionSpec Image_fnstatic[] = {
    {"help",  Image_HELP,    0},
    {0}
};

static JSClass Image_class = {
    "Image", JSCLASS_HAS_PRIVATE,         //Image_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,Image_JSFinalize
};

JSObject*
Image_Object(JSContext *cx, GIFImage* t,bool autodelete,JSPointerBase* Parent)
{
 GETENV;
 JSObject* obj;
 MAKENEW(Image);;

 SETPRIVATE(obj,GIFImage,t,autodelete,Parent);
 return obj;
}

void Image_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(Image);
}

JSClass* Image_Class() {return &Image_class;}
