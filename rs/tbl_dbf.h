#ifndef _TBL_DBF_H
#define _TBL_DBF_H
#ifndef _RS_TABLE_H
#include "rs/table.h"
#endif

class DatabaseTable : public DataTable
{
 public:
 TPointer<DBFFile> Database;
 TPointer<DBFRecord> Record;
 count_t RecordCount;
 uint32 timer;

 protected:
 bool LoadRecord(count_t casenum);

 public:

 const char * filename() {return Filename;}

 DatabaseTable(const char * database,bool SmallCache);
 DatabaseTable(DBFFile* database);
 virtual ~DatabaseTable();

 void GetConnectString(TStr& x);

 const char * GetDataC(count_t i, size_t j);
 bool SetDataC(count_t i, size_t j, const char * c);

 count_t RowCount();
 size_t ColumnCount();

 const char * ColumnTitle(size_t index);
 bool SetTitle(size_t index,const char * c);

 size_t FindColumn(const char * title);

 bool GetRow(count_t row,TNameValueList& pl);
 bool SetRow(count_t row,TNameValueList& pl);
 count_t AddRow(TNameValueList& data);

// bool DelRow(count_t i);
// TRowStatus GetRowStatus(count_t i);
 EDBFieldType GetColStatus(size_t i);
 int GetColWidth(size_t i);
};
#endif
