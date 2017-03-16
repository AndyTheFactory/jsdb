#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

#ifndef TBL_NO_SQLITE

#include "sqlite/sqlite3.h"

//fool the compiler into treating sqlite3 as an opaque pointer
struct sqlite3
{
int x;
};
//gcc.exe -c -fexceptions -I . -idirafter rs/ -idirafter js/  -D NO_NOTES_MAIL -D NO_CMC_MAIL -D NO_DBF_ENCRYPTION -D XP_WIN -D _Windows -D XP_WIN -D _HAS_INT64   -O2 -Os   -oobj\wrap_sqlite.o rs/wrap_sqlite.cpp

int step(sqlite3_stmt* s)
{
    int rc;
    int tries = 0;
#ifndef XP_WIN
    int delay = 1, lastDelay = 1;
#endif
    do{
       rc = sqlite3_step(s);
       if (rc == SQLITE_BUSY)
       {
#ifdef XP_WIN
        Sleep(0);
#else
        usleep(delay << 4);
        delay = delay + lastDelay;
        lastDelay = delay - lastDelay;
#endif
       }
    } while((rc == SQLITE_LOCKED || rc == SQLITE_BUSY) && tries++ < 16);
    return rc;
}

static JSClass* SQLite_Class();

class CallbackData{
  public:

  JSContext *cx;
  JSObject *obj;
  jsval fval;
  int narg;

  CallbackData(JSContext *cxi,JSObject *obji, jsval fvali,  int nargi){
    cx=cxi;
    obj=obji;
    fval=fvali;
    narg=nargi;
  };

};

class SQLite
{
    public:
    sqlite3* db;
    sqlite3_stmt *pStmt;
    TStr fileName;
    TStr lastError;
    sqlite_int64 lastRowid;
    SQLite(const char* name = NULL);
    ~SQLite();
    TList<JSRoot> functions;
    TList<CallbackData> callbackdata;
};

SQLite::SQLite(const char* name) : db(0), fileName(name)
{  // Database filename (UTF-8)
   int r = sqlite3_open_v2( name ? name : ":memory:",
      &db, SQLITE_OPEN_READWRITE| SQLITE_OPEN_CREATE,NULL);
   if (r != SQLITE_OK)
   {
    sqlite3_close(db);
    db = 0;
    lastError = itos(r);
   }
   pStmt = 0;
}

SQLite::~SQLite()
{
 if (pStmt) sqlite3_finalize(pStmt);
 if (db) sqlite3_close(db);
}

