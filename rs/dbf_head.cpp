#include "rslib.h"
#pragma hdrstop

#include <time.h>

//--------------------------------------------------
//
//  DBFHeader: generic header structure
//
//--------------------------------------------------

DBFHeader::DBFHeader(TType _Type)
{ RecordLength=0;
  HeaderSize=0;
  Type=_Type;
};

size_t DBFHeader::FieldCount()
{
 return fields.Count();
};

void DBFHeader::CalculateSizes()
{//calculate headersize and recordlength
 if (Type==dBase || Type==WEB) RecordLength = 1;
 size_t i,imax=fields.Count();
 DBFFieldRec *R;
 size_t fieldcount = 0;
 for (i=0;i<imax;i++)
  {
   R=fields[i];
   R->Offset=RecordLength;
   R->FieldNum=i+1;

   if (R->FieldType == db_ft_Virtual) continue;
   RecordLength+=R->FieldLength;
   fieldcount ++;
  }

 if (Type==ASCIIDOS) RecordLength+=2; //trailing CRLF
 else if (Type==ASCIIUNIX || Type==ASCIIMAC)
   RecordLength +=1; // trailing LF or CR
};

DBFHeader::~DBFHeader()
{
  fields.Flush();
};

void DBFHeader::Clear()
 {
  fields.Flush();
  RecordLength=0;
 }

bool DBFHeader::AddField(const char * name,uint16 length,
                EDBFieldType type,int32 memoaddr,int32 offset)
{
 DBFFieldRec * r;
 bool replacing;
 r = GetFieldInfo(name);
 if (!r)
   {
    r = new DBFFieldRec;
    replacing=false;
   }
   else replacing=true;

 r->FieldName=name;

 r->FieldLength=length;
 r->Offset=offset;
 r->FieldType=type;

 uint8 c = *name;

 if ( c >= 171 && c <= 176 )
      r->IsVisible=false;
      else
      r->IsVisible=true;

 r->MemoAddr=memoaddr;
 if (replacing == false)
   {
    fields.Add(r);
   }

 return true;
}

DBFFieldRec* DBFHeader::GetFieldInfo(const char * name)
{
 FOREACH(DBFFieldRec *R,fields)
  if (R && R->FieldName == name) return R;
 DONEFOREACH
 return 0;
}

DBFFieldRec* DBFHeader::GetFieldInfo(size_t fieldnum)
{
 return fieldnum ? fields[fieldnum-1] : 0;
};

bool DBFHeader::LoadFromFile(const char * filename)
{
//First, check that the file exists and that we can read it
if (!filename) return false; //that's OK. it's a preview instruction

 if (!FileAttributes(filename)) throw xdb("No file access","file",filename);
 Clear();
 FileName=filename;

 try {
 FileStream Data(FileName,Stream::OMBinary,Stream::ReadOnly);

 char header[32];
 Data.read(header,32);

 if (!strcmp(header,"DAT")) //web file
  {
   return LoadWEB(Data,header);
  }

 uint8 ft = header[0];
 bool IsOk = (ft == 3   || ft == 131 ||
              ft == 11  || ft == 139 ||
              ft == 117 || ft == 245);
 //  if (!IsOk || Header.month > 12 || Header.day > 31 )
     if (!IsOk )
     throw xdb("Not a web, dBase III, IV, or FoxBase file","file",filename,"id",ft);

 return LoadDBF(Data,header);
 } catch(...) {return false;}
}

bool DBFHeader::LoadWEB(Stream& Data,const char* header)
{
 RWebHead Header;
 memcpy(&Header,(void*)header,32);

 Type = WEB;
 int hlen = CharToInt(Header.HeaderLength);

 MemoryStream m;
 m.Append(Data,hlen);
 m.rewind();

 HeaderSize = hlen + 32;

 TParameterList Fieldnames;
 m.ReadPaired(Fieldnames,'|');

  FOREACH(TNameValuePair*n,Fieldnames)
   if (!*n->Name) continue;

   char * length = strchr(n->Value,','); // fieldname=offset,length
   if (!length) return false;
   *length++=0;

   AddField(n->Name,atoi(length),db_ft_Char,0,atoi(n->Value));
  DONEFOREACH

 CalculateSizes();
 return true;
}

