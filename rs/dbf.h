#ifndef _RS_DBF_H
#define _RS_DBF_H

#ifndef _RS_STREAM_H
#include "rs/stream.h"
#endif

#ifndef _RS_SORT_H
#include "rs/system.h"
#endif

#ifndef _RS_SORT_H
#include "rs/sort.h"
#endif

#ifndef _RS_FORMTYPE_H
#include "rs/formtype.h"
#endif

// for using flat-file data tables
// system name: DBF

class DBFFile;
class DBFRecord;
class DBFHeader;

#ifndef NO_DBF_ENCRYPTION
class DBFCrypt;
#endif

#define TDatabase DBFFile
#define TDBRecord DBFRecord

// FileStatus flags

#define wr_CantOpen 0x0000

#define wr_Read     0x0001
#define wr_Replace  0x0010 //replace existing records
#define wr_Append   0x0100 //add new records

#define wr_ReadOnly wr_Read

#define wr_NoAdd    (wr_Read | wr_Replace)
#define wr_All      (wr_Read | wr_Replace | wr_Append)
#define wr_Write    (wr_Replace | wr_Append)

#define wr_Unknown  0x0008  //open, but in indeterminate state
#define wr_TryLater 0x0020  //file locked, but will be there later.
#define wr_SNAFU    0x0040  //bum luck. All you can do is save the
                            //transaction and hope it works later.
                            //maybe the disk crashed or something.
#define wr_Denied   0x1000  //server denied access

//dbf-specific values

#define db_dbf_HdrEndChar 13
#define db_DeleteFlag '*'
#define db_LockFlag   '!'
#define db_OKFlag     ' '
#define db_EncryptedFlag  '~'

#define MAX_ANALYSISSTRING 4096
#define MAX_MULTCHOICE 256
#define float80 long double

#define CRYPTFIELDNAME "\xAE\00"

//Transaction constants
#define INVALID_CASE NOT_FOUND
#define NEW_RECORD   0L
#define INVALID_TRX  NOT_FOUND
#define NEW_TRX      0L
#define READONLY_TRX INVALID_TRX

#define MAX_FIELD USHRT_MAX

//return error values:
// For a size_t, NOT_FOUND is the error
// for a count_t, NO_COUNT is the error
// EXCEPT: database reads and writes return
//         zero if an error occurs.
//
// CaseNum is a count_t
// FieldNum is a size_t
// a CaseNum of 0 is a new record
// a FieldNum of 0 is the deletion mark (and is a valid field,
//          but only for DBF or record-locking databases.);


void MakeRandom(char * block, int16 blocksize);

bool DBNeedsCrypt(DBFFile * db);

DBFFile  * PreviewDB(DBFHeader * Header);

//@{

/*!
headerfile can be the same file as the database,
a separate file dbf header, or, for an ASCII file, an ini file
[file]
type=DOS|UNIX|MAC|DBF
[fields]
name1=length
name2=length
*/

DBFFile * OpenDBF(const char * datafile,
                   const char * headerfile=0,
                 //  EDBType Type = edbt_Unknown,
                   bool IgnoreReadOnly = false,
                   EDBCacheSize CacheSize=CACHE_DEFAULT
                   );

DBFFile * OpenDBF(const char * datafile,
                   TParameterList& header,
                   int Type = 0,
                   bool IgnoreReadOnly = false,
                   EDBCacheSize CacheSize=CACHE_DEFAULT
                   );
//@}
// Some really basic classes

struct RDBFHead
    { uint8 filetype;
      uint8 year, month, day;
      uint32 cases;
      unsigned char headerlength[2];
      unsigned char reclength[2];
      uint8 reserved[16];
      char Time[4];
    };

struct  RWebHead
    { char  Signature[4];
      char  HeaderLength[4]; //int32
      char  RecordLength[4]; //int32
      char  CurTime[4];      //int32
      char  Filler1[4];      //int32
      char  Filler2[4];      //int32
      char  Filler3[4];      //int32
      char  CgiTime[4];      //int32
    };

struct RDBFField
    { char name[10];
      char zerobyte;
      char fieldtype;
      int32 dataaddr; //for memo field
      uint8 length;
      uint8 decimalcount;
      uint16 OldSwinFieldLength;
      uint8 reserved[12];
   } ;


