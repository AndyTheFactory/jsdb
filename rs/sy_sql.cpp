#include "rslib.h"
#pragma hdrstop
#ifndef TBL_NO_SQL
#include "rs/sql.h"

//extern TParameterList GlobalOptions;
#define Log(x,y,z) {}
#define LogExec(x,y) {}
/*
void Log(int line,cchar* action, void* handle,cchar* extra=0)
{
 cchar* filename = GlobalOptions["SQL.FUNCLOG"];
 if (!*filename) return;
 FILE* f = fopen(filename,"at");
 if (!f) return;
 if (handle) fprintf(f,"%d:\t%s 0x%x\n",line,action,(int32)handle);
 if (extra) fprintf(f,"\t%s\n",extra);
 fclose(f);
}

void LogExec(const char* statement, const char* result = 0)
{
 const char* filename = GlobalOptions["SQL.EXECLOG"];
 if (!*filename) return;
 FILE* f = fopen(filename,"at");
 if (!f) return;
 if (statement) fprintf(f,"Statement\n%s\n\n",statement);
 if (result) fprintf(f,"Result\n%s\n\n",result);
 fclose(f);
}
*/

void ListErrors(ODBCEnv* ,TStr& Error, SQLHANDLE hHandle, SQLSMALLINT hType)
{
   MemoryStream Message;
    SQLSMALLINT iRec = 1;
    SQLINTEGER  iError;
     SQLCHAR szMessage[1000], szState[100];
   SQLSMALLINT len=0;

   szMessage[0]=0;
   memset(szState,0,sizeof(szState));;

    while (SQLGetDiagRec(hType,
                         hHandle,
                         iRec++,
                         szState,
                         &iError,
                         szMessage,
                         sizeof(szMessage),
                         &len) == SQL_SUCCESS)
    {
    // Hide "data truncated" messages
    Message << "[" << (char*)szState << "] ";
    Message << (char*)szMessage;
    Message << "(" << iError << ")\n";
    }

   Error = Message;
}

void HandleError(ODBCEnv* Env,
                 SQLHANDLE     hHandle,
                    SQLSMALLINT hType,
                    RETCODE      RetCode,
                 xdb& err)
{
    if (RetCode == SQL_INVALID_HANDLE)
    {
        err=xdb("Invalid handle!\n");
    }

   if (RetCode != SQL_ERROR)
   {
      char Error[128];
      sprintf(Error,"Unexpected return code %d!\n",RetCode);
      err=xdb(Error);
   }

   TStr Err;
   ListErrors(Env,Err,hHandle,hType);
   err=xdb(Err);
}

void EscapeSQL(Stream& out, const char * in, int escape)
{
 char escapechar = escape;
 out.write("\'",1);
 if (!in) return;
 while (*in)
  {
   if (*in == escapechar)
      {
       out.write(&escapechar,1);
       out.write(&escapechar,1);
      }
   else if (*in == '\'')
      {
       out.write("\'\'",2);
      }
/*   else if (escapechar && (*in == '_' || *in == '%'))
      {
       out.write(&escapechar,1);
       out.write(in,1);
      }
*/   else out.write(in,1);
   in++;
  }
 out.write("\'",1);
}

//-----------------------------------

ODBCColumn::ODBCColumn() {}
ODBCColumn::~ODBCColumn() {}
void ODBCResultInfo::Clear()
{
 FOREACH(ODBCColumn* x, Data)
  x->sData[0]=0; //Clear out the old data. Oracle doesn't set null values
 DONEFOREACH
}

size_t ODBCResultInfo::FieldNumber(const char * c)
{
 FOREACH(ODBCColumn* x, Data)
  if (x->FieldName == c) return i;
 DONEFOREACH
 return NOT_FOUND;
}

//----------------------------------

ODBCStm::ODBCStm(ODBCEnv* Env,SQLHSTMT s) : SQL(Env), hstmt(s),db(0)
 {
  CurrentRow = 0;
  ColumnCount = 0;
  Results = 0;
 }

