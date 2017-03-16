#include "rslib.h"
#pragma hdrstop

DBFRecord::DBFRecord(DBFFile * _db, count_t _CaseNum, bool am)
{
 mem=backup=0;
 OwnsMem=true;
 CaseNum=_CaseNum;
 IsModified=(CaseNum==0);
 db=_db;
 size_t rl = db->RecordLength();
 mem = AllocStr(rl);
 memset(mem,' ',rl);
 Clear();

#ifdef NO_DBF_ENCRYPTION
 if (db->ReadCaseData(CaseNum,mem,IsPasswordCorrect))
 {
      AllowModify= am && db->ReplaceAllowed();
      AllowBackup=true;
 }
#else
 IsPasswordCorrect = true;
 if (db->ReadCaseData(CaseNum,mem,IsPasswordCorrect))
   {
    if (IsPasswordCorrect)
    {
      AllowModify= am && db->ReplaceAllowed();
      AllowBackup=true;
    }
    else
    {
      AllowModify=false;
      AllowBackup=false;
      CaseNum = NEW_RECORD;
    }
   }
#endif
   else
   {
     AllowModify = am && db->AppendAllowed();
     AllowBackup = false; //no need to back up a blank record
     CaseNum = NEW_RECORD;
   }

};


//copy casebuf instead of going to the database.
DBFRecord::DBFRecord(DBFFile * _db)
{
 AllowBackup=false;
 mem=backup=0;
 OwnsMem=false;
 CaseNum=0;
 IsPasswordCorrect = true;
 IsModified=false;
 db=_db;
 AllowModify=false;
} //special constructor for TDBBrowser

DBFRecord::~DBFRecord()
{
 if (OwnsMem) if (mem) FreeStr(mem);
 if (backup) FreeStr(backup);
}

bool DBFRecord::IsDeleted()
{
 return mem ? db->GetDeleteFlag(*mem) == db_Deleted : false;
}

bool DoEncodeRecord(DBFFile * db, char * CaseBuf);
//declared in db_crypt.cpp

bool DBFRecord::ForceDecryption()
{
#ifndef NO_DBF_ENCRYPTION
  if (IsPasswordCorrect) return true; //already decrypted
  DoEncodeRecord(db, mem);
  IsPasswordCorrect = true;
#endif
  return true;
}


/*
Stream* DBFRecord::GetBlob(size_t fieldnum,TStr& filetype,bool ReadOnly)
{
 char s[MAXPATH];
 if (FieldLength(fieldnum) < 12) return 0; // filenames are 8.3
 if (!GetFieldData(fieldnum,s,MAXPATH)) return 0;

 TStr fname(db->filename());
 if (!*fname) //preview database -- return a disposable stream
  {
   filetype = "";
   return new MemoryStream;
   //doesn't work on preview databases!
  }

 ClipExtension(fname);
 AddBackslash(fname);
 fname += "BLOB";

 char * c = StripCharsFB(s," ");
 if (*c)
 {
  fname += "\\";
  fname += c;
 }
 else
 {
  if (!*filetype) filetype = "BIN";
  GetNewFilename(filetype,fname,fname);
 }

 SYSStream * stream;
 try
 {
 stream = new FileStream(fname,SYSStream::OMBinary,
       ReadOnly?SYSStream::ReadOnly : SYSStream::ReadWrite);
 } catch(...) {return 0;}

 filetype = ClipExtension(fname);
 if (!ReadOnly) IsModified = true;
 return stream;
}
  */

void DBFRecord::GetData(TNameValueList & o)
{
  size_t imax = FieldCount();
  for (size_t i = 0 ; i <= imax; i++)
  {
   size_t length;
   char * c = GetData(i,length);
   TStr s(c,length);
   o.Set(FieldName(i),s);
  }
}

void DBFRecord::operator = (TNameValueList & o)
{
 size_t i,imax=o.Count();
 for (i=0;i<imax;i++)
 {
   SetFieldData(o.Name(i),o.Value(i));
 }
}

size_t DBFRecord::FindFieldBySequence(uint16 SeqNum)
{
 return db->FindFieldBySequence(SeqNum);
}

void DBFRecord::Clear()
{
 if (backup) {delete[] backup; backup=0;}
 if (mem) memset(mem,' ',db->RecordLength());
  //fill with blanks for a new record.
} //clears all field values

void DBFRecord::SetAsNewRecord()
{
 CaseNum=NEW_RECORD;
 if (backup) {delete[] backup; backup=0;}
} //sets CaseNum to zero

/*
void DBFRecord::UseBuf(count_t num,char * buf)
//called ONLY by TDBBrowser in the ForEach() function
{
 CaseNum=num;
 IFDELBUF(backup);
 mem=buf;
// AllowModify= num ? db->ReplaceAllowed() : db->AppendAllowed();
};
  */     /*
void DBFRecord::ClearPassword()
{
 char * fieldname = "\xAE";
 SetFieldData(fieldname,"");
}
    */
void DBFRecord::SetPassword(const char * M, const char * P)
{
#ifndef NO_DBF_ENCRYPTION
 if (!db->HasPassword) return;
 if (!db->crypt)
  {
   TStringList sl;
   sl.Add(M);
   sl.Add(P);
   db->SetPasswords(sl);
  }
 if (mem)
  {
   db->crypt->SetRecordPasswords(db,mem,M,P);
  }
 if (backup)
  {
   db->crypt->SetRecordPasswords(db,backup,M,P);
  }
#endif
}

