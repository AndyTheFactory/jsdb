#include "rslib.h"
#pragma hdrstop

bool DBNeedsCrypt(DBFFile * db)
{
    DBFFieldRec* R;
    R=db->GetFieldInfo(CRYPTFIELDNAME);
    if (R) return true;
    return false;
}

//------------------------------
DBFFile::DBFFile(DBFBaseIO * _db,DBFCache * _cache)
 : /*TDataSource(), */db(_db), cache(0)
{
 if ((!db)||(!IsValid()))
 {
  db = 0;
  throw xdb("Database invalid","");
 }

 cache = _cache;

#ifndef NO_DBF_ENCRYPTION
 crypt=0;
#endif

 HasPassword = HasChevron = HasBookmark = HasUsername = HasOwnPassword = 0;

 char temp[20];
 temp[1]=0;

 temp[0]=175; temp[1]=0;
 HasChevron = ( db->GetFieldInfo(temp)!=0); //175 Chevron

 temp[0]=174; temp[1]=0;
 HasPassword = ( db->GetFieldInfo(temp)!=0); //174 Password

 temp[0]=172; temp[1]=0;
 HasBookmark = ( db->GetFieldInfo(temp)!=0); //172 Bookmark

 temp[0]=171;  temp[1]=173; temp[2]=0;
 HasUsername = ( db->GetFieldInfo(temp)!=0); //173  Username
}

DBFFile::~DBFFile()
 {
// if (db) delete db;
//#ifndef NO_DBF_ENCRYPTION
// if (crypt) delete crypt;
//#endif
// if (cache) delete cache;
 };

void DBFFile::SetCacheSize(EDBCacheSize CacheSize)
  {
    if (cache) cache->SetBufferSize(this,CacheSize);
  }

bool DBFFile::ReadRecordData(TParameterList & RecordData, count_t & CaseNum)
{
 try {
 DBFRecord Record(this,CaseNum);
 size_t i, imax = Record.FieldCount();
 for (i = 1; i <= imax; i++)
  {
   size_t len=0;
   char * c = Record.GetData(i,len);
   char * fn = Record.FieldName(i);
   RecordData.Set(fn,strlen(fn),c,len);
  }
  CaseNum = Record.CaseNum;
 } catch(...) {return false;  }

 return true;
}

bool DBFFile::WriteRecordData(TParameterList & RecordData, count_t & CaseNum)
{
 try {
 DBFRecord Record(this,CaseNum);
 size_t i, imax = Record.FieldCount();
 for (i = 1; i <= imax; i++)
  {
   char * v = RecordData.Get(Record.FieldName(i));
   Record.SetFieldData(i,v);
  }
  Record.CommitChanges();
  CaseNum = Record.CaseNum;
 } catch(...) {return false;  }
 return true;
}

bool DBFFile::ReplaceAllowed()
{ return (db->FileStatus & wr_Replace); }

bool DBFFile::AppendAllowed()
{ return (db->FileStatus & wr_Append); }

bool DBFFile::ReadAllowed()
{ return (db->FileStatus & wr_Read); }

bool DBFFile::IsValid()
{ return db->IsValid(); } //read,write,or append

count_t DBFFile::RecordCount()
{ return db->RecordCount(); }

size_t DBFFile::RecordLength()
{ return db->RecordLength(); }

size_t DBFFile::FieldCount()
{ return db->FieldCount(); }

size_t DBFFile::FindFieldBySequence(uint16 SeqNum)
{
DBFFieldRec* R;
FOREACHPTR(R,(*db))
 if (R->SequenceNumber == SeqNum) return R->FieldNum;
DONEFOREACH
return 0;
}

void DBFFile::SetSequenceNumber(uint16 SeqNum, const char * fieldname)
{ DBFFieldRec* R;
  R = db->GetFieldInfo(fieldname);
  if (R) R->SequenceNumber = SeqNum;
}

void DBFFile::SetSequenceNumber(uint16 SeqNum, size_t fieldnum)
{ DBFFieldRec* R;
  R = db->GetFieldInfo(fieldnum);
  if (R) R->SequenceNumber = SeqNum;
}

void DBFFile::SetQuestType(int16 QuestType, const char * fieldname)
{ DBFFieldRec* R;
  R = db->GetFieldInfo(fieldname);
  if (R) R->QuestType = QuestType;
}

void DBFFile::SetQuestType(int16 QuestType, size_t fieldnum)
{ DBFFieldRec* R;
  R = db->GetFieldInfo(fieldnum);
  if (R) R->QuestType = QuestType;
}

DBFFieldRec* DBFFile::GetFieldInfo(const char *name)
{ return db->GetFieldInfo(name); }

DBFFieldRec* DBFFile::GetFieldInfo(size_t fieldnum)
{ return db->GetFieldInfo(fieldnum); }