ODBCStm::~ODBCStm()
 {
  if (hstmt != SQL_NULL_HSTMT)
   {
     Log(__LINE__,"free statement",hstmt);
     SQLFreeStmt(hstmt,SQL_UNBIND);
     SQLFreeHandle(SQL_HANDLE_STMT, hstmt);      /* free the statement */
     hstmt = SQL_NULL_HSTMT;
     CurrentRow = 0;
   }
  Results = 0;
 }

bool ODBCStm::Rewind()
{
 CurrentRow = 0;

 int r = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,0);
 if (!SQL_SUCCEEDED(r))
   {
    r = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,0);
    if (SQL_SUCCEEDED(r)) CurrentRow = 1;
   }

 if (!SQL_SUCCEEDED(r))
   {
    ListErrors(SQL,LastError,hstmt,SQL_HANDLE_STMT);
    return false;
   }
 return true;
}

bool ODBCStm::Fetch(int32 i)
{
 if (CurrentRow && CurrentRow == i) return true;

 if (RowCount > 0 && i > RowCount) return false;

 int r;

 if (Results) Results->Clear();

 if (i == 0 || (i == (CurrentRow + 1)))
  {
   CurrentRow ++;
   r = SQLFetch(hstmt);

   if (r == SQL_NO_DATA_FOUND) //the end!
   {
    RowCount = CurrentRow - 1;
    LastError = 0;
    CurrentRow = 0;
    return false;
   }
  }
 else
  {
   r = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,i);
   if (SQL_SUCCEEDED(r)) CurrentRow = i;
   else if (CurrentRow == 0)
    {
     for (int x =0; x<i; x++)
       {
        if (x == i - 1)
         if (Results) Results->Clear();
        r = SQLFetch(hstmt);
        if (!SQL_SUCCEEDED(r)) break;
        CurrentRow ++;
       }
    }
  }

 if (!SQL_SUCCEEDED(r))
   {
    if (Results && i==1) Results->Clear();
    r = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,0);

    if (SQL_SUCCEEDED(r))
     {
      CurrentRow = 0;
      while (CurrentRow < i && SQL_SUCCEEDED(r))
       {
        CurrentRow ++;
        if (CurrentRow == i)
         if (Results) Results->Clear();
        r = SQLFetch(hstmt);
       }
     }
     else if (Results) Results->Clear();
   }
 if (!SQL_SUCCEEDED(r))
 {
  CurrentRow = 0;
 }
 return SQL_SUCCEEDED(r);
}

