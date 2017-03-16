#ifndef _RS_SQL_H
#define _RS_SQL_H
// ODBC doesn't like dynamic binding of functions.

#ifndef TBL_NO_SQL

#ifdef XP_WIN
#include <odbc/sqltypes.h>
#include <odbc/sql.h>
#include <odbc/sqlext.h>
#define SQL_NOUNICODEMAP
#include <odbc/sqlucode.h>
#elif defined(XP_UNIX)
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#define SQL_NOUNICODEMAP
#include <sqlucode.h>
#ifndef __SQLTYPES
#define __SQLTYPES
#endif
#else
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#define SQL_NOUNICODEMAP
#include <sqlucode.h>
#ifndef __SQLTYPES
#define __SQLTYPES
#endif
#endif

#ifdef __SQLTYPES
/* my own version of the data access objects */
/* exceptions should be handled fairly well internally */

class ODBCStm;
class ODBCDb;
typedef TEnvelope<ODBCDb> DBPointer;
typedef TEnvelope<ODBCStm> STMPointer;
class ODBCEnv;
class ODBCTable;

void EscapeSQL(Stream& out, const char * in, int escape);

/* my own version of the data access objects */
/* exceptions should be handled fairly well internally */

class ODBCColumn
{
 public:
   TStr     FieldName;
   int32    FieldLength;
   TStr     sData;        ///< data buffer
   long     DataSize;    ///< size or null
   int      Type;
   bool     NeedsQuotes; ///< chars need quotes
   bool     IsNumber;    ///< numbers still converted to strings
 //  bool        Key;
 //  bool        IsBlob; ///< do I need to access data the hard way?
   bool     AutoIncrement;

 ODBCColumn();
 ~ODBCColumn();
};

class ODBCResultInfo: public TNameValueList
{public:
 TRow<ODBCColumn> Data;
// size_t Key;

 size_t FieldNumber(const char * c);

 /// Clear out data, in case the DB doesn't fill in null values
 void Clear();

 size_t Count() {return Data.Count();}

 bool Has(const char * name)
  {return FieldNumber(name) != NOT_FOUND;}

 const char * Name(size_t i)
  {return Data.Has(i) ? (char*)Data[i]->FieldName : (char*)NULL; }

 const char * Value(size_t i)
  {return Data.Has(i) ? (char*)Data[i]->sData : (char*)NULL;}

 const char * Get(size_t i) //one-based, no bounds checking
  {return Value(i-1);}

 const char * operator [] (size_t i) //zero-based!
  {return Value(i);}

 const char * operator () (const char * c)
  {return Value(FieldNumber(c));}

 bool Set(const char* name, const char* value)
  {return false;}
};

class ODBCStm
{protected:
 SQLHSTMT hstmt;
 ODBCEnv* SQL;
 DBPointer db;
 ODBCResultInfo* Results;

 public:
 size_t ColumnCount;
 int32 RowCount,CurrentRow;  //row 0 indicates no data
 TStr LastError;

 ODBCStm(ODBCEnv* Env,SQLHSTMT s);
 ~ODBCStm();

 operator SQLHSTMT() {return hstmt;}

 bool Fetch(int32 i=0);
 bool Rewind(); //after you reach the end
 void Bind(ODBCResultInfo& i);

 const char * GetLastError()
  {return LastError;}
 FRIEND ODBCDb;
};

class ODBCDb
{protected:
 SQLHDBC hdbc;
 ODBCEnv* SQL;

 public:
 TStr Login, ///dsn, uid, pwd
      Connect, ///full connection string
      DriverName; ///driver information

 TStr LastError;
 int escapeChar;

#ifdef XP_WIN
 ODBCDb(ODBCEnv* Env,HWND parent,xdb& err); // for browsing
#else
 ODBCDb(ODBCEnv* Env,void* parent,xdb& err); // for browsing
#endif
 ODBCDb(ODBCEnv* Env,const char * connect,xdb& err);
 ~ODBCDb();
 operator SQLHDBC() {return hdbc;}

 STMPointer OpenTable(DBPointer& self,const char * stm,TStr& Error);
 // Statements don't always have return rows.
 // If you aren't selecting rows, use this other form
 STMPointer OpenTableWithParam(DBPointer& self,const char * stm,TStringList& cparam,TStr& Error);

 STMPointer ListTables(DBPointer& self,TStr& Error);
 STMPointer ListColumns(DBPointer& self,const char* table,TStr& Error);
 STMPointer ListKeys(DBPointer& self,const char * table,TStr& Error);

 bool ExecDirect(const char * stm,TStr& Error,bool log = true);

 bool SetAutoCommit(bool b);
 bool Commit();
 bool Rollback();

 protected:
 SQLHSTMT StartStatement(TStr& Error);
 ODBCStm* FinishStatement(SQLHSTMT hstmt,SQLRETURN r,TStr& Error);
 FRIEND ODBCTable;
};

class ODBCEnv : public TNotifyRelease //: public TODBCInstance
{protected:
 SQLHENV henv;
 TList<DBPointer> Connections;

 public:
 xdb err;
 bool IsVersion3;

 bool IsOpen() {return !err;}

 void NotifyRelease(void* db);

 ODBCEnv(int version=ODBCVER);
 ~ODBCEnv();
 operator SQLHENV() {return henv;}

 DBPointer OpenDatabase(const char * connect,TStr& Error);
#ifdef XP_WIN
 DBPointer OpenDatabase(HWND parent,TStr& Error);
#else
 DBPointer OpenDatabase(void*parent,TStr& Error);
#endif
 DBPointer OpenDatabase(TParameterList& opts,TStr& Error);
};

#endif
#endif
#endif