#if JS_VERSION > 180
#define JS_AddRoot(cx,rp) JS_AddNamedObjectRoot((cx), (rp), __FILE__ )
#define JS_RemoveRoot(cx,rp) JS_RemoveObjectRoot(cx,rp)
#endif
//function callback(record,parameters)
//SQLite.exec("select * from foo",callback,parameters)
WRAP(SQLite,Exec)
{
  GETOBJ(SQLite,SQLite,t);

  GETARGS;
  int rc = SQLITE_OK;
  const char *zLeftover;

  if (t->pStmt)
   ERR_MSG(SQLite3,exec,"The database is already executing a query")

  t->lastError = 0;
  t->pStmt = 0;

  char *zErrMsg = 0;

  int cbargc =0;
  jsval* cbargv = 0;

  if (argc == 0 || !ISSTR(0))
   ERR_TYPE(SQLite,Exec,1,String);

  GETUTF8(0);
  if (!*u0) RETBOOL(true);

  sqlite3* db = t->db;
  const char * zSql = u0;

  int nRetry = 0;
  //int nCallback = 0;

  TParameterList *r = 0;
  JSObject* rec = 0;
  jsval callback = JSVAL_NULL;
  jsval status = JSVAL_NULL;

  if (argc > 1 && JSVAL_IS_OBJECT(ARGV(1)))
    if (JS_ObjectIsFunction(cx,JSVAL_TO_OBJECT(argv[1])))
    {
        r = new TParameterList;
        callback = argv[1];
        rec = Record_Object(cx,r,true,NULL);
        JS_AddRoot(cx,&rec);
        cbargc = argc - 1;
        cbargv = argv + 1;
        cbargv[0] = OBJECT_TO_JSVAL(rec);
    }

  //we're using t->pStmt as a lock to prevent reentrant access to the database
  //sqlite3_mutex_enter(db->mutex);

  int nCol = 0;
  int maxRetry = 100;

#ifndef XP_WIN
  int delay, lastDelay;
#endif

  // sqlite3_prepare can return SQLITE_LOCKED or SQLITE_OK
  // sqlite3_step can return SQLITE_BUSY or SQLITE_OK
  // I think ... so test for both in the retry loop

  //
  if( t->pStmt ) sqlite3_finalize(t->pStmt);

  //keep looping through statements in the command.
  //If locked, wait and repeat. Otherwise, call step() while it succeeds
  while( (rc==SQLITE_OK || (rc==SQLITE_SCHEMA && (++nRetry)< maxRetry)) && zSql[0] )
  {
    t->pStmt = 0;

#ifndef XP_WIN
    delay = lastDelay = 1;
#endif
  //locked? wait for the lock to expire.
  prepare:
    rc = sqlite3_prepare_v2(db, zSql, -1, &t->pStmt, &zLeftover);
    if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED)
    {
#ifdef XP_WIN
        Sleep(0);
#else
        usleep(delay << 4);
        delay = delay + lastDelay;
        lastDelay = delay - lastDelay;
#endif
        if ((++nRetry)< maxRetry)
          goto prepare;
    }

    if( rc != SQLITE_OK ) //SQLITE_SCHEMA, perhaps?
      continue;

    if( !t->pStmt ) //comment or whitespace
      continue;

//    nCallback = 0;

    size_t i;

    do
    {
        //loops
        rc = step(t->pStmt);
      //we could make a more efficient Record class later that didn't
      //allocate its own copies of the data values
        int colCount = sqlite3_column_count(t->pStmt);
        if (r)
        {
            r->Clear();
            for(i=0; i < colCount; i++)
            {
    #if DEBUG
        printf("%d %s %s\n",i,
               (char *)sqlite3_column_name(t->pStmt, i),
               (char *)sqlite3_column_text(t->pStmt, i));
    #endif

    //        if (nCallback == 0)
              r->Set((char *)sqlite3_column_name(t->pStmt, i),
                    (char *)sqlite3_column_text(t->pStmt, i));
    //        else
    //          r->Set(i,
    //                (char *)sqlite3_column_text(t->pStmt, i));
            }
        }
//      nCallback++;
       if ( rc == SQLITE_ROW )
        {
         if (r)
            JS_CallFunctionValue(cx, obj, callback, cbargc, cbargv, &status);
        if (!JSVAL_TO_BOOLEAN(status))
        {
          rc = SQLITE_ABORT;
          goto exec_out;
        }
       }
    } while( rc == SQLITE_ROW );

    rc = sqlite3_finalize(t->pStmt);
    t->pStmt = 0;
    if( rc != SQLITE_SCHEMA )
    {
        nRetry = 0; //whew!
        zSql = zLeftover;
        while( isspace((unsigned char)zSql[0]) ) zSql++;
    }
  }

exec_out:
  if (r)
  {
   cbargv[0] = callback;
   JS_RemoveRoot(cx,&rec);
  }

  if( t->pStmt ) sqlite3_finalize(t->pStmt);
  t->pStmt = 0;

  bool ret = true;

  if( rc != SQLITE_OK && rc==sqlite3_errcode(db))
  {
    t->lastError = sqlite3_errmsg(db);
    ret = false;
  }

//sqlite3_mutex_leave(db->mutex);
 RETBOOL(ret);
}

Stream* BlobToStream(JSContext* cx, sqlite3_blob* blob)
{
      MemoryStream* s = new MemoryStream;
      int length = sqlite3_blob_bytes(blob);
      int start = 0;
      char z[1024];
      while (length)
      {
          int c = sqlite3_blob_read(blob, z, min(length,sizeof(z)),start);
          s->write(z, c);
          start += c;
          length -= c;
      }
      s->rewind();
      return s;
}