struct DBFFieldRec
    { TStr FieldName;
      size_t FieldLength;
      size_t FieldNum;
      size_t Offset;
      uint16 SequenceNumber;
      int32 MemoAddr;
      int16 QuestType;
      uint32 Flags1, Flags2;
      EDBFieldType FieldType; //db_ft_ constants
      bool IsVisible; //user-field or system field?

      DBFFieldRec()
       {
        FieldType = db_ft_NoData; FieldLength = FieldNum = Offset = 0;
        SequenceNumber =0;
        IsVisible = true;
        MemoAddr = 0;
        QuestType = 0;
        Flags1 = Flags2 = 0;
       }
    };

class DBFBaseIO //base class
   { public:
     bool IsCacheable;
     bool IsFileOpen;
     uint16 FileStatus;
     //wr_ write-read permissions. May be wr_TryLater,
     //which indicates a file locked by someone else
     virtual const char* filename();
     DBFBaseIO();
     virtual ~DBFBaseIO();

     virtual bool IsValid();
     //returns false if there is a problem with the file, such as it
     //being an invalid file or something

     virtual bool WriteSummaryInformation();
     //save the current state of the database in the header or ini file
     //returns true if it did anything

     virtual DBFFieldRec* GetFieldInfo(const char *name);
     virtual DBFFieldRec* GetFieldInfo(size_t fieldnum);

     virtual count_t ReadData(count_t CaseNum, count_t count, char *destination,
			   count_t destsize);

     virtual count_t SaveData(count_t CaseNum, count_t count, const char *origin,
			   count_t origsize);

     virtual count_t AddCase(const char *Origin);

     virtual size_t FieldCount();
     virtual size_t RecordLength();
     virtual count_t RecordCount();

     virtual EDBDeleteSetting SetDelete(count_t CaseNum, EDBDeleteSetting DeleteSetting);
     virtual EDBDeleteSetting SetDelete(char * casebuf, EDBDeleteSetting DeleteSetting);
     virtual size_t GetFieldNum(const char * fieldname);
     size_t Count() {return FieldCount();}
     DBFFieldRec* operator [] (size_t i) {return GetFieldInfo(i);}
   };

class DBFHeader
{
protected:
  TRow<DBFFieldRec> fields;
public:
  enum TType {Normal,dBase,ASCIIDOS,ASCIIUNIX,ASCIIMAC,WEB} Type;
  DBFHeader(TType Type);
  ~DBFHeader();

  size_t RecordLength;
  size_t HeaderSize;
  TStr FileName;

  size_t FieldCount();
  void Clear();
  bool AddField(const char * name,uint16 length,
                EDBFieldType type=db_ft_Char,
                int32 memoaddr=0,
                int32 offset = 0);
  DBFFieldRec* GetFieldInfo(const char * name);
  DBFFieldRec* GetFieldInfo(size_t fieldnum);
  bool LoadFromFile(const char * filename); //reads a dbf header
  bool LoadDBF(Stream& data,const char* header); //reads a dbf header
  bool LoadWEB(Stream& data,const char* header); //reads a dbf header
  bool CreateFile(const char * filename); //writes a dbf header
  bool IsValidRecordLength();
  void CalculateSizes();
};

