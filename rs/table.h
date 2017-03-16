#ifndef _RS_TABLE_H
#define _RS_TABLE_H

#ifndef _RS_STREAM_H
#include "rs/stream.h"
#endif

#ifndef _RS_LIST_H
#include "rs/list.h"
#endif

#ifndef _RS_DBF_H
#include "rs/dbf.h"
#endif

#include "rs/ezf.h" // we need this as a basic data-definition language
// #define TBL_NO_SQL
// #define TBL_NO_DBF
#define TBL_NO_NOTES
// #define TBL_NO_PARAM
// #define TBL_NO_ASCII
// #define TBL_NO_SORT

class TBLEnv;

class TableRow;
class DataTable;
class SpreadsheetTable;

enum ETableType
{
  ETableUnknown, //= edbt_Unknown,
  ETableDBF,// = edbt_dBase,
  ETableASCII,// = edbt_ASCII,
  ETableSQL,// = edbt_SQL,
  ETableNotes,// = edbt_Notes,
  ETableParamList,// = edbt_ParamList,
  ETableSort // = edbt_Sort,
};

class TBLEnv;

DataTable* OpenTable(const char * filename,
                     TBLEnv* env,
                     bool Ignored_True=true,
                     bool ReadOnlySmallMemory=false);
// You only need to initialize a table environment if you'll be using
// Notes or ODBC databases. Otherwise, you may ignore the 'env' parameter.

class TBLEnv   // create one of these for your application if you indend
{public:       // to use SQL or Notes databases
 TStr LastError;
#ifndef TBL_NO_SQL
 void * odbcenv;
#endif
#ifndef TBL_NO_SQLITE
 void * sqlite3env;
#endif
#ifndef TBL_NO_NOTES
 NotesLib* Notes;
#endif
 TBLEnv();
 ~TBLEnv();
 DataTable* OpenTable(const char * filename)
  {return ::OpenTable(filename,this);}
 void Reset();
};

bool TableMatchesForm(DataTable* Table,EZFForm& Form);

bool UpdateTable(const char * URL,
                 TEZSForm& Form,
                 TStr& Error,
                 TBLEnv* env=NULL,
                 bool ForceRebuild = false);
//unlike CreateTable, UpdateTable will not alter the URL.
//may throw exceptions on error

bool GenerateForm(DataTable* t,EZFForm& Form,bool GuessResponses=true,
                  RSCallback Callback=NULL,void*v=NULL);

void CreateTableSQL(const char * DriverName,const char* table,
                    EZFForm& Form,TStringList &cmds,TStr& error);

bool CreateTable(TStr& URL,
                 TEZSForm& Form,
                 TStr& Error,
                 ETableType Type=ETableUnknown,
                 TBLEnv* env=NULL);
//If Type is "unknown", the URL service is used to decide the table type.
//If no type is found, DBF is used.
//For ODBC databases, a complete URL is required, with id and pwd.
//Notes databses are not presently available.
//The URL parameter is modified to give a complete, fully-qualified
//path name for the new database.
//may throw exceptions on error

count_t TableAddRow(DataTable& dest,TParameterList& data,const char * sqldefault);
// sqldefault comes from the KEY.DEFAULT field of a form

bool CopyHeaders(DataTable& D,DataTable& S);

bool TransferData(DataTable& dest, DataTable& src,const char * keydefault=NULL,RSCallback Callback=NULL,void*v=NULL);
bool TransferData(const char * dest, const char * src,TBLEnv* env,const char * keydefault=NULL,RSCallback Callback=NULL,void*v=NULL);
//this doesn't work: while (source.GetRow(++i,data)) dest.AddRow(data);
//use this instead:  for (count_t CaseNum = 1; Database.GetDataC(CaseNum,0); CaseNum++)
//keydefault: what to use for the default key value if not otherwise
//specified

double RsStrtod(const char * strP,char** endptr=0,size_t* len=0,bool* isnum=0);
//convert text (with dollar signs, etc) into numbers

struct TableLocation
{
 count_t I;
 size_t  J;
 TableLocation() {I=0;J=0;};
 TableLocation(count_t a, size_t  b) : I(a),J(b) {};
 int Compare(TableLocation& o)  //for sorting
  {int i; return (i = I - o.I, i) ? i : J - o.J; }
   // -  a < b
   // 0  a = b
   // +  a > b
 operator bool() {return (I && J);}
 bool operator == (TableLocation& o) {return I==o.I && J == o.J;}
};

class TableDataCache
{public:
  TList<SpreadsheetTable> Items;
  size_t lastSearch;
  size_t columnCount;
  size_t pageSize;
  size_t maxPages;

 TableDataCache(size_t Columns);
 ~TableDataCache();
 void Set(count_t x,size_t y, const char * data);
 const char * Get(count_t x,size_t y);
 void Flush();
};

class DataTable: public TDataList
{ // * indicates those functions you need to override
 public:
 bool IsModified, IsChanged;
 //IsModified: data changed
 //IsChanged: needs to be redrawn