void DBFRecord::Backup()
{
if ((!mem)||(CaseNum==0)||(CaseNum==INVALID_CASE)) return;
if (backup)
 {delete[] backup; backup =NULL; }

 size_t rl=db->RecordLength();
 backup=AllocStr(rl);
 memcpy(backup,mem,rl);
};

size_t DBFRecord::RecordLength()
{return db->RecordLength();
}

void DBFRecord::RestoreBackup()
{
if ((!mem)||(!backup)) return;
size_t rl=db->RecordLength();
memcpy(mem,backup,rl);
};


char* DBFRecord::FieldName(size_t fieldnum)
{return db->GetFieldName(fieldnum); }

size_t DBFRecord::FieldNumber(const char * fieldname)
 {return db->GetFieldNum(fieldname);}

size_t DBFRecord::FieldLength(const char * fieldname)
{ DBFFieldRec * R;
  R=db->GetFieldInfo(fieldname);
  if (R) return R->FieldLength;
  return 0;
}

size_t DBFRecord::FieldLength(size_t fieldnum)
{ DBFFieldRec * R;
  R=db->GetFieldInfo(fieldnum);
  if (R) return R->FieldLength;
  return 0;
}

EDBFieldType DBFRecord::FieldType(const char * fieldname)
{ DBFFieldRec * R;
  R=db->GetFieldInfo(fieldname);
  if (R) return R->FieldType;
  return db_ft_NoData;
}

char* DBFRecord::GetData(size_t fieldnum, size_t& length)
{
 DBFFieldRec* R;
  R=db->GetFieldInfo(fieldnum);
  if (!R) return 0;
  length=R->FieldLength;
  return mem + R->Offset;
}

bool DBFRecord::SetData(size_t fieldnum, const char * data, size_t& length)
{
 if ((!AllowModify)||(!data)||(!length)) return false;
if ((!backup)&&(AllowBackup)) Backup();
 DBFFieldRec* R;
  R=db->GetFieldInfo(fieldnum);
  if (!R) return false;
  char * c = mem + R->Offset;
  length = length ? min(R->FieldLength,length) : R->FieldLength;
  size_t i = 0;
  IsModified=true;
  if (!R->IsVisible)
    {
     memmove(c,data,length);
     return true;
    }

  while ( i < length )
   {
    if (!*data) break;
    *c = *data;
    i++;
    c++;
    data++;
   }
  while (i < R->FieldLength)
   {
    *c = ' ';
    i++;
    c++;
   }
 // memcpy(c,data,length);
  return true;
}

bool DBFRecord::GetFieldData(const char * fieldname,char * destination,size_t maxcopy)
{
 size_t length;
 char * c = GetData(FieldNumber(fieldname),length);
 if (!c) return false;
 length=min(length,maxcopy);
 memcpy(destination,c,length);
 destination[length]=0;
 return true;
}

bool DBFRecord::GetFieldData(size_t fieldnum,char * destination,size_t maxcopy)
{
 size_t length;
 char * c = GetData(fieldnum,length);
 if (!c) return false;
 length=min(length,maxcopy);
 memcpy(destination,c,length);
 destination[length]=0;
 return true;
}

bool DBFRecord::SetFieldData(const char * fieldname,const char * source,size_t maxcopy)
{return SetData(FieldNumber(fieldname),source,maxcopy);
}

bool DBFRecord::SetFieldData(size_t fieldnum,const char * source,size_t maxcopy)
{return SetData(fieldnum,source,maxcopy);
}

bool DBFRecord::CommitChanges()
{if (!IsModified)
   {
   // THROWMSG("Not modified");
   return true; //that's good, don't show an error
   }
 if (!mem)
   {
    //THROWMSG("No buffer");
    return false; //big error
   }
if (db->HasChevron)
  {
  char name[2];name[0]=175; name[1]=0;
  size_t len;
  char *c =GetData(FieldNumber(name),len);
  if (c && len) *c = 175;
  }
count_t c=db->UpdateCase(CaseNum,mem,backup);
 if (backup) {delete[] backup; backup=0;}
if (c) {if (!CaseNum) CaseNum=c; return true;}
return false;
}

//utility functions
bool DBFRecord::CopyRecord(DBFRecord& r, bool BinaryCopy)
{
if (!AllowModify) return false;
Backup();
size_t i,imax=r.FieldCount();

size_t fieldnum,length;
char * src;
IsModified=true;

if (BinaryCopy)
 mem[0] = r.mem[0];
 else
 db->SetDeleteFlag(mem, r.db->GetDeleteFlag(r.mem));

 //first entry is the deletion flag for DBF. May have erratic consequences
 //for other databases, but who cares?

  for (i=1;i<=imax;i++)
  {
   fieldnum=FieldNumber(r.FieldName(i));
   if (fieldnum)
   {
    src=r.GetData(i,length); //sets length
    if (BinaryCopy)
    {size_t destlength;
     char * dest = GetData(i,destlength);
     if (length < destlength) memset(dest,' ',destlength);
     memcpy(dest,src,min(length,destlength));
    }
    else
    {
    SetData(fieldnum,src,length);
    }
   }
  }
return true;
}

size_t DBFRecord::FieldCount() {return db->FieldCount();}

