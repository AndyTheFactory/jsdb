#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

#ifndef TBL_NO_SQL

#include "rs/tbl_sql.h"
#include "rs/sql.h"
/* class ODBC

var db = new ODBC('connection string');
var table = db.query('SELECT * FROM table');
var db.exec('INSERT INTO table BLAH BLAH BLAH');

*/

WRAP_HELP(ODBC,L"query('SELECT * FROM ...')\nexec('INSERT INTO ...')\n");

JSBool
ODBC_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 const char * c = 0;
 count_t i = 0;

 GETOBJ(ODBC,DBPointer,t);

 int x = JSVAL_TO_INT(id);

 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: c = (*t)->DriverName; break;
   case 3: RETSTRW(L"ODBC");
   default: return JS_FALSE;
  }
 else return JS_FALSE;

 if (c) RETSTR(c);
 RETINT(i);
}


JSBool
ODBC_ODBC(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
    jsval *rval)
{
 GETENV;
 if (!Env) return JS_FALSE;

 if (!Env->TableEnv) return JS_FALSE;

 ODBCEnv* tblenv = (ODBCEnv*)Env->TableEnv->odbcenv;
 if (!tblenv) return JS_FALSE;

 JSString* j0;
 char * s0;
 GETSTRING(0);
 if (!s0) ERR_TYPE(ODBC,ODBC,1,String);
 if (!*s0) ERR_TYPE(ODBC,ODBC,1,String);

 DBPointer* dbp;
 TStr Error;
 try {
  dbp = new DBPointer(tblenv->OpenDatabase(s0,Error));
 }
 catch (...)
 {
  if (Env->errorOnFailure)
    return JS_FALSE;
  dbp = 0;
 }

 if (!dbp || !*dbp)
   ERR_MSG(ODBC,new,Error);

 SETPRIVATE(obj,DBPointer,dbp,true,NULL);
// JS_SetPrivate(cx,obj,dt);

 return JS_TRUE;
}

void ODBC_JSFinalize(JSContext *cx, JSObject *obj)
{
 DELPRIVATE(DBPointer);
}

static JSBool
ODBC_Close(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 CLOSEPRIVATE(ODBC,DBPointer);
 RETBOOL(true);
}

static JSBool
ODBC_ToString(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);
 RETSTR((*t)->DriverName);
}

//wrap_env.cpp
jsval* nameList(JSContext *cx,Strings& names, int& count);

static JSBool
ODBC_ListTables(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);

 TStr Error;
 STMPointer stm((*t)->ListTables(*t,Error));
 if (!stm)
 {
  return JS_TRUE;
 }

 ODBCResultInfo Result;
 stm->Bind(Result);

 TStringList names;
 while(stm->Fetch())
     names.Add(Result.Get(3));

 int count;
 jsval* arr = nameList(cx, names, count);
 *rval = OBJECT_TO_JSVAL(JS_NewArrayObject(cx,count,arr));
 if (arr) delete[] arr;
 return JS_TRUE;
}

static JSBool
ODBC_Columns(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);

 if (argc == 0) ERR_COUNT(ODBC,Keys);

 GETUTF8(0);

 JSBool longFormat = JS_FALSE;
 if (argc > 1)
  JS_ValueToBoolean(cx, argv[1], &longFormat);

 TStr Error;
 STMPointer stm((*t)->ListColumns(*t,u0,Error));
 if (!stm)
 {
  return JS_TRUE;
 }

 TStringList names;
 ODBCResultInfo Result;
 stm->Bind(Result);

 while(stm->Fetch())
 {
     if (longFormat)
     {
         char length[64];
         const char* l = Result.Get(7);
         const char* prec = Result.Get(9);
         if (*l && *prec) sprintf(length,"(%s,%s)",l,prec);
         else if (*l) sprintf(length,"(%s)",l);
         else length[0]=0;
         names.Add(new TStr(Result.Get(4)," ",Result.Get(6),length));
     }
     else
     {
         names.Add(Result.Get(4));
     }
}

 int count;
 jsval* arr = nameList(cx, names, count);
 *rval = OBJECT_TO_JSVAL(JS_NewArrayObject(cx,count,arr));
 if (arr) delete[] arr;
 return JS_TRUE;
}

