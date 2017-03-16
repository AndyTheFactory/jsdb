#include "rslib.h"
#pragma hdrstop
#include <time.h>

//#include "rs/dbf.h"
//#include "rs/file.h"
//-----------------------------------
//
//  DBFBaseIO
//  virtual ancestor to everything
//
//-------------------------------

DBFBaseIO::DBFBaseIO() {IsCacheable=IsFileOpen=FileStatus=0;};
DBFBaseIO::~DBFBaseIO() {};

const char * DBFBaseIO::filename() {return 0;};

bool DBFBaseIO::IsValid()  {return false;}

bool DBFBaseIO::WriteSummaryInformation() {return false;}

DBFFieldRec* DBFBaseIO::GetFieldInfo(const char *)
{
 return 0 ;
};
DBFFieldRec* DBFBaseIO::GetFieldInfo(size_t ){return 0;}

count_t DBFBaseIO::ReadData(count_t , count_t , char *,
			   count_t ) {return 0;};

count_t DBFBaseIO::SaveData(count_t , count_t , const char *,
			   count_t ){return 0;};

count_t DBFBaseIO::AddCase(const char *) {return 0;}

size_t DBFBaseIO::FieldCount() {return 0;};
size_t DBFBaseIO::RecordLength() {return 0;};
count_t DBFBaseIO::RecordCount() {return 0;};

EDBDeleteSetting DBFBaseIO::SetDelete(count_t , EDBDeleteSetting )
{return db_Unknown;};
EDBDeleteSetting DBFBaseIO::SetDelete(char * , EDBDeleteSetting )
{return db_Unknown;}

size_t DBFBaseIO::GetFieldNum(const char * ) {return 0;};


//--------------------------------------------------
//
//  DBFFixedFormat: generic header structure
//
//--------------------------------------------------

DBFFixedFormatFile::DBFFixedFormatFile(DBFHeader * hdr,const char * fname):
Filename(fname), Header(hdr)
{
 if (!Header) throw xdb("No header",fname);
  IsCacheable=true;
  IsFileOpen=true;
  switch(FileAttributes(fname))
  {
		case 1: FileStatus = wr_Read; break;
		case 2: FileStatus = wr_Write; break;
		case 3: FileStatus = wr_All; break;
		default: FileStatus = wr_CantOpen;
		         throw xdb("File not found",fname);
	};
};
     //assumes you've already put the necessary fields in the header.
DBFFixedFormatFile::~DBFFixedFormatFile()
{
//delete Header;
};

const char* DBFFixedFormatFile::filename()
{return Filename;}

bool DBFFixedFormatFile::IsValid()
 {
  return (FileStatus != wr_CantOpen);
 };

DBFFieldRec* DBFFixedFormatFile::GetFieldInfo(const char *name)
{return Header->GetFieldInfo(name);
};
DBFFieldRec* DBFFixedFormatFile::GetFieldInfo(size_t fieldnum)
{return Header->GetFieldInfo(fieldnum);
};

// RecordAddress:
//{
// return =Header->RecordLength * (CaseNum - 1) + Header->HeaderSize;
//};

count_t DBFFixedFormatFile::ReadData(count_t CaseNum, count_t count, char *destination,
			   count_t destsize)
{
  if ((FileStatus & wr_Read)==0) return 0;
  if ((CaseNum==0)||(CaseNum==NO_COUNT)) return 0;
  long FileOffset,l;
  size_t rl = Header->RecordLength,hs=Header->HeaderSize;

  FileOffset = rl * (CaseNum - 1) + hs;

  l=count * rl;
  if (l > destsize) l=destsize;

  l= LongPntrRead(Filename,FileOffset,l,destination);
  FileOffset -= hs;
  if ((l<=0) || (FileOffset < 0)) return 0;

  l = l + FileOffset ; //how far did it go into the data stream?
  l /= rl;
  return l;
};

count_t DBFFixedFormatFile::SaveData(count_t CaseNum, count_t count, const char *origin,
			   count_t origsize)
{
if ((origin ==0)||(origsize==0)||(origsize==NO_COUNT)) return 0;
if ((FileStatus & wr_Write) ==0) return 0;

//check the file length
 long FileOffset,l;
 size_t rl = Header->RecordLength, hs=Header->HeaderSize;
 const char * fn = filename();
 FileOffset=FileSize(fn);
 l = FileOffset - hs;

 switch (l % rl)
 {
 case 0: FileOffset = -1L; break; //simple append mode
 case 1: FileOffset-- ; break; //some moron probably added an EOF marker
 default: return 0;
 }

if (CaseNum == 0)
{//append data
 if ((FileStatus & wr_Append)==0) return 0;
}else
{//overwrite data
 if ((FileStatus & wr_Replace)==0) return 0;
 FileOffset = hs + ((CaseNum -1) * rl);
}
 l = LongPntrWrite(fn,FileOffset,count*rl,(char*)origin,true);
  //writes over pesky EOFs
  //FileOffset is modified by LongPntrWrite to be the starting position.
  //use the returned value to calculate the record number that we just wrote.
 FileOffset -= hs;
 if ((l<=0) || (FileOffset < 0)) return 0; //some sort of file access error

 l = FileOffset / rl ; //how far did it go into the data stream?
 return l+1; //last case number written
}

count_t DBFFixedFormatFile::AddCase(const char *Origin)
{
 return SaveData(0,1,Origin,Header->RecordLength);
};

size_t DBFFixedFormatFile::FieldCount()
{return Header->FieldCount();};

size_t DBFFixedFormatFile::RecordLength()
{return Header->RecordLength;};

