#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

void Server_JSFinalize(JSContext *cx, JSObject *obj)
{
 JSPointer<InternetServer> * t =
   (JSPointer<InternetServer>*)JS_GetPrivate(cx,obj);
 if (t) delete t;

 JS_SetPrivate(cx,obj,NULL);
}

WRAP(Server,Server)
{
 GETENV;
 if (!Env) return JS_FALSE;
 if (Env->SafeMode)
 {
  if (Env->errorOnFailure) ERR_MSG(Server,Server,"blocked by security settings");
 }

 int port = 0;

 if (argc > 1) ERR_COUNT(Server,Server);

 if (argc == 1) port = INT(0);

 InternetServer * dt = 0;

 try {
  dt = new InternetServer(port);
 } catch (...){}
 if (!dt)
 {
  if (Env->errorOnFailure) return JS_FALSE;
  ERR_MSG(Server,"Couldn't start a server",itos(port));
 }

 if (dt->error)
 {
  TStr err(dt->error->info());
  delete dt;
  dt = 0;
  ERR_MSG(Server,"bind",err);
 }
 if (dt)
   CONSTRUCTOR(InternetServer,dt,true,NULL);

 return JS_TRUE;
}

WRAP(Server,Close)
{
 CLOSEPRIVATE(Server,InternetServer);
 RETBOOL(true);
}
// we use an asynchronous procedure call system to alllow us to sleep between
// connections.

WRAP(Server,ToString)
{
 GETOBJ(Server,InternetServer,t);
 RETSTR(t->hostinfo);
}

WRAP(Server,Wait)
{
 JS_MaybeGC(cx);
 GETOBJ(Server,InternetServer,t);
 int32 ms=-1;
 if (argc)
 JS_ValueToInt32(cx,ARGV(0),&ms);
 RETBOOL(t->AnyoneWaiting(ms));
}

WRAP(Server,Accept)
{
 JS_MaybeGC(cx);
 GETOBJ(Server,InternetServer,t);
 if (argc == 1)
 {
     int32 ms=-1;
     JS_ValueToInt32(cx,ARGV(0),&ms);
     if (!t->AnyoneWaiting(ms)) RETOBJ(0);
 }
 else if (argc != 0) ERR_COUNT(Server,Accept);

 InternetStream * s = t->Accept();
 if (!s) RETOBJ(0); // returns JSVAL_NULL
/*
 struct linger l;
 l.l_onoff = 1;
 l.l_linger = 5; //5 second timeout

 setsockopt(s->s,SOL_SOCKET,SO_LINGER,(const char*)&l,sizeof(l));
*/
 JSObject* o = Stream_Object(cx,s,true,GETPOINTER);
 //all child streams will be closed when the server is deleted!

 RETOBJ(o);
}

JSBool
Server_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 GETOBJ(Server,InternetServer,t);

 int x = JSVAL_TO_INT(id);

 if (JSVAL_IS_INT(id))
  switch (x)
  {//host names are supposed to be UTF-8
   case 0: RETBOOL(t->AnyoneWaiting());
   case 1: RETSTR(t->hostname);
   case 2: RETINT(t->port);
   case 3: RETSTRW(L"Server");
   case 4: RETSTR(t->hostinfo);
   case 5: RETSTR(t->address);
  }

 return JS_FALSE;
}

WRAP_HELP(Server,
 "Server(port=80) listens on a port\n"
 "accept() returns a read/write stream connection\n"
 "close() disconnects the server and all open connections\n"
 "toString() returns a description\n"
 "anyoneWaiting is true if a connection is requested\n"
 "hostName and port are read-only variables\n")

static JSPropertySpec Server_properties[] = {
    {"anyoneWaiting",      0,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Server_JSGet},
    {"hostName",      1,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Server_JSGet},
    {"port",      2,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Server_JSGet},
    {"className",3, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Server_JSGet},
    {"name",4, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Server_JSGet},
    {"address",5, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Server_JSGet},
    {0}
};

static JSFunctionSpec Server_functions[] = {
    {"accept",     Server_Accept,      0},
    {"close",      Server_Close,   0},
    {"toString",   Server_ToString, 0},
    {"wait",     Server_Wait,      0},
    {0}
};

static JSFunctionSpec Server_fnstatic[] = {
    {"help",  Server_HELP,    0},
    {0}
};

static JSClass Server_class = {
    "Server", JSCLASS_HAS_PRIVATE,         //Server_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,Server_JSFinalize
};



JSObject*
Server_Object(JSContext *cx, InternetServer* t,bool autodelete,JSPointerBase* Parent)
{
 JSObject* obj;
 GETENV;

 MAKENEW(Server);
 SETPRIVATE(obj,InternetServer,t,autodelete,Parent);

 return obj;
}

void Server_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(Server);
}

JSClass* Server_Class() {return &Server_class;}