static JSBool
ODBC_ListKeys(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);
 DBPointer db(*t);
 if (argc == 0) ERR_COUNT(ODBC,Keys);

 GETUTF8(0);

 TStr Error;
 STMPointer stm(db->ListKeys(db,u0,Error));
 if (!stm)
 {
  return JS_TRUE;
 }
 ODBCResultInfo Result;
 stm->Bind(Result);

 TStringList names;
 while(stm->Fetch())
 {
//    if (*s0 && s0 != Result.Get(3)) continue;
    names.Add(Result.Get(4));
 }

 int count;
 jsval* arr = nameList(cx,names, count);
 *rval = OBJECT_TO_JSVAL(JS_NewArrayObject(cx,count,arr));
 if (arr) delete[] arr;
 return JS_TRUE;
}

WRAP(ODBC,Escape)
{
 GETOBJ(ODBC,DBPointer,t);
 if (!argc) ERR_COUNT(ODBC,Escape);

 GETUTF8(0);
 MemoryStream out;
 EscapeSQL(out, u0, (*t)->escapeChar);
 RETSTRWC(out);
}

static JSBool
ODBC_Exec(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);
 if (!argc) ERR_COUNT(ODBC,Exec);

 JSString* j0;
 char * s0;
 GETSTRING(0);
 if (!s0) ERR_TYPE(ODBC,Exec,1,String);
 if (!*s0) ERR_TYPE(ODBC,Exec,1,String);

 TStr Error;
 STMPointer stm((*t)->OpenTable((*t),s0,Error));

 if (*Error) goto error;

 if (!stm) //success, no rows returned
  RETBOOL(true);

 if (argc > 1 && JSVAL_IS_OBJECT(argv[1]))
  if (JS_ObjectIsFunction(cx,JSVAL_TO_OBJECT(argv[1])))
    {
     ODBCResultInfo * Result = new ODBCResultInfo;
     stm->Bind(*Result);

     int cbargc =0;
     jsval* cbargv = 0;
     JSObject* rec = JSVAL_NULL;
     jsval callback = JSVAL_NULL;
     jsval status = JSVAL_NULL;
     int nCallback = 0;

     callback = argv[1];
     rec = Record_Object(cx,Result,true,NULL);
     JS_AddRoot(cx,&rec);
     cbargc = argc - 1;
     cbargv = argv + 1;
     cbargv[0] = OBJECT_TO_JSVAL(rec);

     while (stm->Fetch())
     {
      JS_CallFunctionValue(cx, obj, callback, cbargc, cbargv, &status);
      if (!JSVAL_TO_BOOLEAN(status))
        break;
     }

     JS_RemoveRoot(cx,&rec);
    }

 if (*Error)
  {
 error:
   *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(char*)Error));
   JS_SetProperty(cx, obj,"error",rval);
   RETBOOL(false);
  }

 RETBOOL(true);
}

static JSBool
ODBC_ExecWithParam(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);
 if (argc == 0) ERR_COUNT(ODBC,ExecWithParam);

 JSString* j0;
 char * s0;
 GETSTRING(0);
 if (!s0) ERR_TYPE(ODBC,ExecWithParam,1,String);
 if (!*s0) ERR_TYPE(ODBC,ExecWithParam,1,String);

 TStringList params;
 size_t i = 1;
 while (i < argc)
 {
   JSString* ji;
   char * si;
   GETSTRING(i);
   params.Add(si);
   i++;
 }

 TStr Error;
 STMPointer stm((*t)->OpenTableWithParam((*t),s0,params,Error));

 if (*Error) goto error;

 if (!stm) //success, no rows returned
  RETBOOL(true);

 if (*Error)
  {
 error:
   *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(char*)Error));
   JS_SetProperty(cx, obj,"error",rval);
   RETBOOL(false);
  }

 RETBOOL(true);
}

/*

static JSBool
ODBC_Exec(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);
 if (!argc) ERR_COUNT(ODBC,Exec);

 JSString* j0;
 char * s0;
 GETSTRING(0);
 if (!s0) ERR_TYPE(ODBC,Exec,1,String);
 if (!*s0) ERR_TYPE(ODBC,Exec,1,String);

 TStr Error;
 bool result = (*t)->ExecDirect(s0,Error,false);
 if (!result)
 {
  *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(char*)Error));
  JS_SetProperty(cx, obj,"error",rval);
 }
 RETBOOL(result);
}
*/