void ODBCStm::Bind(ODBCResultInfo& info)
{
 if (info.Data.Count())
 {
  Log(__LINE__,"unbind",hstmt);
  SQLFreeStmt(hstmt,SQL_UNBIND);
  info.Data.Clear();
  //unbind the old results list
 }

 Results = &info;

 CurrentRow = 0;

 long int d,size;
 char szTitle[256];

 for (SQLUSMALLINT i=1; i <= (SQLUSMALLINT)ColumnCount; i++)
    {
    ODBCColumn* b = new ODBCColumn;
    info.Data.Add(b);

        // Figure out if this is a character or numeric column; this is
        // used to determine if we want to display the data left- or right-
        // aligned.

        // !! Note a bug in the 3.x documentation.  We claim that
        // SQL_DESC_TYPE is a 1.x feature.   That is not true, SQL_DESC_TYPE
        // is a 3.x feature.   SQL_DESC_CONCISE_TYPE maps to the 1.x
        // SQL_COLUMN_TYPE.   This is what you must use if you want to work
        // against a 2.x driver.  Sorry for the inconvenience...

      d=0;

        SQLColAttribute(hstmt,
                                i,
                                SQL_DESC_CONCISE_TYPE,
                                NULL,
                                0,
                                NULL,
                                &d);

        b->NeedsQuotes =
                 (d == SQL_CHAR ||
                        d == SQL_VARCHAR ||
                        d == SQL_LONGVARCHAR);

      b->IsNumber =
                 (d ==  SQL_NUMERIC ||
                  d ==  SQL_DECIMAL ||
                  d ==  SQL_INTEGER ||
                  d ==  SQL_SMALLINT ||
                  d ==  SQL_FLOAT ||
                  d ==  SQL_REAL ||
                  d ==  SQL_DOUBLE);

      b->Type = d;

      d = 0;
      SQLColAttribute(hstmt,
                                i,
                                SQL_DESC_AUTO_UNIQUE_VALUE,
                                NULL,
                                0,
                                NULL,
                                &d);
      b->AutoIncrement = d;

        // Allocate a buffer big enough to hold the text representation
        // of the data.  Add one character for the null terminator

      size=0;
      int r = SQLColAttribute(hstmt,
                                i,
                                SQL_DESC_LENGTH,
                                NULL,
                                0,
                                NULL,
                                &size);

 #define MAXSIZE (1024*1024)

      long int displaySize = 0;
      int r2 = SQLColAttribute(hstmt,
                                i,
                                SQL_DESC_DISPLAY_SIZE,
                                NULL,
                                0,
                                NULL,
                                &displaySize);

      if (!SQL_SUCCEEDED(r) && !SQL_SUCCEEDED(r2))
         size = 16384;
      else if (!SQL_SUCCEEDED(r) && SQL_SUCCEEDED(r2))
         size = displaySize;
      else
         size = max(size,displaySize);

      if (size <= 0) size = 16384;    //constrain the buffer size...
      else if (size > MAXSIZE) size = MAXSIZE;

      b->FieldLength = size;
        b->sData.Resize(size);

        // Map this buffer to the driver's buffer.   At Fetch time,
        // the driver will fill in this data.  Note that the size is
        // count of bytes (for Unicode).  All ODBC functions that take
        // SQLPOINTER use count of bytes; all functions that take only
        // strings use count of characters.

        // Now set the display size that we will use to display
        // the data.   Figure out the length of the column name

      szTitle[0]=0;

      SQLColAttribute(hstmt,
                                i,
                                SQL_DESC_NAME,
                                szTitle,
                                sizeof(szTitle),    // Note count of bytes!
                                NULL,
                                NULL);

      b->FieldName = szTitle;

        SQLBindCol(hstmt,
                      i,
                 SQL_C_CHAR,
                 (SQLPOINTER) b->sData,
                 (b->FieldLength + 1) * sizeof(char),
                 &b->DataSize);
    }

}

//----------------------------------


#ifdef XP_WIN
ODBCDb::ODBCDb(ODBCEnv* Env,HWND parent,xdb& err)
#else
ODBCDb::ODBCDb(ODBCEnv* Env,void*parent,xdb& err)
#endif
 : SQL(Env), hdbc(SQL_NULL_HDBC),Connect(4096),DriverName(1024)
{
 int r = SQLAllocHandle(SQL_HANDLE_DBC,*SQL,&hdbc);
 Log(__LINE__,"alloc database",hdbc);

 if (!SQL_SUCCEEDED(r))
 {
  err=xdb("Can't allocate database");
  return;
 }

 Log(__LINE__,"SQLDriverConnect",hdbc);
 r = SQLDriverConnect(hdbc,
                      parent,
                             (unsigned char*)"",
                             SQL_NTS,
                             (unsigned char*)(char*)Connect,
                             4096,
                             NULL,
                             SQL_DRIVER_COMPLETE);

 if (!SQL_SUCCEEDED(r))
  {
   HandleError(SQL,hdbc,SQL_HANDLE_DBC,r,err);
   SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
   hdbc = SQL_NULL_HDBC;
   return;
  }

 Log(__LINE__,"SQLGetInfo",hdbc);
 SQLGetInfo(hdbc,SQL_DBMS_NAME, DriverName,1024,NULL);

 if (stristr(DriverName,"ACCESS"))
     escapeChar = 0; // doesn't have backslash-escaping
 else
     escapeChar = '\\';
}

