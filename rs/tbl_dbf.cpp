#include "rslib.h"
#pragma hdrstop

#ifndef TBL_NO_DBF

#include "rs/tbl_dbf.h"

#define SETFLAG(i,flag) (i) = (ETableStyle) ((int)i | (int)(flag))
#define CLEARFLAG(i,flag) (i) = (ETableStyle) ((int)i & ~(int)(flag))
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

DatabaseTable::DatabaseTable(const char * db,bool SmallCache)
 : DataTable()
 {
    TableType = ETableDBF;

  Filename = db;
  Database = OpenDBF(db,db,false,
                           SmallCache ? CACHE_BIG : CACHE_HUGE); // 1M data cache
  if (!Database) throw xdb("Database not found");
  Record = 0;
//  PromptOnSave = false; //annoying
  if (Database->AppendAllowed()) SETFLAG(Style,AllowAddRow);
  if (Database->ReplaceAllowed()) SETFLAG(Style,AllowModify|AllowDelRow);
  timer = 0;
 }

DatabaseTable::DatabaseTable(DBFFile* db)
 : DataTable()
 {
 TableType = ETableDBF;

  Database = db;
  if (!Database) throw xdb("Database not found");
  Filename = db->filename();

  Record = 0;
//  PromptOnSave = false; //annoying
  if (Database->AppendAllowed()) SETFLAG(Style,AllowAddRow);
  if (Database->ReplaceAllowed()) SETFLAG(Style,AllowModify|AllowDelRow);
  timer = 0;
 }

size_t DatabaseTable::FindColumn(const char * title)
{
 return Database->GetFieldNum(title);
}

DatabaseTable::~DatabaseTable()
 {
  if (Record && Record->IsModified && Record->AllowModify)
       {
          Record->CommitChanges();
       }
 // if (Record) delete Record;
  Record = 0;
 // delete Database;
 }

void DatabaseTable::GetConnectString(TStr& x)
{
 x = TStr("dbf://",Filename);
}

count_t DatabaseTable::AddRow(TNameValueList& data)
{
 count_t ret=0;
 if (Style & AllowAddRow)
  {
    DBFRecord Rec(Database,0);
    Rec = data;
    Rec.CommitChanges();
    ret=Rec.CaseNum;
    IsChanged = true;
    RecordCount ++;
  }
 return ret;
}

EDBFieldType DatabaseTable::GetColStatus(size_t i)
{
  DBFFieldRec* R = Database->GetFieldInfo(i);
  if (!R) return db_ft_NoData;
  return R->FieldType;
}

bool DatabaseTable::GetRow(count_t row,TNameValueList& pl)
{
 if (row == 0) return false;

 if (!LoadRecord(row)) return false;

 count_t jmax = Record->FieldCount();

 for (count_t j = 1; j <= jmax;j++)
  {
   size_t fl = Record->FieldLength(j);
   if (!fl) return false;
   if (fl >  INT_MAX) return false;

   if (fl > TempBuffer.size) TempBuffer.Resize(fl);
   if (!Record->GetFieldData(j,TempBuffer,fl)) return false;
   pl.Set(ColumnTitle(j),StripCharsFB(TempBuffer," ",fl));
  }
 return true;
}

bool DatabaseTable::SetRow(count_t row,TNameValueList& pl)
{
 if (row==0)
  {
   return AddRow(pl) != 0;
  }

 if (!LoadRecord(row)) return false;

 if (!Record->AllowModify) return false;

 count_t jmax = Record->FieldCount();

 for (count_t j = 1; j <= jmax; j++)
    {
     const char * c = Record->FieldName(j);
     if (pl.Has(c)) Record->SetFieldData(j,pl(c));
    }

 IsChanged = true;
 Record->CommitChanges();
 return true;
}

bool DatabaseTable::LoadRecord(count_t casenum)
 {
  if (Record)
  {
   if (Record->CaseNum == casenum) return true;
   //else save changes and load a new one
   if (Record->IsModified && Record->AllowModify)
          Record->CommitChanges();
//   delete Record;
   Record = NULL;
 }

#ifdef __DEMO__
 if (casenum > 10) return false;
#endif

  if (casenum > RowCount()) return false;
  try
  {
    if (casenum && !Record) Record = new TDBRecord(Database,casenum);
  } catch(...) {}
  return (Record != NULL);
 }

const char * DatabaseTable::GetDataC(count_t i, size_t j)
{
 if (!i && !j) return NULL;

 if (!i) return ColumnTitle(j);

 if (!j)
  {
     if (i > RowCount()) return NULL;

     switch(Database->GetDeleteFlag(i))
     {
      case db_Deleted: return "D";
      case db_Locked: return "L";
      case db_OK: return "";
      default: return "E"; // error value
     }
  }

 bool loaded = false;

 if (Record)
   if (Record->CaseNum == i) loaded = true;

 if (!loaded)
  {
   bool a = false;

   if (!Database->HasPassword)
    if (Database->GetDataC(i, j,TempBuffer))
    {
     a = true;
     if (Record) LoadRecord(0);
    }

   if (!a)
    {
     if (!LoadRecord(i)) return NULL;
    }
  }

 if (Record)
  {

   size_t fl = Record->FieldLength(j);

   if (!fl) return NULL;
   if (fl > INT_MAX) return NULL;

   if (fl > TempBuffer.size) TempBuffer.Resize(fl);

   if (!Record->GetFieldData(j,TempBuffer,fl)) return NULL;

   return StripCharsFB(TempBuffer," ",fl);
  }

 return StripCharsFB(TempBuffer," ");
}

bool DatabaseTable::SetDataC(count_t i, size_t j, const char * c)
{
 if (i == 0) return false;

 if (j == 0)
   {
    IsChanged = true;
    if (!c) return false;
    if (*c == 'D' && (Style & AllowDelRow))
                        Database->SetDeleteFlag(i,db_Deleted);
    else if (*c == 'L') Database->SetDeleteFlag(i,db_Locked);
    else if (*c == 0 || *c == ' ') Database->SetDeleteFlag(i,db_OK);
    else return false;
    if (Record) LoadRecord(0);
    return true;
   }

 LoadRecord(i);
 if (!Record) return false;

 bool ret = Record->SetFieldData(j,c);
 if (ret) IsChanged = true;
 return ret;
}

count_t DatabaseTable::RowCount()
{
 uint32 t = GetTickCount();
 if (t - timer > 2048)
    RecordCount = Database->RecordCount();
 timer = t;
 return RecordCount;
}

size_t DatabaseTable::ColumnCount()
{
 return Database->FieldCount();
}

int DatabaseTable::GetColWidth(size_t i)
{
   DBFFieldRec* R = Database->GetFieldInfo(i);
   return R ? R->FieldLength : 0;
}

const char * DatabaseTable::ColumnTitle(size_t index)
{
 return Database->GetFieldName(index);
}

bool DatabaseTable::SetTitle(size_t,const char *)
{
 return false;
}
#endif
