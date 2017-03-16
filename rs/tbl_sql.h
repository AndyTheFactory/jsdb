#ifndef _TBL_SQL_H
#define _TBL_SQL_H

#ifndef TBL_NO_SQL

#ifndef _RS_TABLE_H
#include "rs/table.h"
#endif

#ifndef _RS_SQL_H
#include "rs/sql.h"
#endif

#define GETQUOTECHAR(driver) \
 quotea = quoteb = "\""; \
 if (stristr(driver,"MYSQL")) quotea = quoteb = "`"; \
 else if (stristr(driver,"SYBASE") || stristr(driver,"ADAPTIVE SERVER")) \
          {quotea = "["; quoteb = "]";}

// || (stristr(driver,"SQL Server") && !stristr(driver,"MICROSOFT")))

class ODBCTable : public DataTable
{ // * indicates those functions you need to override
 protected:
   DBPointer Database;
   STMPointer Table;

 public:
   ODBCResultInfo Results;
   TStr originalquery;

 protected:
   TStr LastError;
   TStr name;           // table name

   TStringList keys;    // key fields in the form of "1" or "1,3,4" or etc
//   TIntList keyi;

   MemoryStream query; // extra selection criteria
   TStr select;
   TIntList deli;       // list of deleted records
   TStr keydefault;     // if we're not given a key name, guess one, depending
                        // on the database service

   TStringList map;     //list of key values

   count_t iRowCount, jColumnCount;
   bool Fetch(count_t i);

 public:
   bool GetWhere(Stream& out,int32 row);
   // query statement to select a specific record
   // field1='b' and field2='b'

 protected:
   TableDataCache Cache;

   char escapeChar;

   void QuoteSQL(Stream& out,const char * in);
   void EscapeSQL(Stream& out,const char * in);

   bool LoadTable();
   void SetupTable();
   void FreeTable();

   void FreeMemory();

   bool doDelRow(count_t i,Stream& out);

  public:

   ODBCColumn* GetField(const char * name);
   ODBCColumn* GetField(count_t j);

   bool Save();
   void BeginTrx();
   void EndTrx(bool commit = true);
   const char * GetLastError() {return LastError;}
   const char * TableName() {return name;}
   const char * DriverName() {return Database->DriverName;}

/*   ODBCTable(ODBCStm* stm,
             const char * Query=0);
*/

   ODBCTable(DBPointer &db,
             const char * Table,
             const char * Query=0,
             const char * Statement=0);

/*   ODBCTable(ODBCEnv& Env,
             const char * DSN,    // db name
             const char * Userid,
             const char * Password,
             const char * Table,
             const char * Query=0);

   ODBCTable(ODBCEnv& Env,
             const char * connect,
             const char * Table,
             const char * Query=0);
*/
  virtual ~ODBCTable();
   // unless query is specified, these should open the entire table
   // query = field1=value1&field2=value2&...

   void GetConnectString(TStr& x);

   const char * GetDataC(count_t i, size_t j);

   const char * ColumnTitle(size_t index);
   bool SetDataC(count_t i, size_t j, const char * c);

   count_t RowCount();
   size_t ColumnCount();

//   TRowStatus GetRowStatus(count_t i);
   EDBFieldType GetColStatus(size_t i);

   int GetColWidth(size_t i); //number of chars, default is INT_MAX

   bool SetRow(count_t i,TNameValueList& pl);
   count_t AddRow(TNameValueList& data);
   count_t SQLAddRow(TNameValueList& data,const char * defkey);
   bool GetRow(count_t i,TNameValueList& pl);
   bool SQLDelRow(count_t i,bool del);

   bool ExecDirect(const char * cmd,TStr& Error)
    {
      FreeTable(); //start all over again!
      return Database->ExecDirect(cmd,Error);
    }

   bool TableOpen()
    {return Table != 0; }
//   bool DelRow(count_t i);
};

#endif
#endif