void SQLiteCallbackFunc(sqlite3_context* context,int argc, sqlite3_value **argv){
  CallbackData *data;
  jsval *jsargv;
  int n;

  //retrieve the information about the JS function
  data=(CallbackData *)sqlite3_user_data(context);


  //regardless of the requested number of parameters, the function gets
  //the number that is supplied i.e. argc
  jsargv= new jsval[argc]; //malloc(sizeof(jsval)*argc);

  //create the argv list for the JS function
  for(n=0;n<argc;n++){
    switch(sqlite3_value_type(argv[n])){
      case SQLITE_INTEGER:
       jsargv[n]=INT_TO_JSVAL(sqlite3_value_int(argv[n]));
      break;
      case SQLITE_FLOAT:
       //jsargv[n]=DOUBLE_TO_JSVAL(sqlite3_value_double(argv[n]));
       jsargv[n]=JS_NewNumberValue(data->cx,sqlite3_value_double(argv[n]),&jsargv[n]);
      break;
      case SQLITE_BLOB:
          jsargv[n] = OBJECT_TO_JSVAL(Stream_Object(data->cx, BlobToStream(data->cx,(sqlite3_blob*) sqlite3_value_blob(argv[n])), true, 0));
      break;
      case SQLITE3_TEXT:
       jsargv[n]=STRING_TO_JSVAL(JS_NewUCStringCopyZ(data->cx,(const jschar*)sqlite3_value_text16(argv[n])));
      break;
      case SQLITE_NULL: //everything else is null
      default:
      jsargv[n]=JSVAL_NULL;
      break;
    }
  }
  //Call the stored function and retrieve the result into the proper
  //data type
  jsval rval;
  if(JS_CallFunctionValue(data->cx,data->obj,data->fval,argc,jsargv,&rval)){
      if(JSVAL_IS_INT(rval)){
         sqlite3_result_int(context,JSVAL_TO_INT(rval));
      }else if(JSVAL_IS_DOUBLE(rval)){
#if JS_VERSION > 180
         sqlite3_result_double(context,JSVAL_TO_DOUBLE(rval));
#else
         sqlite3_result_double(context,*JSVAL_TO_DOUBLE(rval));
#endif
      }else if(JSVAL_IS_BOOLEAN(rval)){
         sqlite3_result_int(context,JSVAL_TO_BOOLEAN(rval));
      }else if(JSVAL_IS_NULL(rval)){
         sqlite3_result_null(context);
      }else{ //everything else is a string or converted to it (I hope)
         JSString* s = JS_ValueToString(data->cx,rval);
         sqlite3_result_text16(context,JS_GetStringCharsZ(data->cx,s),-1,NULL);
      }
  }
  //release the argumentlist
  delete [] jsargv;
}


WRAP(SQLite,CreateFunction)
{
 GETOBJ(SQLite,SQLite,t);
 GETARGS;
 //createFunction(function name(a, b){return a+b}, extras);
 //createFunction(name, function(a, b){return a+b}, extras);
 //createFunction(name, argcount, function(a, b){return a+b}, extras);
 //createFunction(argcount, function(a, b){return a+b}, extras);
 size_t index = 0;
 JSString* name = 0;
 int narg = 0;

 if (argc == 0)
  ERR_COUNT(SQLite,CreateFunction);

 if (argc > 1 && ISSTR(0))
 {
   name = JSVAL_TO_STRING(argv[0]);
   index++;
 }

 if (argc > (index + 1) && ISINT(index))
  {
    narg = INT(index);
    index++;
  }

 if(!JS_ObjectIsFunction(cx,JSVAL_TO_OBJECT(argv[index])))
  ERR_TYPE(SQLite,CreateFunction,index,Function)

 //root the function
 t->functions.Add(new JSRoot(cx,argv[index]));

 JSFunction * fun = JS_ValueToFunction(cx,argv[index]);

 //how many parameters?
 if (!narg) narg = JS_GetFunctionArity(fun);
 if (!name) name = JS_GetFunctionId(fun);

 //store relevant data in the CallbackData objectlist
 CallbackData * cb = new CallbackData(cx,obj,argv[index],narg);
 t->callbackdata.Add(cb);

 //create the actual function
 //Q: should we throw an error instead?
 if (SQLITE_OK != sqlite3_create_function16(t->db,JS_GetStringChars(name),narg,SQLITE_UTF8,
            cb,SQLiteCallbackFunc,NULL,NULL)==SQLITE_ERROR)
 {
   t->lastError = sqlite3_errmsg(t->db);
   RETBOOL(false);
 }else{
   RETBOOL(true);
 };
}

JSBool
SQLite_JSGet(JSContext *cx, JSObject *obj, jsid id, jsval *rval)
{
 const char * c = 0;
 count_t i = 0;

 GETOBJ2(SQLite,SQLite,t);

 int x = JSVAL_TO_INT(id);

 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: c = t->fileName; break;
   case 3: c = "SQLite"; break;
   case 4: c = t->lastError; break;
   case 5: i = sqlite3_last_insert_rowid(t->db); break;
   default: return JS_FALSE;
  }
 else return JS_FALSE;

 if (c) RETSTR(c);
 RETINT(i);
}