char* DBFFile::GetFieldName(size_t fieldnum)
{
DBFFieldRec * R;
R=db->GetFieldInfo(fieldnum);
if (R) return R->FieldName;
return 0;
};

size_t DBFFile::GetFieldNum(const char * fieldname)
{ return db->GetFieldNum(fieldname); }

const char * DBFFile::filename()
{ return db->filename(); }

void DBFFile::SetPasswords(char * pw)
  {
#ifndef NO_DBF_ENCRYPTION
//   if (crypt) delete crypt;
   crypt=new DBFCrypt(pw);
#endif
  }

void DBFFile::SetPasswords(int16 Password, int16 Master)
 {
#ifndef NO_DBF_ENCRYPTION
//   if (crypt) delete crypt;
   crypt=new DBFCrypt(Password,Master);
#endif
 }

void DBFFile::SetPasswords(TStringList & pw)
  {
#ifndef NO_DBF_ENCRYPTION
//   if (crypt) delete crypt;
   crypt=new DBFCrypt(pw);
#endif
  }

EDBDeleteSetting DBFFile::SetDeleteFlag(count_t CaseNum,  EDBDeleteSetting E)
{
 if (cache && IsCached(CaseNum))
    {
     char * c = cache->RecordData(this,CaseNum);
     if (c) db->SetDelete(c,E);
    }

 return db->SetDelete(CaseNum,E);
}

EDBDeleteSetting DBFFile::GetDeleteFlag(count_t CaseNum)
{
 if (cache && IsCached(CaseNum))
    {
     char * c = cache->RecordData(this,CaseNum);
     return db->SetDelete(c,db_CheckDelete);
    }

 return db->SetDelete(CaseNum,db_CheckDelete);
}

EDBDeleteSetting DBFFile::SetDeleteFlag(char * c,  EDBDeleteSetting E)
{ return db->SetDelete(c,E); }

EDBDeleteSetting DBFFile::GetDeleteFlag(char * c)
{ return db->SetDelete(c,db_CheckDelete); }

bool DBFFile::IsCached(count_t CaseNum)
{ return cache ? cache->HasRecord(CaseNum) : false ;}

bool DBFFile::GetDataC(count_t CaseNum, size_t fieldnum,TChars& data)
{
 if (!cache || !ReadAllowed()) return false;

 DBFFieldRec* R;
 R = db->GetFieldInfo(fieldnum);
 if (!R) return false;

 const char * c = cache->RecordData(this,CaseNum);

 if (!c) return false;
 data.Set(c + R->Offset,R->FieldLength);
 return true;
}

bool DBFFile::ReadCaseRawBuf(count_t CaseNum,char * outbuf)
{
 if ((!outbuf)||(!CaseNum)) return false;
 //load it into the cache
 if (IsCached(CaseNum))
    {
   //  cache->ReloadRecord(this,CaseNum);
   //  if (!IsCached(CaseNum)) return false;
     memcpy(outbuf,cache->RecordData(this,CaseNum),RecordLength());
    }
 else
    {
     if (!db->ReadData(CaseNum,1,outbuf,ULONG_MAX)) return false;
        //database calculates the length
    }
 return true;
} //doesn't decrypt

bool DBFFile::ReadCaseData(count_t CaseNum,char * outbuf, bool &PasswordOK)
{
 if (!ReadAllowed()) return false;
 if (!ReadCaseRawBuf(CaseNum,outbuf)) return false;
 PasswordOK = DecryptRecord(outbuf);
 return true;
}

count_t DBFFile::SaveCaseData(count_t CaseNum,const char * inbuf)
{
 count_t ret;

#ifndef NO_DBF_ENCRYPTION
 if (HasPassword)
  {
   TChars Mem(inbuf,RecordLength());
   EncryptRecord(Mem);
   ret = WriteCaseRawBuf(CaseNum,Mem);
  }
 else
#endif
   ret = WriteCaseRawBuf(CaseNum,inbuf);

 if (!ret) return 0;
 if (IsCached(CaseNum))
     if (!cache->ReloadRecord(this,CaseNum)) return 0;
 return ret;
}

bool DBFFile::WriteCaseRawBuf(count_t CaseNum,const char * inbuf)
{
const char * c = inbuf;
//  count_t ret;
size_t rl = RecordLength();

if (CaseNum==0)
 {
  if (!AppendAllowed()) return 0;
  return db->AddCase(inbuf);
 }

if (!ReplaceAllowed()) return 0;
  return db->SaveData(CaseNum,1,c,rl);

}