/*
class DBFReadFile: public DBFHeader, TDataList
{
 protected:
  TPointer<Stream> File;
  TChars Buffer1, Buffer2, LastData;
  count_t Record1, Record2;
  count_t LastCase;
  bool HasPassword;
  size_t PWOffset;

 public:
  TParameterList Options;

  enum EFileType {ft_None,ft_DBF,ft_WEB,ft_WRK} FileType;

  DBFReadFile();
  ~DBFReadFile();

  bool  Open(const char * pFileName); //

  bool  IsOpen(){return File != NULL;}
  void  Close();

  const char * filename() {return FileName;}

  count_t GetCurCase(){ return Record1; }

  const char * GetDataC(count_t Case, size_t FieldNum);

  const char * GetDataC(int FieldNum)
              { return GetDataC(Record1, FieldNum); }

  const char * GetDataC(const char * Name)
              { return GetDataC(Record1, FindColumn(Name)); }

  bool   SeekRecord(count_t Case,size_t offset = 0); //
  bool   IsValidPW(const char * Password);
  bool   IsSameData(count_t Case1, count_t Case2);
  bool   IsDeleted(count_t Case);

  size_t ColumnCount() {return fields.Count();}

  count_t RowCount() {return LastCase; }

  count_t HeaderLen(){ return HeaderSize;}

  count_t RecordLen(){ return RecordLength;}

  const char * ColumnTitle(size_t FieldNum)
              {return fields[FieldNum]->FieldName;}

  bool   IsVisible(size_t FieldNum)
              {return fields[FieldNum]->IsVisible;}

  bool   IsNumber(size_t FieldNum)
              {return fields[FieldNum]->FieldType == db_ft_Number;}

  size_t Offset(size_t FieldNum)
              {return fields[FieldNum]->Offset;}

  int32  Len(size_t FieldNum)
              {return fields[FieldNum]->FieldLength;}

  int    FieldType(size_t FieldNum)
              {return fields[FieldNum]->FieldType;}

  bool   ReadData(count_t Case,TNameValueList& data);

  bool   WriteData(count_t Case,TNameValueList& data);

 protected:
  bool   OpenDBF(const char * pFileName); //
  bool   OpenWeb(const char * pFileName); //
  bool   ReadData(count_t CurCase,TChars& data);
  void   Decrypt(int Len);
};
 */

class DBFPreview : public DBFBaseIO
{
public:
 TPointer<DBFHeader> Header;
 DBFPreview(DBFHeader * Header);
 virtual ~DBFPreview();
 size_t FieldCount();
 size_t RecordLength();
 DBFFieldRec* GetFieldInfo(const char * name);
 DBFFieldRec* GetFieldInfo(size_t fieldnum);
 count_t ReadData(count_t CaseNum, count_t count, char *destination,
			   count_t destsize);

 count_t SaveData(count_t CaseNum, count_t count, const char *origin,
			   count_t origsize);
 size_t GetFieldNum(const char * fieldname);
 count_t AddCase(const char *Origin);
 bool IsValid();
};

class DBFFixedFormatFile : public DBFBaseIO
{
public:
     TStr Filename;
     TPointer<DBFHeader> Header;

     const char* filename();
     DBFFixedFormatFile(DBFHeader * Header,const char * Filename);
     //assumes you've already put the necessary fields in the header.
     virtual ~DBFFixedFormatFile();

     bool IsValid();
     bool WriteSummaryInformation();

     DBFFieldRec* GetFieldInfo(const char *name);
     DBFFieldRec* GetFieldInfo(size_t fieldnum);

     count_t ReadData(count_t CaseNum, count_t count, char *destination,
			   count_t destsize);

     count_t SaveData(count_t CaseNum, count_t count, const char *origin,
			   count_t origsize);

     count_t AddCase(const char *Origin);

     size_t FieldCount();
     size_t RecordLength();
     count_t RecordCount();

     EDBDeleteSetting SetDelete(count_t CaseNum, EDBDeleteSetting DeleteSetting);
     EDBDeleteSetting SetDelete(char * casebuf, EDBDeleteSetting DeleteSetting);
     size_t GetFieldNum(const char * fieldname);
};

#ifndef NO_DBF_ENCRYPTION

class DBFCrypt
  {
  protected:
   size_t keycount;
   int32 *keys;
  public:
   DBFCrypt(char * password);
   DBFCrypt(int16 pw, int16 master);
   DBFCrypt(TStringList & password);
    //comma-delimited passwords accepted.
   ~DBFCrypt();
   bool IsRecordEncrypted(DBFFile * db,const char * CaseBuf);
   bool DoesRecordMatchUser(DBFFile * db,const char * CaseBuf);
   bool DecryptRecord(DBFFile * db,char * CaseBuf);
   //if another user has encrypted the record, but this user
   //does not have an encryption key, tough beans.
   bool EncryptRecord(DBFFile * db,char * CaseBuf);
   //if another user has already encrypted the record, but this user
   //does not yet have an encryption entry, the encryption fails
   bool SetRecordPasswords(DBFFile * db,char * CaseBuf,
                       const char * Master,const char * Pass);
   //copies the key for another user and password
  };

#endif