 enum ETableStyle { HasSave     = 0x01, AllowModify = 0x02,
                   AllowAddRow = 0x04, AllowAddCol = 0x08,
                   AllowDelRow = 0x10, AllowDelCol = 0x20}
      Style;

// enum ERowStatus { Normal = 0, Special = 0x01, Hidden = 0x02 };
 // see formtype.h for field data types

 TStr Filename;
 TChars TempBuffer;
 ETableType TableType;

 DataTable(); // *
 virtual ~DataTable();

 virtual const char * filename() {return Filename;}
 virtual void GetConnectString(TStr& x) = 0;
 //URL needed to open this database

 virtual const char * GetLastError();

 ///should return NULL if no data exists
 ///GetDataC(0,0)   tells the table that we will want to read data
 ///                for demand-load databases.
 ///GetDataC(row,1) returns a non-null value if the row exists.
 ///                (you could return the title, but that's not necessary)
 ///GetDataC(row,1) returns NULL if the row is beyond the end of the table.
 ///                That's how you can iterate a table without calling GetSize().
 virtual const char * GetDataC(count_t i, size_t  j); // *

// virtual bool SetDataC(count_t i, size_t  j, const char * c); // *

 virtual double GetDataD(count_t i, size_t  j); // i>0, j>0
 virtual bool SetDataD(count_t i, size_t  j, double d); // i>0, j>0

// virtual count_t RowCount();
// virtual size_t ColumnCount();

// virtual ERowStatus GetRowStatus(count_t i);
 virtual EDBFieldType GetColStatus(size_t i);

 virtual int GetColWidth(size_t i); //number of chars, default is INT_MAX

// virtual const char * GetTitle(int dimension,count_t index);  // *

 virtual const char * ColumnTitle(size_t index);
 virtual bool SetTitle(size_t index,const char * c);  // *

 virtual size_t AddCol(const char * title);

 virtual count_t AddRow(TNameValueList& data);
 count_t AddRow(int count,...);//add text for new rows.

 virtual bool DelCol(size_t j);

 // virtual bool DelRow(count_t i);
 // SQL: values for key fields beginning with a backslash will be used
 // exactly and not quoted in creating the SQL command.

 virtual bool Save();

 count_t Count() {return RowCount();}

 ///re-indexes the rows and columns at 0
 TableRow operator [] (count_t row);

 ///delta = -1 and start = 0 to search backwards from the end
 ///returns 0 for not found
 virtual count_t FindRow(TNameValueList& pl,count_t start=0,int delta = 1);

 ///usage: while (Table.GetRow(++i,pl)) {DoStuff(); SetRow(i,pl);}
 ///if 'pl' is initially blank, the fields should come up in order
 virtual bool GetRow(count_t i,TNameValueList& pl);

 virtual bool SetRow(count_t i,TNameValueList& pl);

 ///If you know you're not going to need any data that you've already read,
 ///allow the table to empty its cache.
 virtual void FreeMemory();
};

/*! DataIndex is optimized for searching a fixed set of fields.
    It uses newline-delimited keys of a limited set of records.
*/
class DataIndex
{public:
 bool CaseSensitive;
 TWORMList Index;
 TWORMList Overflow;

 DataIndex(TDataList& Source,TStringList& ColumnNames);
 DataIndex(TDataList& Source,size_t column);
 DataIndex(size_t base=0);
 DataIndex(TNameValueList& Source);
 ~DataIndex() {}

 bool BuildIndex(RSCallback Callback=NULL,void*v=NULL);

 void AddRecord(TNameValueList& Rec);
 void AddKey(const char * key);

 bool FindPosition(const char* key,size_t &pos);
 size_t FindRecord(TNameValueList& Q);

protected:
 size_t base;
 TDataList* Data;
 MemoryStream temp;
 TStr MainColumn;

 TIndexList Columns;
 TIndexList Order;
 TIndexList Rows;
 void BuildKey(Stream& key, TNameValueList& Q);
};

// Those were all the generic functions. Now, we see specific implementations
// of tables. These go in separate header files: tbl_*.h

#ifndef TBL_NO_SORT
#include "rs/tbl_sort.h"
#endif

#ifndef TBL_NO_ASCII
#include "rs/tbl_ascii.h"
#endif

#ifndef TBL_NO_PARAM
#include "rs/tbl_param.h"
#endif

#if 0
replace TableDataCache with the more efficient SpreadsheetTable
class TableDataCache //helper class for ODBC, etc. Generic, simple cache
{public:
 count_t sequence;
 size_t size; // cache grows between size and 2*size
 size_t lastsearch;
 size_t lastsort;
 size_t Oldest;

 struct TData
   {
     count_t x,y,age; //age 0 means "delete me"
     TStr Data;
     TData(count_t _x, count_t _y, count_t _age, const char * s):
           x(_x),y(_y), age(_age), Data(s) {}
     int Compare(TData & o) { return age-o.age; }
   };

 TRow<TData> Items;
 TableDataCache(size_t Size); //0 or -1 for quasi-infinite size
 ~TableDataCache();

 const char * Set(count_t x,size_t y, const char * data);
 void Unset(count_t x); //de-cache a row

 const char * Get(count_t x,size_t y);

 TData* Find(count_t x,size_t y);
 void Flush() {Items.Flush();sequence=0;LastSearch=0;Oldest=0;}
};
#endif

#endif