bool DBFHeader::LoadDBF(Stream& Data,const char* header)
{
 RDBFHead Header;
 memcpy(&Header,(void*)header,32);

 //is the header size defined?
 if (Header.headerlength[0] != 0 ||
     Header.headerlength[1] !=0)
  {
    HeaderSize=Header.headerlength[0] + 256 * Header.headerlength[1];
    RecordLength = 1;
    long fieldcount = HeaderSize / 32 - 1;

    TAPointer<RDBFField> Fields(new RDBFField[fieldcount+1]);

    long j;

    j=Data.read(Fields,fieldcount*32);

    if (j < fieldcount*32)
        throw xdb("Header too short","file",FileName,"size",HeaderSize);

    int i;
    for (i=0; i < fieldcount; i++)
     {
     size_t length=Fields[i].length;
     if (Fields[i].fieldtype == 'C' && Fields[i].decimalcount)
       {
        length += Fields[i].decimalcount * 256; //for foxpro, clipper
       }

     if (length == 0)
       {
        length=(uint16)Fields[i].OldSwinFieldLength;
       }

     if (Fields[i].name[0] != db_dbf_HdrEndChar)
      {
       AddField(Fields[i].name,
                length,
                (EDBFieldType)Fields[i].fieldtype,
                Fields[i].dataaddr);
      }
      else throw xdb("Invalid filename","file",FileName,"field",i+1);
     //assume whoever wrote the header size knew what they were doing,
     //take it on faith (but recalculate recordlength)
     }
  } //read that many fields in.
 //if not, read 'em all in
 else
  {//read one at a time.
   //keep reading fields until a newline is encountered
   RDBFField Field;

   int fieldcount = 0;

   while (Data.read(&Field,32))
   {
   if (Field.name[0] == db_dbf_HdrEndChar) break;

   size_t length=Field.length;

   if (Field.fieldtype == 'C' && Field.decimalcount)
       {
        length += Field.decimalcount * 256;
       }

   if (length == 0)
       {
        length=(uint16)Field.OldSwinFieldLength;
       }

   AddField(Field.name,
               length,
               (EDBFieldType)Field.fieldtype,
               Field.dataaddr);
   fieldcount ++;
  }

  HeaderSize = ((fieldcount + 1) * 32 )+ 1;
  }
 CalculateSizes(); //calculates offsets
 return true;
}

bool DBFHeader::IsValidRecordLength()
   {
     if (fields.Count() > 16384)
          return  false;
     return true;
   }


bool DBFHeader::CreateFile(const char * pfname)
{
 CalculateSizes();

 if (!IsValidRecordLength()) return false;  // > 64K record

 if (pfname==0) return 0;

 RDBFHead Header;
 RDBFField Field;
 memset(&Field,0,32);

 FileName=pfname;
 if (FileExists(FileName)) return false; //file exists

 Header.filetype=3;
 {
   time_t timer;
   time(&timer);
   struct tm *tb;
   tb = localtime(&timer);

   Header.year=tb->tm_year;
   Header.month=tb->tm_mon;
   Header.day=tb->tm_mday;
   memset(Header.reserved,0,sizeof(Header.reserved));
   memset(Header.Time, 0,sizeof(Header.Time));
 }

 if (!HeaderSize)
  {
   HeaderSize = 1+ (fields.Count() +1)*32;
  }

   Header.cases=0;
   if (HeaderSize < 65535)
    {
     Header.headerlength[0] = HeaderSize % 256;
     Header.headerlength[1] = HeaderSize / 256;
    }
   else
   Header.headerlength[0]=Header.headerlength[1]=0;

   if (RecordLength < 65535)
    {
     Header.reclength[0] = RecordLength % 256;
     Header.reclength[1] = RecordLength / 256;
    }
   else
   Header.reclength[0]=Header.reclength[1]=0;
   //doesn't read these. Calculate each time

   memset(Header.reserved,0,sizeof(Header.reserved));

 try {

   FileStream file(FileName,
                  Stream::OMBinary,
                  Stream::WriteOnly);

    file.write((char*)(void*)&Header,32);

     FOREACH(DBFFieldRec * R,fields)
       if (R->FieldType == db_ft_Virtual) continue;

       memset(&Field,0,sizeof(Field));
       strncpy(Field.name,R->FieldName,10);

       Field.fieldtype=(char)R->FieldType;

       if (R->FieldLength > 255 && Field.fieldtype == 'C')
       {//that's how foxpro,clipper write their files
        Field.length          =   (R->FieldLength) & 0x00ff; //= 0; // LOBYTE
        Field.decimalcount    =   (R->FieldLength >> 8) & 0x00ff; //= 0; // HIBYTE
       }
       else
       {//that's how we normally do it
        Field.length             = min(R->FieldLength,255u);
        Field.decimalcount       = 0;
        Field.OldSwinFieldLength = 0;
       }

    file.write((char*)(void*)&Field,32);

    DONEFOREACH;
     char c= db_dbf_HdrEndChar;
    file.write(&c,1);
     return true;
    } catch(...)
     {
      return false;
    } //couldn't make the file
}