count_t DBFFixedFormatFile::RecordCount()
{
 if ((FileStatus & wr_Read) == 0) return 0;
  //sometimes, you have write permission but not read permission
 long FileOffset,l;
 size_t rl = Header->RecordLength, hs=Header->HeaderSize;
 const char * fn = filename();
 FileOffset=FileSize(fn);
 if (FileOffset == 0) return 0;
 l = FileOffset - hs;
 return l / rl;
};

EDBDeleteSetting DBFFixedFormatFile::SetDelete(char * c, EDBDeleteSetting DeleteSetting)
{
if (!c) return db_OK;
if (Header->Type!=DBFHeader::dBase) return db_OK;
if (DeleteSetting==db_CheckDelete)
  {
   switch (*c)
   {
    case db_DeleteFlag: return db_Deleted;
    case db_LockFlag:   return db_Locked;
    case 0:
    case db_OKFlag:     return db_OK;
    case db_EncryptedFlag: return db_Encrypted;
    default:            return db_Unknown; //ingore all else
   };
  };

switch (DeleteSetting)
  {
  case db_Deleted: *c=db_DeleteFlag;
       break;
  case db_Locked:  *c=db_LockFlag;
       break;
  case db_OK:      *c=db_OKFlag;
       break;
  case db_Encrypted: *c=db_EncryptedFlag;
       break;
  default: return db_Unknown;
  }
return DeleteSetting;
}

EDBDeleteSetting DBFFixedFormatFile::SetDelete(count_t CaseNum,
            EDBDeleteSetting DeleteSetting)
{
if ((CaseNum==0)||(CaseNum==NO_COUNT)) return db_OK;
if (Header->Type!=DBFHeader::dBase) return db_OK;

long FileOffset=Header->RecordLength * (CaseNum - 1) + Header->HeaderSize;
char c;
if (DeleteSetting==db_CheckDelete)
  {
   LongPntrRead(Filename,FileOffset,1,&c);
   switch (c)
   {
    case db_DeleteFlag: return db_Deleted;
    case db_LockFlag:   return db_Locked;
    case 0:
    case db_OKFlag:     return db_OK;
    case db_EncryptedFlag: return db_Encrypted;
    default:            return db_Unknown; //ingore all else
   };
  };

switch (DeleteSetting)
  {
  case db_Deleted: c=db_DeleteFlag;
       break;
  case db_Locked:  c=db_LockFlag;
       break;
  case db_OK:      c=db_OKFlag;
       break;
  case db_Encrypted: c=db_EncryptedFlag;
       break;
  default: return db_Unknown;
  }
   if (LongPntrWrite(Filename,FileOffset,1,&c,false)==1) return DeleteSetting;
return DeleteSetting;
};

size_t DBFFixedFormatFile::GetFieldNum(const char * fieldname)
{DBFFieldRec * R = Header->GetFieldInfo(fieldname);
 return R ? R->FieldNum : 0;
}

//--------------------------------------------------
//
//  DBFPreview
//
//--------------------------------------------------
DBFPreview::DBFPreview(DBFHeader * _Header)
: Header(_Header)
{
if (!Header) throw xdb("No header","Preview");
FileStatus=wr_All;
};

DBFPreview::~DBFPreview()
{
// delete Header;
};

size_t DBFPreview::FieldCount()
{return Header->FieldCount();}

size_t DBFPreview::RecordLength()
{return Header->RecordLength;}

DBFFieldRec* DBFPreview::GetFieldInfo(const char * name)
{return Header->GetFieldInfo(name);
};

DBFFieldRec* DBFPreview::GetFieldInfo(size_t fieldnum)
{return Header->GetFieldInfo(fieldnum);
}

count_t DBFPreview::ReadData(count_t CaseNum, count_t count, char *, count_t )
{return CaseNum+count-1;} //simulates a correct save

count_t DBFPreview::SaveData(count_t CaseNum, count_t count, const char *, count_t )
{return CaseNum+count-1;} //simulates a correct save

size_t DBFPreview::GetFieldNum(const char * fieldname)
{DBFFieldRec * R = Header->GetFieldInfo(fieldname);
 return R? R->FieldNum : 0;
}
count_t DBFPreview::AddCase(const char *)
{return true;
}

bool DBFPreview::IsValid() {return true;}

//-----------------------
//
// dBaseIII types
//
//-----------------------
bool DBFFixedFormatFile::WriteSummaryInformation()
{
  switch (Header->Type)
  {
  case DBFHeader::dBase:
    {
     count_t cases = RecordCount();
     RDBFHead Header;
     try {
     long start = 0;
     LongPntrRead(filename(), start,
                  sizeof(Header), (char*)&Header);
     Header.cases = cases;
     LongPntrWrite(filename(), start, sizeof(Header),
                   (char*)&Header,false);
     return true;
        }catch(...) {}
   }
  }
 return false;
}

/*
EDBFieldType EXPORTFUNC FieldType(char c)
{
  switch (c)
  {
case db_FieldNumber: return  db_ft_Number;
case db_FieldDate  : return  db_ft_Date  ;
case db_FieldTime  : return  db_ft_Time  ;
//case db_FieldInt   :
case db_FieldBlob  : return  db_ft_Blob;
case db_FieldFloat : return  db_ft_NoData;
case db_FieldLogic :
case db_FieldText  :
default :            return  db_ft_Char ;
  }
}

char EXPORTFUNC FieldType(EDBFieldType c)
{
  switch (c)
  {
case db_ft_Number : return db_FieldNumber;
case db_ft_Date   : return db_FieldDate;
case db_ft_Time   : return db_FieldTime;
case db_ft_NoData : return db_FieldInt;
case db_ft_Blob   : return db_FieldBlob;
case db_ft_Char   :
default :  return db_FieldText;
  }
}
*/


