#ifndef  _RS_FORMTYPE_H
#define  _RS_FORMTYPE_H

/*
enum EDBType {edbt_Unknown,
              edbt_dBase,    // dbf:// or file://path/filename.dbf
              edbt_xBase = edbt_dBase,
              edbt_ASCII,    // ascii:// or file://path/filename.txt
              edbt_ASCIIDOS = edbt_ASCII,
              edbt_ASCIIUNIX,
              edbt_ASCIIMAC,
              edbt_Inifile,
              edbt_ParamList,
              edbt_Sort,
              edbt_Notes,    // notes://user:password@[server]/table
              edbt_ODBC,     // odbc://user:password@service/table
              edbt_SQL = edbt_ODBC,
              edbt_Access,
              edbc_MSSQL,
              edbt_Oracle,
              edbt_Sybase};
*/

enum EDBFieldType {db_ft_NoData,       //data types
                   db_ft_Virtual= 'V', //on-the-fly
                   db_ft_Date   = 'D', //date or datetime
                   db_ft_Time   = 'T', //time
                   db_ft_Number = 'N', //number
                   db_ft_Mult   = 'C', //not used
                   db_ft_Char   = 'C', //character
                   db_ft_Memo   = 'M', //text memo (unused)
                   db_ft_Blob   = 'B', //binary data
                   db_ft_Other  = 'C'};//treat all as char

enum EDBDeleteSetting {db_Unknown,
                       db_OK,
                       db_CheckDelete,
                       db_Deleted,
                       db_Locked,
                       db_Encrypted};

enum EDBCacheSize { CACHE_NONE,
                    CACHE_TINY,
                    CACHE_SMALL,
                    CACHE_BIG,
                    CACHE_HUGE,
                    CACHE_DEFAULT=CACHE_BIG};


enum EToolButtonType
    {
      etbt_Next          = 1,
      etbt_Previous      = 2,
      etbt_Ok            = 3,
      etbt_Save          = 4,
      etbt_Print         = 5,
      etbt_Cancel        = 6,
      etbt_Abort         = 7,
      etbt_Top           = 8,
      etbt_Bottom        = 9,
      etbt_Comment       = 10,
    };


#define QTypeInRange(x,range) \
  ((int)x >= (int)eqt_ ## range ## Start && (int)x <= (int)eqt_ ## range ## Stop)

enum EQuestType
    {
      eqt_None             = 0,

      eqt_DataStart        = 1,    // QTypeInRange(x,Data)

      eqt_ButtonStart      = 1,    // QTypeInRange(x,Button)
      eqt_SingleResponse   = 1,
      eqt_Weighted         = 2,
      eqt_CheckAll         = 3,
      eqt_ButtonStop       = 3,

      eqt_WriteInStart     = 4,     // QTypeInRange(x,WriteIn)
      eqt_Writein          = 4,
      eqt_WriteinMulti     = 5,
      eqt_Number           = 6,
      eqt_NumberOnly       = 7,
      eqt_WriteinPassword  = 8,
      eqt_WriteInStop      = 9,

      eqt_ListStart        = 10,    // QTypeInRange(x,List)
      eqt_ListBox          = 10,
      eqt_ListBoxMulti     = 11,
      eqt_ComboBox         = 12,
      eqt_ListBoxRanked    = 13,
      eqt_ListStop         = 14,

      eqt_DateStart        = 21,    // QTypeInRange(x,Date)
      eqt_Date             = 21,
      eqt_DateFirst        = 22,
      eqt_DateLast         = 23,
      eqt_DateStop         = 29,

      eqt_TimeStart        = 41,    // QTypeInRange(x,Time)
      eqt_Time             = 41,
      eqt_TimeFirst        = 42,
      eqt_TimeLast         = 43,
      eqt_TimeStop         = 49,

      eqt_TimeNumStart     = 44,    // QTypeInRange(x,TimeNum)
      eqt_TimeElapsedFirst = 44,
      eqt_TimeElapsedLast  = 45,
      eqt_TimeElapsedTotal = 46,
      eqt_TimeNumStop      = 46,

      eqt_Hidden           = 51,
      eqt_BlobStart        = 52,    // QTypeInRange(x,Blob)
      eqt_Image            = 52,
      eqt_RichText         = 53,
      eqt_BlobStop         = 53,

      eqt_Ask              = 62,
      eqt_Lst              = 63,

      eqt_CurCase          = 61,
      eqt_DataStop         = 100,

      eqt_ControlStart     = 101,   // QTypeInRange(x,Control)
      eqt_ButtonSkip       = 101,
      eqt_ButtonHelp       = 102,
      eqt_ButtonPopup      = 103,
      eqt_ButtonDial       = 104,
      eqt_ControlStop      = 110,

      eqt_PageBreak        = 150,
      eqt_SectionBreak     = 151,

      eqt_DecorationStart  = 161,   // QTypeInRange(x,Decoration)
      eqt_PlainText        = 162,
      eqt_PageHeading      = 163,
      eqt_Bitmap           = 165,
      eqt_DecorationStop   = 169,

      eqt_ButtonTool       = 200,
    };

#define   MAXFIELDWIDTH  10000

inline bool HasButtons(EQuestType t)
 { return ((t > eqt_None) && (t < eqt_Writein)); }


enum EFrmObjType
    {
      //  FrmObjType  as stored in .FRM file
      //
      //  < 100  are StreamBuf
      //  > 100  are TInts stored as Stream

      eft_None             = 0,
      eft_RespSingle       = 10,
      eft_RespWeighted     = 11,
      eft_RespCheckAll     = 12,
      eft_RespListBox      = 13,
      eft_RespComboBox     = 14,

      eft_Quest            = 30,
      eft_QuestLocked      = 31,     // used by GoForm
      eft_QuestCalc        = 32,

      eft_PageElect        = 41,
      eft_PagePaper        = 42,

      eft_Help             = 51,
      eft_Bitmap           = 52,

      eft_RotatePage       = 61,

      eft_PageOrder        = 101,
      eft_QuestFastEntry   = 102,
      eft_PageComment      = 103,

      eft_Password         = 201,
      eft_ParamList        = 205,

      eft_EOF              = 250,
    };

#endif