WRAP(SQLite,ToString)
{
 GETOBJ(SQLite,SQLite,t);
 RETSTR(t->fileName);
}

//wrap_env.cpp
jsval* nameList(JSContext *cx,Strings& names, int& count);

int QueryList(sqlite3* db, const char* zSql, TStringList* result, DataTable* table, JSContext*cx=0, JSObject* array = 0)
{
    sqlite3_stmt *pStmt;
    int rc;
    int row = 0;
    int cols = 0;
    int index = 0;

  prepare:
    rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
    if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED)
    {
#ifdef XP_WIN
 SleepEx(10,true);
#else
 usleep(10000);
#endif
     goto prepare;
    }

    if( rc!=SQLITE_OK ) return sqlite3_errcode(db);

    cols = sqlite3_column_count(pStmt);

    while(SQLITE_ROW==step(pStmt) )
    {
      if (array)
      {
       JSObject * o = JS_NewObject(cx, NULL, NULL, NULL);
       jsval v=OBJECT_TO_JSVAL(o);
       JS_SetElement(cx, array, index++, &v);
       for (int i=0; i<cols; i++)
       {
         jsval ptr = STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx,(const jschar*)sqlite3_column_text16(pStmt,i)));
         const jschar* name = (const jschar*)sqlite3_column_name16(pStmt,i);
         JS_SetUCProperty(cx, o, name,wcslen((wchar_t*)name),&ptr);
       }
      }
      if (table)
      {
       if (row == 0)
         for (int i=0; i<cols; i++)
           table->SetDataC(0,i+1,sqlite3_column_name(pStmt,i));

       row++;
       for (int i=0; i<cols; i++)
        table->SetDataC(row,i+1,(const char*)sqlite3_column_text(pStmt,i));
      }
      if (result) result->Add((const char*)sqlite3_column_text(pStmt, 0));
    }

  return sqlite3_finalize(pStmt);
}

void escapeSQLite(Stream& out, const char* in,size_t length= NOT_FOUND)
{
    if (length == NOT_FOUND) length = strlen(in);
 out.write("\'",1);
 for (size_t i=0; i<length; i++)
 {
  if (*in == '\'') out.write("\'",1);
  out.write(in++,1);
 }
 out.write("\'",1);
}

void escapeSQLite(TStr& out, const char* in)
{
 out.Resize(2*strlen(in) + 2);
 size_t i=1;
 out[0] = '\'';
 while (*in)
 {
  if (*in == '\'') out[i++] = '\'';
  out[i++]= *in++;
 }
 out[i] = '\'';
}

WRAP(SQLite,Insert) //table, object
{
 GETOBJ(SQLite,SQLite,t);
 RETINT(sqlite3_last_insert_rowid(t->db));
}

WRAP(SQLite,Select) //table, object|record|string
{
 if (argc == 0)
    ERR_TYPE(SQLite,Select,1,String);

 ENTERNATIVE(cx);
 GETARGS;
 GETOBJ(SQLite,SQLite,t);
 GETUTF8(0);
 TStr where;
 TStr table;
 escapeSQLite(table,u0);

 if (argc > 1)
 {
   TStringList fields;
   TStr query("SELECT ","sql"," FROM sqlite_master WHERE type='table' AND rootpage>0 AND name","=",table);
   QueryList(t->db,query,&fields,0);
   TNameValueList* r1;
   GETREC1(1,r1);
   JSObject* o = JSVAL_IS_OBJECT(argv[1]) ? JSVAL_TO_OBJECT(argv[1]) : 0;

   if (r1)
   {
    MemoryStream m;
    int any = 0;
    for (size_t i=0; i<r1->Count(); i++)
    {
      if (!fields.Has(r1->Name(i))) continue;
      if (any++) m << " AND ";
      escapeSQLite(m,r1->Name(i));
      m << "=";
      escapeSQLite(m,r1->Value(i));
    }
    where = m;
   }
   else if (o)
   {
    MemoryStream m;
    int any = 0;
     FOREACH(const char* n, fields)
        jsval v=0;
        JS_GetProperty(cx, o, n, &v);

        if (JSVAL_IS_VOID(v)) continue;
        if (JSVAL_IS_NULL(v)) continue;


      if (any++) m << " AND ";
      escapeSQLite(m,n);
      m << "=";
        JSString * value = JS_ValueToString(cx,v);
        GETUTF8(v);
        escapeSQLite(m,uv);
        //escapeSQLite(m,TStr(JS_GetStringChars(value),JS_GetStringLength(value)));
       DONEFOREACH
    where = m;
   }
   else if (ISSTR(1))
   {
          GETUTF8(1);
          where.Exchange(u1);
    }
    else ERR_TYPE(SQLite,Select,2,Query);

 }

 JSObject* array = ROOT(JS_NewArrayObject(cx,0,NULL));
 TStr query("SELECT * from ",table, *where && strncasecmp(where,"WHERE",5) ? " WHERE " : " ", where);
// printf(query);

 QueryList(t->db, query, 0, 0, cx, array);
 RETOBJ(array);
}

