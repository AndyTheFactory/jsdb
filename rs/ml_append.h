#ifndef _ML_APPEND_H
#define _ML_APPEND_H
  /*
struct DBFHeader_Rec
    { uint8 filetype;   //  3 = dbf   2 = paramlist
      uint8 year, month, day;
      uint32 cases;       //  GoForm uses to store FormNumber
      uint16 headerlength, reclength;
      uint32 Signature;    //  Hash value of Frm Signature
      int8 reserved[6];
      uint32  OwnPassword;
      struct {
               UINT  MailSent          : 1;  //  mail sent back already
               UINT  RecordSaved       : 1;  //  Saved may not be sent
               UINT  RotatePageBlock   : 1;
               UINT  Reserved          : 13;
             } Flags;
      uint32 Time;
    };
     */
   /*(
struct RDBFField
    { char name[11];
      char fieldtype;
      int32 dataaddr; //for memo field
      uint8 length;
      uint8 decimalcount;
      uint16 FieldLength; //  if length == 0 length = FieldLength
      uint8 reserved[12];
   } ;  */
            /*
struct  RAppendDatRec
   {
     char  * DatBuf;

     RAppendDatRec()
        {
          DatBuf = 0;
          DatFileName[0] = 0;
        }

     ~RAppendDatRec()
        {
          Close();
        }

     char  DatFileName[300];
     long  LastCase, RecordLen, HeaderLen;
     int   HasChevron;

     DBFHeader_Rec Header;
     TParameterList FieldList;
     TParameterList HashList;

     void  Open(const char * FileName);
     void  Close();
     void  DatListToDatbuf(TParameterList & List);
     void  AppendDatFile(const char * FileName);
     long  AppendRecord(); //  return last case number or 0
     long  ReplaceRecord(long CurCase); //  return CurCase or 0

     int   IsOpen() {return DatFileName[0] != 0;}

   };


  */

#endif