ODBCDb::ODBCDb(ODBCEnv* Env,const char * connstr,xdb& err)
 : SQL(Env), hdbc(SQL_NULL_HDBC),Connect(4096),DriverName(1024),
  Login(connstr)
{

 int r = SQLAllocHandle(SQL_HANDLE_DBC,*SQL,&hdbc);
 Log(__LINE__,"alloc database",hdbc);

 if (!SQL_SUCCEEDED(r))
 {
   err=xdb("Can't allocate database");
   return;
 }

 Log(__LINE__,"SQLDriverConnect",hdbc);
 r = SQLDriverConnect(hdbc,
                      NULL, //GetDesktopWindow(), //GetFocus(),
                             (SQLCHAR*)(char*)connstr,
                             SQL_NTS,
                             (SQLCHAR*)(char*)Connect,
                             4096,
                             NULL,
                             SQL_DRIVER_COMPLETE);

 if (!SQL_SUCCEEDED(r))
 {
   HandleError(SQL,hdbc,SQL_HANDLE_DBC,r,err);
   SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
   hdbc = SQL_NULL_HDBC;
   return;
  }

 Log(__LINE__,"SQLGetInfo",hdbc);
 SQLGetInfo(hdbc,SQL_DBMS_NAME, DriverName,1024,NULL);

 if (stristr(DriverName,"ACCESS"))
     escapeChar = 0; // doesn't have backslash-escaping
 else
     escapeChar = '\\';
}

ODBCDb::~ODBCDb()
 {
  if (hdbc != SQL_NULL_HDBC)
  {
   Log(__LINE__,"disconnect",hdbc);
//   if (DriverName != "ACCESS")  SQLDisconnect(hdbc);      /* assume a connection was established */
   SQLDisconnect(hdbc);
   Log(__LINE__,"free database",hdbc);
   SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
  }
 }

SQLHSTMT ODBCDb::StartStatement(TStr& Error)
{
 SQLHSTMT st=0;
 SQLRETURN r;

 Log(__LINE__,"alloc statement",st);
 r = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &st);

 if (!SQL_SUCCEEDED(r))
  {
   Error="Can't allocate statement";
   return NULL;
  }
 return st;
}

ODBCStm* ODBCDb::FinishStatement(SQLHSTMT hstm,SQLRETURN r,TStr& Error)
{
 if (hstm == NULL) return NULL;

 ODBCStm * Stm = NULL;

 if (r == SQL_NO_DATA)
  {
   SQLFreeHandle(SQL_HANDLE_STMT, hstm);
   Log(__LINE__,"free statement",hstm);

   Error = 0;
   return NULL;
  }

 if (SQL_SUCCEEDED(r))
  {
   Stm = new ODBCStm(SQL,hstm);
   SQLSMALLINT sNumResults;
    r = SQLNumResultCols(hstm,&sNumResults);
   Stm->ColumnCount = sNumResults;
   Stm->RowCount = 0;
   long int rc = 0;
   SQLRowCount(hstm,&rc);
   Stm->RowCount = rc;

   if (rc < 0)
     Stm->RowCount = -1; //size wasn't reported, sorry.

   //we have to guess about the number of rows -- we don't
   //know how many there are until we actually retrieve them.
   // don't care about the return value here
  }

 if (r == SQL_SUCCESS)
  {
   Error = 0;
  }
 else
  {
   ListErrors(SQL,Error,hstm,SQL_HANDLE_STMT);
  }

 return Stm;
}

bool ODBCDb::SetAutoCommit(bool b)
{
 int r;
 r = SQLSetConnectAttr (hdbc, SQL_ATTR_AUTOCOMMIT, b ? (SQLPOINTER)SQL_AUTOCOMMIT_ON: (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);

 if (!SQL_SUCCEEDED(r))
  {
   ListErrors(SQL,LastError,hdbc,SQL_HANDLE_DBC);
   //HandleError(SQL,hdbc,SQL_HANDLE_DBC,r,err);
   SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
   hdbc = SQL_NULL_HDBC;
   return false;
  }
 return true;
}

bool ODBCDb::Commit()
{
    int r = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    if (!SQL_SUCCEEDED(r))
      ListErrors(SQL,LastError,hdbc,SQL_HANDLE_DBC);
    return (SQL_SUCCEEDED(r));
}

bool ODBCDb::Rollback()
{
    int r = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_ROLLBACK);
    if (!SQL_SUCCEEDED(r))
      ListErrors(SQL,LastError,hdbc,SQL_HANDLE_DBC);
    return (SQL_SUCCEEDED(r));
}