count_t DBFFile::UpdateCase(count_t CaseNum, char* Changes, char* Backup)
{
 if (!db) return 0;
 if (Changes==0) return 0;
 if (CaseNum==NO_COUNT) return NO_COUNT;

 char *p1,*p2,*p3;
 unsigned int i;
 long l;
 size_t rl;

 if (CaseNum==0)
    {
    if (!AppendAllowed())
    {
    //THROWMSG("Append not allowed");
    return 0;
    }
#ifndef NO_DBF_ENCRYPTION
    if (HasPassword)
     {
     TChars Mem(Changes,RecordLength());
     EncryptRecord(Mem);
     return db->AddCase(Mem);
     }
#endif
    return db->AddCase(Changes);
    }
 if (!ReplaceAllowed())
  {//THROWMSG("Replace not allowed");
  return 0;
  }

 if (!Backup)
    {
     return SaveCaseData(CaseNum,Changes);
    };

 rl=db->RecordLength();

 TChars NewData(rl);

 {
 bool PwOk;
 if (!ReadCaseData(CaseNum,NewData,PwOk)) return 0;
 if (!PwOk) return 0;
 }

 if (memcmp(Backup,NewData,rl)== 0)
    {//nothing changed--save and flag success
     return SaveCaseData(CaseNum,Changes);
    }

 return 0;
//The database was changed by someone else, so confirm replace
//    char msg1[128],msg2[32];
//    msg1[0]=msg2[0]=0;
//set default messages, then replace them with the language
//messages. Since these routines are so low-level, I think
//it's OK to use them where there might not be a plug-in
//language library.


//     strcpy(msg1,"Someone else changed this record while you were"
//                 "working. Continue with this save?");
//     strcpy(msg2,"Database Message");
//switch (dspMessageYesNo(string(_hInstance,5).c_str(),string(_hInstance,6).c_str() ))
/*
switch (dspMessageYesNo(
    "Someone else changed this record while you were"
    "working. Continue with this save?",
    "Database Manager"))
	 {
	   case IDYES :
				 p1=(char *)Backup;
                 p2=(char *)Changes;
                 p3=(char *)NewData;
                  for (i=0; i<rl ; i++)
                   {
                   if (*p1 != *p3) {*p1=*p2;};
                   p1++;p2++;p3++ ;
                   };
                   l=SaveCaseData(CaseNum,Backup);
                   memcpy(Changes,Backup,rl);
                   break;
//backup now represents the state of the database. It is also
//copied to the Changes record.
       case IDNO :
       default:  return 0;
     };
     return l;
*/
  };

void DBFFile::ReloadFile()
 {
  if (cache) cache->Clear();
 };

 bool DBFFile::WriteSummaryInformation()
  {
   return db->WriteSummaryInformation();
  }

bool Overlap (count_t l1,count_t h1,count_t l2, count_t h2)
{
 return ( ((l2 <= l1) && (l1 <= h2)) || ((l1 <= l2) && (l2 <= h1)) ) ;
}

count_t DBFFile::ReadCaseRange(count_t start, count_t count, char * outbuf,count_t max)
{ return db->ReadData(start,count,outbuf,max); }

count_t DBFFile::SaveCaseRange(count_t start, count_t count,const char * inbuf,count_t max)
{ return db->SaveData(start,count,inbuf,max);
}

bool DBFFile::DecryptRecord(char * CaseBuf)
{ if (!HasPassword) return true;
#ifndef NO_DBF_ENCRYPTION
  if (crypt) return crypt->DecryptRecord(this,CaseBuf);
  //encrypted database, but we don't have the password, so just do a blind
  //copy. To tell the saver not to encrypt the database as it saves,
  //set the deletion flag to a tilde. This flag tells the DBFFile to
  //bypass encrypting or decrypting the record. Of course, this means that
  //deleted records... will not be saved

 // if (*CaseBuf == db_OKFlag)
  //return true, indicating to the database that it can handle the data.
  //otherwise, it would clear out the information and think that the DB
  //read had failed, which it didn't.
 //   {*CaseBuf = db_EncryptedFlag;
 //    return true;
 //   }
  //the record was deleted, so we're going to indicate that we couldn't
  //decrypt it. This should flag an error and prevent TDBRecord from being
  //able to commit its changes.
#endif
  return false;
}

bool DBFFile::EncryptRecord(char * CaseBuf)
{ if (!HasPassword) return true;
  //flag that we're bypassing the encryption
 // if (*CaseBuf == db_EncryptedFlag)
 //   {*CaseBuf =db_OKFlag;
 //    return true;
 //   }
  //now create a default encryptor object, so that records which were
  //decrypted by a different database and saved into this one (as in
  //Strip Fields) get properly encrypted, if the password field is set
  //with a password.
 // if (!crypt) SetPasswords(0);
#ifndef NO_DBF_ENCRYPTION
  return (crypt) ? crypt->EncryptRecord(this,CaseBuf) : 0;
#else
  return false;
#endif
}
//if passwords aren't set
