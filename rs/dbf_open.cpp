#include "rslib.h"
#pragma hdrstop

DBFFile * PreviewDB(DBFHeader * Header)
{
  Header->CreateFile(0);
  DBFPreview * pre = new DBFPreview(Header);
  DBFFile * dat = new DBFFile(pre,0);
  return dat;
};

DBFFile * OpenDBF(const char * datafile,
                   DBFHeader* header,
             //      EDBType Type,
                   bool IgnoreReadOnly,
                   EDBCacheSize CacheSize)
{
 DBFBaseIO* dbf;

 try
  {
   dbf=new DBFFixedFormatFile(header,datafile);
  }
  catch(...)
   {
    delete header;
    return NULL;
   }

  if ((dbf->RecordLength()<=0)||(!dbf->IsValid()))
   {
    delete dbf;
    return NULL;
   }

 if (IgnoreReadOnly) dbf->FileStatus |= wr_Write;

 DBFCache * cache=0;
 DBFFile * db;

 if (dbf->IsCacheable&&CacheSize)
    cache = new DBFCache();

 try {
  db=new DBFFile(dbf,cache);
  }
 catch(...) {delete dbf; if (cache) delete cache; return NULL; }

 if (cache)
  {
   try {
    cache->SetBufferSize(db,CacheSize);
    } catch(...) {delete db; return NULL; }
  }

 return db;


}

DBFFile * OpenDBF(const char * datafile,
                   TParameterList& header,
                   int Type,
                   bool IgnoreReadOnly,
                   EDBCacheSize CacheSize)
{
 if (Type == 0) Type = DBFHeader::ASCIIDOS;
 DBFHeader* dbh = new DBFHeader((DBFHeader::TType)Type);
 FOREACH(TNameValuePair* n, header)
  dbh->AddField(n->Name,atoi(n->Value));
 DONEFOREACH
 dbh->CalculateSizes();
 dbh->HeaderSize = 0;

 //DBFFixedFormatFile * file = new DBFFixedFormatFile(dbh,datafile);

 return OpenDBF(datafile,dbh,IgnoreReadOnly,CacheSize);

}

DBFFile * OpenDBF(const char * datafile,
                   const char * headerfile,
//                   EDBType Type,
                   bool IgnoreReadOnly,
                   EDBCacheSize CacheSize)
{
 if (!headerfile) headerfile = "";

 const char * ext = GetExtension((char*)headerfile);
 if (!strcasecmp(ext,"ini"))
  {
   TParameterList Header;
#ifdef XP_WIN
     DBFHeader::TType Type = DBFHeader::ASCIIDOS;
#else
     DBFHeader::TType Type = DBFHeader::ASCIIUNIX;
#endif
     Header.ReadINIFileSection(headerfile,"file");
     const char * t = Header["type"];
     if (!strcasecmp(t,"DOS")) Type = DBFHeader::ASCIIDOS;
     else if (!strcasecmp(t,"MAC")) Type = DBFHeader::ASCIIMAC;
     else if (!strcasecmp(t,"UNIX")) Type = DBFHeader::ASCIIUNIX;
     else if (!strcasecmp(t,"DBF")) Type = DBFHeader::dBase;
     else if (!strcasecmp(t,"DATABASE")) Type = DBFHeader::WEB;
     else if (!strcasecmp(t,"WEB")) Type = DBFHeader::WEB;
     TStr newfile(Header["data"]);
     if (*newfile) datafile = newfile;
     printf("%s\n",datafile);
     Header.Clear();
     Header.ReadINIFileSection(headerfile,"fields");
     return OpenDBF(datafile,Header,Type,IgnoreReadOnly,CacheSize);
 }

// if (Type == edbt_Unknown) Type = edbt_dBase;
 if (!*headerfile) headerfile = datafile;

 DBFHeader* dbh;
  try {
   dbh=new DBFHeader(DBFHeader::dBase);
   if (!dbh->LoadFromFile(headerfile)) {delete dbh; return NULL;}
   if (dbh->FieldCount()==0) {delete dbh; return NULL;} //must have fields
   }
   catch(...) {delete dbh; return NULL; }

 return OpenDBF(datafile,dbh,IgnoreReadOnly,CacheSize);
}

