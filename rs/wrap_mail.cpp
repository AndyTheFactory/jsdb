#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

/* just sends mail for now */

void Mail_JSFinalize(JSContext *cx, JSObject *obj)
{
 //if (JS_GetParent(cx,obj)) return;
 DELPRIVATE(MailLibrary);
}

JSBool
Mail_Mail(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
    jsval *rval)
{
 GETENV;
 if (!Env) return JS_FALSE;
 if (argc == 0) ERR_COUNT(Mail,Mail);

 char * params[6];
#ifdef XP_WIN
 TParameterList Type("CCMAIL5=5|BEYONDMAIL=4|VIM=7|NOTES=3|MAPI=5|MSMAIL=5|EXCHANGE=6|"
                     "CCMAIL8=8|CCMAIL=8|GROUPWISE=11|CMC=9|INTERNET=10|POP3=10|"
                     "SMTP=10",'|');
#else
 TParameterList Type("INTERNET=10|POP3=10|SMTP=10",'|');
#endif

 int t = Type.GetInt(STR(0));
 if (!t) ERR_TYPE(Mail,Mail,1,mail type);

 for (int i=1; i< 7; i++)
 {
  if (i < argc) params[i-1] = STR(i);
  else params[i-1] = 0;
 }

 const char* addr = params[5]; //sender address
 while (*addr)
 {
  if (strchr("\r\n",*addr))
   ERR_MSG(Mail,"Sender address contains invalid characters",params[5]);

  addr++;
 }

 MailLibrary * dt = 0;
 try {
 dt = OpenMailSystem((EMailType)t,true,
                     params[0],params[1],params[2],params[3],params[4],params[5]);
 } catch (...) {dt = 0;}

 if (!dt)
 {
  if (Env->errorOnFailure)
   return JS_FALSE;

  ERR_MSG(Mail,"Could not open the mailbox",TStr(params[0],params[1],params[2],params[3],params[4],params[5]));
 }

 if (dt)
 SETPRIVATE(obj,MailLibrary,dt,true,NULL);

 return JS_TRUE;
}

static JSBool
Mail_ToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Mail,MailLibrary,t);
 RETSTR(t->filename());
}

static JSBool
Mail_Send(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Mail,MailLibrary,t);
 if (argc < 3) ERR_COUNT(Mail,Send);

 GETUTF8(0);
 GETUTF8(1);
 GETUTF8(2);
 GETUTF8(3);
 char * recip = u0;
 char * subject = u1;
 char * text = u2;
 char * html = u3;

 bool ret = false;
 xdb err;
 try {
 if (*u3)
  ret = t->SendMultipart(recip,subject,0,text,html);
 else
  ret = t->SendDocuments(recip,0,subject,text);
 }
 catch(xdb& x)
 {
  ERR_MSG(Mail,Send,TStr(x.why(),"\t",x.info()));
 }
 if (!ret && t->error)
 {
  ERR_MSG(Mail,Send,TStr(t->error->why(),"\t",t->error->info()));
 }

 RETBOOL(ret);
}


static JSBool
Mail_SendFiles(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Mail,MailLibrary,t);
 if (argc < 3) ERR_COUNT(Mail,Send);
 for (int i=0; i<argc; i++)
  {
   if (!ISSTR(i)) ERR_TYPE(Mail,send,i+1,string);
  }

 GETUTF8(0);
 GETUTF8(1);
 GETUTF8(2);
 GETUTF8(3);
 GETUTF8(4);
 char * recip = u0;
 char * subject = u1;
 char * files = u2;
 char * text = u3;
 char * html = u4;

 bool ret = false;
 try {
  ret = t->SendMultipart(recip,subject,files,text,html);
 }
 catch(xdb& x)
 {
  ERR_MSG(Mail,Send,TStr(x.why(),"\t",x.info()));
 }
 if (!ret && t->error)
 {
  ERR_MSG(Mail,Send,TStr(t->error->why(),"\t",t->error->info()));
 }

 RETBOOL(ret);
}

JSBool
Mail_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 GETOBJ(Mail,MailLibrary,t);

 int x = JSVAL_TO_INT(id);

 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: RETSTR(t->filename());
   case 1: RETSTRW(L"Mail");
   default: return JS_FALSE;
  }
 else return JS_FALSE;
}

static JSBool
Mail_Get(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Mail,MailLibrary,t);
 RETOBJ(Table_Object(cx,new TMailList(t),true,GETPOINTER));
}

WRAP_HELP(Mail,
 "Mail(type,login,password,server,smtp_server,language) where type is one of\n"
 "  CMC,VIM,NOTES,MAPI,CMC,INTERNET and language is a MIME character encoding\n"
 "send(recipient, subject, textNote, htmlNote)\n"
 "sendFiles(recipient, subject, fileList, textNote, htmlNote)\n"
 "  fileList is comma-delimited\n"
 "get(unread_only) (not implemented) returns a list of MailMessage objects\n"
 "help()\n")

static JSPropertySpec Mail_properties[] = {
    {"name",      0,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Mail_JSGet},

    {"className",1, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Mail_JSGet},
    {0}
};

static JSFunctionSpec Mail_functions[] = {
    {"send",     Mail_Send,      4},
    {"sendFiles",Mail_SendFiles,      5},
    {"get",      Mail_Get,      3},
    {"toString", Mail_ToString, 0},
    {0}
};

static JSFunctionSpec Mail_fnstatic[] = {
    {"help",  Mail_HELP,    0},
    {0}
};

static JSClass Mail_class = {
    "Mail", JSCLASS_HAS_PRIVATE,         //Mail_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,Mail_JSFinalize
};



JSObject*
Mail_Object(JSContext *cx, MailLibrary* t,bool autodelete,JSPointerBase* Parent)
{
 JSObject* obj;
 GETENV;

 MAKENEW(Mail);
 SETPRIVATE(obj,MailLibrary,t,autodelete,Parent);

 return obj;
}

void Mail_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(Mail);
}

JSClass* Mail_Class() {return &Mail_class;}