WRAP(SQLite,Update) //table, object
{

 RETBOOL(true);
}

WRAP(SQLite,Escape)
{
 if (argc == 0) RETSTRW(L"\'\'");

 GETARGS;
 int j;
 size_t length = 2;
 uint16* out;
 uint16** strs = new uint16*[argc];
 for (j=0; j<argc; j++)
 {
  GETUCS2(j);
  if (!sj)
  {
    JS_SetPendingException(cx,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,"Can't convert parameter too a string")));
    delete[] strs;
    return JS_FALSE;
  }
  strs[j]=sj;
  out = sj;
  while (*out)
  {
   if (*out == L'\'') length++;
   length++;
   out++;
  }
 }
 WStr result(length);
 out = result;
 size_t i=1;
 out[0] = L'\'';
 for (j=0; j<argc; j++)
 {
  uint16* s0 = strs[j];
  while (*s0)
  {
   if (*s0 == L'\'') out[i++] = L'\'';
   out[i++]= *s0++;
  }
 }
 out[i] = L'\'';
 delete[] strs;
 RETSTRW(out);
}

WRAP(SQLite,Table)
{
 GETOBJ(SQLite,SQLite,t);
 GETUTF8(0);

 SpreadsheetTable *table = new SpreadsheetTable;
 TStr name;
 escapeSQLite(name,u0);
 TStr query("SELECT * from ",name);

 QueryList(t->db, query, 0, table);
 RETOBJ(Table_Object(cx, table,true,0));
}

WRAP(SQLite,Tables)
{
 GETOBJ(SQLite,SQLite,t);
 TStringList result;

 //allow merging of duplicate strings
 TStr query("SELECT ","name"," FROM sqlite_master WHERE type='table' AND rootpage>0 AND name","!=","'sqlite_sequence'");
 QueryList(t->db,query,&result,0);
 int count = 0;
 jsval* arr = nameList(cx, result, count);
 RETOBJ(JS_NewArrayObject(cx,count,arr));
 return JS_TRUE;
}

WRAP(SQLite,Keys)
{
 GETENV;
 GETOBJ(SQLite,SQLite,t);
 if (argc == 0 || !ISSTR(0))
    ERR_TYPE(SQLite,Keys,1,String);

 GETUTF8(0);
 if (!*u0) RETOBJ(NULL);

 TStringList sql;
 TStringList keys;
 TStr table;
 escapeSQLite(table,u0);
 //allow merging of duplicate strings
 TStr query("SELECT ","sql"," FROM sqlite_master WHERE type='table' AND rootpage>0 AND name","=",table);
 QueryList(t->db,query,&sql,0);

 if (!sql.Count()) RETOBJ(NULL);

 char * text = strstr(sql[0],"(");
 if (!text) RETOBJ(NULL);

 text[strlen(text)-1]=0;
 TStringList fields(text+1,",");
 FOREACH(char* c, fields)
 while (*c == ' ') c++;
  if (!stristr(c," PRIMARY KEY"))
   continue;
  *strchr(c,' ') = 0;
  keys.Add(c);
 DONEFOREACH
 int count = 0;
 jsval* arr = nameList(cx, keys, count);
 RETOBJ(JS_NewArrayObject(cx,count,arr));
 return JS_TRUE;
}