static JSBool
ODBC_Table(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);
 if (!argc) ERR_COUNT(ODBC,Table);

 JSString* j0;
 char * s0;
 GETSTRING(0);
 if (!s0) ERR_TYPE(ODBC,Table,1,String);

 try {
  DataTable * table = new ODBCTable(*t,s0,0,0);
  //DBPointer handles garbage collection for ODBCDb, but we
  //also want to close all open tables if the database is
  //explicitly closed or deleted.

  //handy to pre-load the first row of the result set and to estimate
  //the result set length.

  table->GetDataC(0,0);
  RETOBJ(Table_Object(cx,table,true,GETPOINTER));
 } catch (xdb& x)
 {
  TStr Error(x.why()," ", x.info());
  *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(char*)Error));
  JS_SetProperty(cx, obj,"error",rval);
  RETOBJ(NULL);
 }
 catch(...)
 {RETOBJ(NULL);}
}

static JSBool
ODBC_Query(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);
 if (!argc) ERR_COUNT(ODBC,Query);

 JSString* j0;
 char * s0;
 GETSTRING(0);
 if (!s0) ERR_TYPE(ODBC,Query,1,String);
 if (!*s0) ERR_TYPE(ODBC,Query,1,String);

 try {
  DataTable * table = new ODBCTable(*t,0,0,s0);
  //DBPointer handles garbage collection for ODBCDb, but we
  //also want to close all open tables if the database is
  //explicitly closed or deleted.

  //handy to pre-load the first row of the result set and to estimate
  //the result set length.
  table->GetDataC(0,0);
//  table->LoadTable();
  RETOBJ(Table_Object(cx,table,true,GETPOINTER));

 } catch (xdb& x)
 {
  TStr Error(x.why()," ", x.info());
  *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(char*)Error));
  JS_SetProperty(cx, obj,"error",rval);
  RETOBJ(NULL);
 }
 catch(...)
 {RETOBJ(NULL);}

}

static JSBool
ODBC_SetAutoCommit(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);

 JSBool b = JS_TRUE;
 if (argc) TOBOOL(0,b);
 RETBOOL((*t)->SetAutoCommit(b));
}

static JSBool
ODBC_Commit(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);

 if (!(*t)->Commit())
 {
   *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(char*)(*t)->LastError));
   JS_SetProperty(cx, obj,"error",rval);
   RETBOOL(false);
 }
 else
 {
  RETBOOL(true);
 }
}

static JSBool
ODBC_Rollback(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(ODBC,DBPointer,t);

 if (!(*t)->Rollback())
 {
   *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(char*)(*t)->LastError));
   JS_SetProperty(cx, obj,"error",rval);
   RETBOOL(false);
  }
  else
  {
   RETBOOL(true);
  }
}

static JSPropertySpec ODBC_properties[] = {
    {"name",      0,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,ODBC_JSGet},
    {"className",3, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,ODBC_JSGet},
    {0}
};

static JSFunctionSpec ODBC_functions[] = {
    {"query",     ODBC_Query,      1},
    {"exec",    ODBC_Exec, 1},
    {"execWithParam",    ODBC_ExecWithParam, 2},
    {"execute",    ODBC_Exec, 1},
    {"toString",ODBC_ToString,0},
    {"escape",    ODBC_Escape, 1},
    {"tables",ODBC_ListTables,0},
    {"keys",ODBC_ListKeys,1},
    {"columns",ODBC_Columns,2},
    {"close",ODBC_Close,0},
    {"table",ODBC_Table,1},
    {"commit",ODBC_Commit,1},
    {"setAutoCommit",ODBC_SetAutoCommit,1},
    {"rollback",ODBC_Rollback,1},
    {0}
};

static JSFunctionSpec ODBC_fnstatic[] = {
    {"help",  ODBC_HELP,    0},
    {0}
};

static JSClass ODBC_class = {
    "ODBC", JSCLASS_HAS_PRIVATE,         //ODBC_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,ODBC_JSFinalize
};

JSObject*
ODBC_Object(JSContext *cx, DBPointer* t,bool autodelete,JSPointerBase* Parent)
{
 JSObject* obj;
 GETENV;
 MAKENEW(ODBC);
 SETPRIVATE(obj,DBPointer,t,autodelete,Parent);


 return obj;
}

JSClass* ODBC_Class() {return &ODBC_class;}

void ODBC_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(ODBC);
}

#endif
