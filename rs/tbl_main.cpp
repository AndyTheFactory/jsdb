#include "rslib.h"
#pragma hdrstop

#ifndef TBL_NO_SQL
#include "rs/tbl_sql.h"
#endif

#ifndef TBL_NO_DBF
#include "rs/tbl_dbf.h"
#endif

#ifndef TBL_NO_NOTES
#include "rs/tbl_notes.h"
#endif

#include "rs/file.h"
/* we read two types of URLs for databases:
  General
    URL       = service://user@password:database/table?query

  ODBC
    URL       = odbc://user@password:database/table?query
    service   = "odbc"

  Notes
    URL       = notes://user@password:server/table?query
    service   = "notes"
    database  = "server"

  Files
    tablename = file://user@password:filename
    service   = "file"
    table     = "filename..."  any type of slash is OK
    database  = "DBF"
*/

//needs work.

TBLEnv::TBLEnv()
{
#ifndef TBL_NO_SQL
 odbcenv = NULL;
 try {
 odbcenv = new ODBCEnv;
 } catch(...) {odbcenv = NULL;}
#endif
}

void TBLEnv::Reset()
{
#ifndef TBL_NO_SQL
 if (odbcenv != NULL)
 {
  ((ODBCEnv*)odbcenv)->NotifyRelease(0);
 }
#endif
}

TBLEnv::~TBLEnv()
{
#ifndef TBL_NO_SQL
 if (odbcenv != NULL)
 {
  delete (ODBCEnv*)odbcenv;
  odbcenv = NULL;
 }
#endif
}

DataTable* OpenTable(const char * URL,
                     TBLEnv* tenv,
                     bool FirstRowAsTitles,
                     bool SmallMemory)
{
 TStr service,user,database,table,password,query;

 URLSplit(URL,service,user,password,database,table,query);

 if (service == "odbc" || service == "sql")
 {
#ifndef TBL_NO_SQL
  if (!tenv) return NULL;
  ODBCEnv * env = (ODBCEnv*)(tenv->odbcenv);
  if (!env) return NULL;
  try {
    TStr error;
    TStr Connect("DSN=",database,";UID=",user,";PWD=",password);

    DBPointer dbp(env->OpenDatabase(Connect,error));
    if (!dbp) return NULL;

    ODBCTable * f = new ODBCTable(dbp,table,query);

   return f;
  }
   catch (...)
    {
    } //(...) {return 0;}
#endif
  return NULL;
 }
 else if (database == "dsn")
 {
#ifndef TBL_NO_SQL
  if (!tenv) return NULL;
  ODBCEnv * env = (ODBCEnv*)(tenv->odbcenv);
  if (!env) return NULL;
  try {
   Replace(table,"/",'\\');
   //which table to open?
   TStr tblname(GetFilename(table));
   ClipExtension(tblname);
   TStr error;

   // customer.dsn opens the table 'customer'

   DBPointer dbp (env->OpenDatabase(TStr("FILEDSN=",table),error));
   if (!dbp) return NULL;
   TEnvelope<ODBCDb> db (dbp);

   ODBCTable * f = new ODBCTable(db,tblname,query);
   return f;
  }
   catch (...)
    {
    } //(...) {return 0;}
#endif
     return NULL;
 }
#ifndef TBL_NO_NOTES
 else if (service == "notes")
 {
  open a Lotus Notes database here
 }
#endif
 else if (service == "file")
 {
  FixFilename(table);//  Replace(table,"/",'\\');
  char * ext = GetExtension(table);
  if (!FileExists(table)) return 0;
#ifndef TBL_NO_DBF
  try{
      if (strcasecmp(ext,"DAT")==0 || strcasecmp(ext,"DBF")==0
          || strcasecmp(ext,"INI")==0 || strcasecmp(ext,"DATABASE")==0)
       {
        DatabaseTable* Grid = new DatabaseTable(table,SmallMemory);
        if (*password) Grid->Database->SetPasswords(password);
        if (Grid) return Grid;
       }
     }
     catch(...) {}
#endif
#ifndef TBL_NO_ASCII
 if (strcasecmp(ext,"dbf")) /* don't open DBF files this way */
  try {
  SpreadsheetTable* Grid;

  if (SmallMemory)
   {
    Stream* m = new FileStream(table,Stream::OMText,Stream::ReadOnly);
    Grid = new SpreadsheetTable(m,true);
   }
  else Grid = new SpreadsheetTable(table,FirstRowAsTitles);
  return Grid;
     }
     catch(...) {}
#endif
 }
return NULL;
}