//returns COL_NAME or the complete construction string
WRAP(SQLite,Columns)
{
 GETENV;
 GETOBJ(SQLite,SQLite,t);
 if (argc == 0 || !ISSTR(0))
    ERR_TYPE(SQLite,Columns,1,String);

 GETUTF8(0);
 if (!*u0) RETOBJ(NULL);

 JSBool longFormat = JS_FALSE;
 if (argc > 1)
  JS_ValueToBoolean(cx, ARGV(1), &longFormat);

 TStringList sql;
 TStringList keys;
 TStr table;
 escapeSQLite(table,u0);
 //allow merging of duplicate strings
 TStr query("SELECT ","sql"," FROM sqlite_master WHERE type='table' AND rootpage>0 AND name","=",table);
 QueryList(t->db,query,&sql,0);

 if (!sql.Count()) RETOBJ(NULL);

 char * text = strstr(sql[0],"(");
 if (!text) RETOBJ(NULL);

 text[strlen(text)-1]=0;
 TStringList fields(text+1,",");
 FOREACH(char* c, fields)
 if (!longFormat)
 {
     char * n = c;
  while (IsSpace(*c)) c++;
  char* d = strchr(c,' ');
  if (d) *d = 0;
  strcpy(n,c);
 }
 DONEFOREACH
 int count = 0;
 jsval* arr = nameList(cx, fields, count);
 RETOBJ(JS_NewArrayObject(cx,count,arr));
 return JS_TRUE;
}

WRAP(SQLite,Close)
{
 GETTHIS;
 CLOSEPRIVATE(SQLite,SQLite);
 RETBOOL(true);
}

void SQLite_JSFinalize(JSContext *cx, JSObject *obj)
{
 DELPRIVATE(SQLite);
}

static JSPropertySpec SQLite_properties[] = {
    {"name",      0,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,SQLite_JSGet},
    {"className",3, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,SQLite_JSGet},
    {"error",4,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,SQLite_JSGet},
    {"lastID",5,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,SQLite_JSGet},
    {"lastId",5,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,SQLite_JSGet},
    {"lastid",5,JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,SQLite_JSGet},
    {0}
};

static JSFunctionSpec SQLite_functions[] = {
    {"exec",     SQLite_Exec,      4},
    {"execute",     SQLite_Exec,      4},
//    {"blob",     SQLite_Blob,      1}
    {"close",SQLite_Close,0},
    {"toString",SQLite_ToString,0},
//    {"query",     ODBC_Query,      1},
    {"escape",SQLite_Escape,1},
    {"tables",SQLite_Tables,0},
    {"select",SQLite_Select,2},
    {"keys",SQLite_Keys,1},
    {"table",SQLite_Table,1},
    {"columns",SQLite_Columns,2},
    {"createFunction",SQLite_CreateFunction,3},
    {0}
};

static JSFunctionSpec SQLite_fnstatic[] = {
//    {"help",  SQLite_HELP,    0},
    {"escape",SQLite_Escape,1},
    {0}
};

static JSClass SQLite_class = {
    "SQLite", JSCLASS_HAS_PRIVATE,         //SQLite_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,SQLite_JSFinalize
};


JSObject*
SQLite_Object(JSContext *cx, SQLite* t,bool autodelete,JSPointerBase* Parent)
{
 JSObject* obj;
 GETENV;
 ENTERNATIVE(cx);
 MAKENEW(SQLite);
 SETPRIVATE(obj,SQLite,t,autodelete,Parent);

 return obj;
}

//JSBool
//SQLite_SQLite(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
//    jsval *rval)
NATIVE(SQLite_SQLite)
{
 GETENV;
 if (!Env) return JS_FALSE;

 SQLite * dt = NULL;

 try {
 if (argc == 1)
 {
     if (ISSTR(0))
     {
        GETUTF8(0);
        dt = new SQLite(u0);
     }
     else
        ERR_TYPE(SQLite,new,1,String);
 }
 else
     dt = new SQLite();
 } catch (...) {dt = NULL;}

 if (!dt)
 {
  if (Env->errorOnFailure)
    return JS_FALSE;
  ERR_MSG(SQLite,"Open SQLite failed","");
 }

 if (dt)
 {
  CONSTRUCTOR(SQLite,dt,true,NULL);
 }
// JS_SetPrivate(cx,obj,dt);

 return JS_TRUE;
}

static JSClass* SQLite_Class() {return &SQLite_class;}

void SQLite_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(SQLite);
}

#endif //TBL_NO_SQLITE