bool ODBCDb::ExecDirect(const char * cmd,TStr& Error,bool log)
{
 SQLHSTMT st;
 SQLRETURN r;

 bool ret, append = false;
 Error = 0;

 /* alloc statement */
 r = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &st);
 Log(__LINE__,"alloc statement",st);

 if (!SQL_SUCCEEDED(r))
  {
   Error += "Can't allocate statement";
   Log(__LINE__,"Can't alloc statement",st);
   return false;
  }

 /* execute the statement */
// Log(__LINE__,"exec command",st,cmd);

 r = SQLExecDirect(st, (unsigned char*)(char*)cmd, SQL_NTS);

 if (r != SQL_SUCCESS)
  {
//   if (log) ListErrors(SQL,Error,hdbc,SQL_HANDLE_DBC);
   ListErrors(SQL,Error,st,SQL_HANDLE_STMT);
   if (log) LogExec(cmd,Error);
   Error += cmd;
   append=true;
  }
 else if (log) LogExec(cmd,"OK");

 ret = SQL_SUCCEEDED(r);

 /* free the statement */
 r = SQLFreeHandle(SQL_HANDLE_STMT, st);
 Log(__LINE__,"free statement",st);

 if (!SQL_SUCCEEDED(r))
  {
   Error += "Can't free the statement";
  }
  //well, it worked, so don't ruin a perfectly good return value
  //with a bad statement handle

 if (*Error) Log(__LINE__,Error,st);

 return ret;
}

STMPointer ODBCDb::ListTables(DBPointer& self,TStr& Error)
{
 SQLHSTMT hstm = StartStatement(Error);
 if (!hstm) return STMPointer(0);
 Log(__LINE__,"list tables",hstm);
 ODBCStm* stm = FinishStatement(hstm,
                        SQLTables(hstm,NULL,0,NULL,0,NULL,0,NULL,0),
                        Error);

 if (stm) stm->db = self;
 else LogExec("List tables",Error);
 return STMPointer(stm);
// r = SQLExecDirect(hstm,(unsigned char*)stm,SQL_NTS);
}

STMPointer ODBCDb::ListColumns(DBPointer& self,const char* table,TStr& Error)
{
 SQLHSTMT hstm = StartStatement(Error);
 if (!hstm) return STMPointer(0);
 Log(__LINE__,"list columns",hstm);
 ODBCStm* stm = FinishStatement(hstm,
                        SQLColumns(hstm, (SQLCHAR*)"",0,(SQLCHAR*)"",0,(SQLCHAR*)table,strlen(table),NULL,0),
                        Error);

 if (stm) stm->db = self;
 else LogExec("List columns",Error);
 return STMPointer(stm);
// r = SQLExecDirect(hstm,(unsigned char*)stm,SQL_NTS);
}

STMPointer ODBCDb::OpenTable(DBPointer& self,const char * cstm,TStr& Error)
{
 SQLHSTMT hstm = StartStatement(Error);
  if (!hstm) return STMPointer(0);
// Log(__LINE__,"exec direct",hstm,cstm);
 ODBCStm* stm = FinishStatement(hstm,
                        SQLExecDirect(hstm,(unsigned char*)cstm,SQL_NTS),
                        Error);

 if (stm) stm->db = self;
 else LogExec(cstm,Error);
 return STMPointer(stm);
}

STMPointer ODBCDb::OpenTableWithParam(DBPointer& self,const char * cstm,TStringList& params,TStr& Error)
{
 SQLHSTMT hstm = StartStatement(Error);
  if (!hstm) return STMPointer(0);
// Log(__LINE__,"exec direct",hstm,cstm);

 int ret;
 for (size_t i=0; i<params.Count(); i++)
 {
     char* cparam = params[i];
     printf("%s\n",cparam);
     size_t len = strlen(cparam);
     ret = SQLBindParameter (hstm, i+1, SQL_PARAM_INPUT,
       SQL_C_CHAR, SQL_LONGVARBINARY, len, 0, (unsigned char*)cparam, len + 1, NULL);

     if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
       LogExec(cstm,Error);
 }

 ODBCStm* stm = FinishStatement(hstm,
                        SQLExecDirect(hstm,(unsigned char*)cstm,SQL_NTS),
                        Error);

 if (stm)
     stm->db = self;
 else
     LogExec(cstm,Error);

 return STMPointer(stm);
}