#ifdef _DONT_COMPILE_THIS
/*
struct OpenDb
 {  TStr inifn, datfn, hdrfn, descr;
    EDBType type;
 };

bool ReadDatabaseIni(OpenDb &o)
{
 char temp[MAXPATH];
 if (!o.inifn) return false;
 if (o.type==edbt_Unknown)
  {
   GetPrivateProfileString("Database","Type","",temp,MAXPATH,o.inifn);
   if (strcasecmp(temp,"DBF")==0)        o.type=edbt_dBase;
   else if (strcasecmp(temp,"ASCII")==0) o.type=edbt_ASCIIDOS;
   GetPrivateProfileString("Database","Name","",temp,MAXPATH,o.inifn);
   o.descr=temp;
  }
 if (o.type == edbt_Unknown) return false;
switch (o.type)
 {
 case edbt_dBase:
    GetPrivateProfileString("DBF","filename",o.datfn,temp,MAXPATH,o.inifn);
    o.datfn = temp;
    o.hdrfn = temp;
    break;
 case edbt_ASCIIDOS:
 case edbt_ASCIIUNIX:
    GetPrivateProfileString("ASCII","header",o.hdrfn,temp,MAXPATH,o.inifn);
    o.hdrfn=temp;
if (!o.datfn)
{
    GetPrivateProfileString("ASCII","stream",o.datfn,temp,MAXPATH,o.inifn);
    o.datfn=temp;
}
    GetPrivateProfileString("ASCII","terminator","CRLF",temp,MAXPATH,o.inifn);
    if (strcmp(temp,"LF")==0) o.type=edbt_ASCIIUNIX;
    else if (strcmp(temp,"CR")==0) o.type=edbt_ASCIIUNIX;
    else o.type=edbt_ASCIIDOS;
    break;
 default: return false;
 }
return true;
}

bool CheckSetupFile(const char * filename,OpenDb &o)
{
if (*filename==':') //open BDE, or equiv INI file
 {
 //look in the current directory
  TStr temp(filename+1,".INI");
  o.inifn=temp;
  return ReadDatabaseIni(o);
 }

if (o.type != edbt_Unknown)
 {
   o.datfn=filename;
   o.hdrfn=filename;
   return true;
 }

const char * ext = GetExtension((char*)filename);

if (strlen(ext) ==0 ) //dunno what kind of DB to use
    return false;

if (strcasecmp(ext,"INI")==0)
  {
   o.inifn=filename;
   ClipExtension(o.inifn);
   o.inifn == ".INI";
   return ReadDatabaseIni(o);
  }

if ((strcasecmp(ext,"DBF")==0)||(strcasecmp(ext,"DAT")==0))
  {
   o.type=edbt_dBase;
   o.datfn=filename;
   o.hdrfn=filename;
   return true;
  }
if (strcasecmp(ext,"DATABASE")==0)
  {
   o.type=edbt_WEB;
   o.datfn=filename;
   o.hdrfn=filename;
   return true;
  }
if (strcasecmp(ext,"ASC")==0)
  {
   o.type=edbt_ASCIIDOS;
   o.datfn=filename;
   o.inifn=filename;
   ClipExtension(o.inifn);
   o.inifn == ".INI";
   return ReadDatabaseIni(o);
  }
 return false; //unknown database type
}

DBFFile * OpenDB(const char * filename,EDBCacheSize CacheSize,
                 bool IgnoreReadOnly, EDBType Type)
{
 OpenDb o;
//   dspMessageBox("Before CheckSEtupFile",filename);

 o.type = Type;

 if (!CheckSetupFile(filename,o)) return NULL; //couldn't get enough info

DBFHeader* dbh;

  if (o.type==edbt_dBase)
  {
  try {
   dbh=new DBFHeader(DBFHeader::dBase);
   if (!dbh->LoadFromDBFFile(o.hdrfn)) {delete dbh; return NULL;}
   if (dbh->FieldCount()==0) {delete dbh; return NULL;} //must have fields
   }
   catch(...) {delete dbh; return NULL; }
  }

TDBFile * dbf=0;

if ((o.type==edbt_dBase)||(o.type==edbt_ASCIIDOS)||(o.type==edbt_ASCIIUNIX))
  {
  try{
  dbf=new TDBFixedFormatFile(dbh,o.datfn);
  }
   catch(...) {delete dbh; return NULL; }

  if ((dbf->RecordLength()<=0)||(!dbf->IsValid()))
   {delete dbf; return 0;};
  }

if (!dbf) return 0;

if (IgnoreReadOnly) dbf->FileStatus |= wr_Write;

TDBCache * cache=0;
DBFFile * dba;
if ((! dbf->IsCacheable)&&(CacheSize))
 cache = new TDBCache();
try {
  dba=new DBFFile( dbf,cache);
if (cache) cache->SetBufferSize(dba,CacheSize);
  }
   catch(...) {delete dbf; if (cache) delete cache; return NULL; }

return dba;
};

      */
#endif