class DBFRecord
  {
protected:
   DBFFile * db;
//   char * mem,* backup;
   bool OwnsMem;
public:
   count_t CaseNum;
   char * mem,* backup;
   bool IsModified,AllowModify,AllowBackup;
   bool IsPasswordCorrect;
   //diskimage and backup are both encrypted images. Their layout may
   //not correspond exactly to the memory layout that is stored in mem.

   DBFRecord(DBFFile * db, count_t _CaseNum, bool _AllowModify=true);
//   DBFRecord(DBFFile * db, count_t _CaseNum, char *casebuf,bool _AllowModify=true);
//this was supposed to be a copy constructor, but we never used it.
   DBFRecord(DBFFile* db);
   ~DBFRecord();

protected:
//   DBFRecord(DBFFile * db);
      //special constructor for TDBBrowser
      //the new record can't be modified or backed up.
//   void UseBuf(count_t num,char *);
      //called ONLY by TDBBrowser in its ForEach() function

public:
   DBFFile * Database() {return db;}
   size_t FindFieldBySequence(uint16 SeqNum);
   //load a new value. Can't modify it.
   void Clear(); //clears all field values
   void SetAsNewRecord(); //sets CaseNum to zero

   bool IsDeleted();
   bool ForceDecryption();
   void SetPassword(const char * Master=0, const char * Pass=0);
   void SetPassword(uint16 master, uint16 pass);

   void Backup();
   void RestoreBackup();

   size_t RecordLength();

   size_t FieldCount();
   size_t FieldNumber(const char * fieldname);// {return db->GetFieldNum(fieldname);};

   size_t FieldLength(const char * fieldname);
   size_t FieldLength(size_t fieldnum);

   char* FieldName(size_t fieldnum);
   EDBFieldType FieldType(const char * fieldname); //one of the db_field constants

   char* GetData(size_t fieldnum, size_t& length);
   bool SetData(size_t fieldnum, const char * data, size_t& length);
//replaces length with the length actually stored.

   bool GetFieldData(const char * fieldname,char * destination,size_t maxcopy=MAX_FIELD);
   bool GetFieldData(size_t fieldnum,char * destination,size_t maxcopy=MAX_FIELD);
   bool SetFieldData(const char * fieldname,const char * source,size_t maxcopy=MAX_FIELD);
   bool SetFieldData(size_t fieldnum,const char * source,size_t maxcopy=MAX_FIELD);
   bool GetFieldData(const char * fname,TStr& dest)
     {
      dest.Resize(FieldLength(fname));
      return GetFieldData(fname,(char*)dest);
     }

   Stream* GetBlob(size_t fieldnum,TStr& filetype,bool ReadOnly = true);
   //use the filetype to determine the type. If this is a new blob,
   //set filetype to type desired extension (or blank for BIN)

   bool CommitChanges();

//utility functions
   bool CopyRecord(DBFRecord& recorddata, bool BinaryCopy = false);
   void operator = (TNameValueList & o);
   void GetData(TNameValueList & o);

protected:
//   friend TDBBrowser;
   FRIEND DBFFile;
  };

class DBFCache
 {
protected:
  TChars Storage;
public:
  enum TCacheMode {ReadForward,ReadBackwards,ReadRandom} CacheMode;

  int32 LastUsed; //seconds counter

  count_t LoInMem, HiInMem, MaxAvail;

  DBFCache();
  ~DBFCache();
  void SetBufferSize(DBFFile * db,EDBCacheSize CacheSize);
  //forces reloading the database, too.
protected:
  bool ReloadData(DBFFile * db);
  bool ReloadRecord(DBFFile * db,count_t CaseNum);

  bool NeedsReloading(); //checks the time
  bool HasRecord(count_t CaseNum);

  char * RecordData(DBFFile * db,count_t CaseNum);

  count_t CacheSize() {return MaxAvail;}
  //how many records can it hold?
  bool Load(DBFFile * db);

  void Clear();
  bool IsEmpty() {return (HiInMem==0);}
  FRIEND          DBFFile;
 };