STMPointer ODBCDb::ListKeys(DBPointer& self,const char * tbl,TStr& Error)
{
 SQLHSTMT hstm = StartStatement(Error);
  if (!hstm) return STMPointer(0);

 Log(__LINE__,"list keys",hstm);
 ODBCStm* stm = FinishStatement(hstm,
                        SQLPrimaryKeys(hstm,(SQLCHAR*)"",0,(SQLCHAR*)"",0,(SQLCHAR*)(char*)tbl,strlen(tbl)),
                        Error);
 if (stm) stm->db = self;
 else LogExec("List keys",Error);
 return STMPointer(stm);
}

//-----------------------------------

ODBCEnv::ODBCEnv(int )
 {
  int32 r;

  Log(__LINE__,"alloc env",henv);
  r = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

  if (!SQL_SUCCEEDED(r))
  {
   henv = NULL;
   err=xdb("Can't start an ODBC environment");
   return;
  }

  IsVersion3 = true;

  r = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,SQLPOINTER(SQL_OV_ODBC3),0);

  if (!SQL_SUCCEEDED(r))
  {
   err=xdb("Sorry, ODBC 3 or later is required");
   return;
  }
 }

ODBCEnv::~ODBCEnv()
 {
  Connections.Clear();
  if (henv != SQL_NULL_HENV)
  {
   Log(__LINE__,"free env",henv);
   SQLFreeHandle(SQL_HANDLE_ENV, henv);
  }
 }

DBPointer ODBCEnv::OpenDatabase(TParameterList& opts,TStr& Error)
{
 MemoryStream connect;
 connect.WritePaired(opts,";");
 // "FILEDSN=C:\\win09\\mydb.dsn;UID=DBA;PWD=SQL";
 // Connect to the driver.  Use the connection string if supplied
 // on the input, otherwise let the driver manager prompt for input.
 return DBPointer(OpenDatabase(connect,Error));
 }

#ifdef XP_WIN
DBPointer ODBCEnv::OpenDatabase(HWND parent,TStr& Error)
#else
DBPointer ODBCEnv::OpenDatabase(void* parent,TStr& Error)
#endif
{
 if (!henv) return DBPointer(0);
 NotifyRelease(0); //free all freeable pointers

 ODBCDb* db;
 xdb err;

 try {
   db = new ODBCDb(this,parent,err);
   if (err) {delete db; db=0;}
  }
 catch(...)
  {
   db = NULL;
  }
 if (err) Error = TStr(err.why(),"\n",err.info());
 return DBPointer(db);
 }

void ODBCEnv::NotifyRelease(void* db)
{
 FOREACHBACK(DBPointer* sp,Connections)
  if (sp->IsMine() &&  (!db || (sp->TypecastPointer() != db)))
    Connections.Destroy(i);
 DONEFOREACH
}

DBPointer ODBCEnv::OpenDatabase(const char * connect,TStr& Error)
{
 if (!henv) return DBPointer(0);

 FOREACH(DBPointer* sp,Connections)
  if ((*sp)->Login != "")
   if ((*sp)->Login == connect) return DBPointer(*sp);
 DONEFOREACH
 NotifyRelease(0);

 ODBCDb* db;
 xdb err;
 try {
   db = new ODBCDb(this,connect,err);
  }
 catch (xdb& x)
  {
   Error = TStr(x.why(),"\n",x.info());
   db = NULL;
  }
 catch(...)
  {
   db = NULL;
  }

 if (err)
  {
   Error = TStr(err.why(),"\n",err.info());
   db = NULL;
  }

 DBPointer* x = new DBPointer(db,this);
 if (db) Connections.Add(x);
 return DBPointer(*x);
}

//----------------------------------------------------
#endif