class DBFFile
 {
protected:
  TPointer<DBFBaseIO> db;
#ifndef NO_DBF_ENCRYPTION
  TPointer<DBFCrypt> crypt;
#endif
  TPointer<DBFCache> cache;

public:
//read these fields directly

  bool HasChevron, HasPassword, HasUsername, HasBookmark, HasOwnPassword;
  //  175 = Chevron   174 = Password  173 = Username   172 = Bookmark
  //  0xAF            0xAE            0xAD             0xAC
  //  171 = OwnPassword
  //  0xAB

  //constructor, destructor
  DBFFile(DBFBaseIO * Db,DBFCache * cache);
  ~DBFFile();

//read record data into an existing object
  bool ReadRecordData(TParameterList & RecordData, count_t & CaseNum);
  bool WriteRecordData(TParameterList & RecordData, count_t & CaseNum);

  void SetCacheSize(EDBCacheSize CacheSize);

//functions to find out about the database
  bool ReplaceAllowed();
  bool AppendAllowed();
  bool ReadAllowed();
  bool IsValid();

  count_t RecordCount() ;
  count_t LastCase() {return RecordCount();}

  size_t RecordLength();
  size_t FieldCount();

  DBFFieldRec* GetFieldInfo(const char * name);
  DBFFieldRec* GetFieldInfo(size_t fieldnum);

     ///used for dumb reads and writes
  bool ReadCaseRawBuf(count_t CaseNum,char * outbuf); //doesn't decrypt
     ///used for dumb reads and writes
  bool WriteCaseRawBuf(count_t CaseNum,const char * inbuf);

     ///uses a cache to store data
  bool GetDataC(count_t CaseNum, size_t fieldnum,TChars& data);

     /*! caller figures out the size from Recordlength()
     returns true if you have the wrong password, but
     sets PasswordOK to false; If that is set, you aren't allowed
     to save the data, though the data will be kept.
     */

  bool ReadCaseData(count_t CaseNum,char * outbuf, bool & PasswordOK);
  count_t UpdateCase(count_t CaseNum, char* Changes, char* Backup=0);
     //returns the case number. If Backup is 0, writes straight through.

  char* GetFieldName(size_t fieldnum);
  size_t GetFieldNum(const char * fieldname); //returns NOT_FOUND on error

  size_t FindFieldBySequence(uint16 SeqNum);
  void SetSequenceNumber(uint16 SeqNum, const char * fieldname);
  void SetSequenceNumber(uint16 SeqNum, size_t fieldnum);
  void SetQuestType(int16 QuestType, const char * fieldname);
  void SetQuestType(int16 QuestType, size_t fieldnum);
  const char * filename();
  DBFBaseIO * DataFile() {return db;}

//@name call this if IsEncrypted is true.
//@{
  void SetPasswords(char * passwords);
  void SetPasswords(TStringList & passwords);
  void SetPasswords(int16 Password, int16 Master);
//@}

//@name delete/undelete/lock records
//@{
  EDBDeleteSetting SetDeleteFlag(count_t number,  EDBDeleteSetting) ;
  EDBDeleteSetting GetDeleteFlag(count_t number);
  EDBDeleteSetting SetDeleteFlag(char * c,  EDBDeleteSetting) ;
  EDBDeleteSetting GetDeleteFlag(char * c);
//@}

///find out about individual records. mostly an internal-use function
  bool IsCached(count_t CaseNum);

//utility functions
  ///clears caches and reloads DB header info, etc...
  void ReloadFile();

  ///writes number of cases into the file header

  bool WriteSummaryInformation();

protected:
//database access functions
     ///returns number of last record read
  count_t ReadCaseRange(count_t start, count_t count, char * outbuf,count_t maxcopy);

     /*!
      Returns number for first record saved. All else are contiguous
      To get the actual data, use ReadCaseData and SaveCaseData.
     */
  count_t SaveCaseRange(count_t start, count_t count, const char * inbuf,count_t maxcopy);

  count_t SaveCaseData(count_t CaseNum,const char * inbuf);
//translation functions
  bool DecryptRecord(char * CaseBuf);
  bool EncryptRecord(char * CaseBuf);

//friends
  FRIEND DBFCache;
  FRIEND DBFRecord;
    } ;

//Global functions

//@name  file read/write functions
//@{
long LongPntrWrite(const char * fname,long &FileOffset, long ByteCount, void  * Mem, bool lockrgn);

long LongPntrRead(const char * fname,long &FileOffset, long ByteCount, void  * Mem);
//@}
#ifdef OWL_WINDOW_H
bool PasswordCheck(DBFFile * db, TWindow * parent);
#endif

#endif
