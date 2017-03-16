#ifndef _RS_EZF_H
#define _RS_EZF_H

#ifndef _RS_SYSTEM_H
#include "rs/system.h"
#endif

#include "rs/formtype.h"

#define EZSURVEY_VERSION 200

//#include "pshpack1.h"

#define TEZSQuestion EZFQuestion
struct EZFQuestion
{
  TStr FieldName, QuesText, Description, HelpText, UnusedTags;
  int Length;

  TParameterList Responses;
  TParameterList Options;

  TParameterList Skips; //code=dest,dest,dest,dest

  enum EType {None,
            Comment    = eqt_Writein,
            Text       = eqt_Writein,
            Password   = eqt_WriteinPassword,
            Date       = eqt_Date,
            Time       = eqt_Time,
            Number     = eqt_Number,
            Radio      = eqt_SingleResponse,
            Weighted   = eqt_Weighted,
            Checkbox   = eqt_CheckAll,
            Check      = Checkbox,
            ListSingle = eqt_ListBox,
            ListMulti  = eqt_ListBoxMulti,
            ListCombo  = eqt_ComboBox,
            ListRank   = eqt_ListBoxRanked,
            Hidden     = eqt_Hidden,
            Section    = eqt_SectionBreak,
            PageBreak  = eqt_PageBreak,
            RichText   = eqt_RichText,
            Image      = eqt_Image,
            PlainText  = eqt_PlainText,
            ToolButton = eqt_ButtonTool} Type;

  EZFQuestion() {Length = 240; Type=Comment;}

  void CalcLength();

  EZFQuestion& operator = (EZFQuestion& o)
   {
    Type       = o.Type;
    Length     = o.Length;
    Responses  =o.Responses;
    Options    =o.Options;
    Skips      =o.Skips;
    FieldName  =o.FieldName;
    QuesText   =o.QuesText;
    Description=o.Description;
    Length     =o.Length;
    return *this;
   }

  const char * Doc() {return *Description ? Description : QuesText; }
  bool HasData()
    {return (QTypeInRange(Type,Data));}
  bool HasLength()
    {return (QTypeInRange(Type,WriteIn) || Type == Hidden || Type == ListCombo);}
  bool HasResponses()
    {return (Type == Number || QTypeInRange(Type,Button) || QTypeInRange(Type,List));}
  bool HasButtons()
    {return (QTypeInRange(Type,Button));}
  bool IsWritein()
    {return (QTypeInRange(Type,WriteIn) || Type == ListCombo);}
  bool IsVisible()
    {return (Type != Hidden);}
  bool IsList()
    {return (QTypeInRange(Type,List));}
  bool IsNumeric()
    {return (Type == Weighted || Type == Number);}
  bool IsDateTime()
    {return (QTypeInRange(Type,Date) || QTypeInRange(Type,Time));}
  bool IsTime()
    {return (QTypeInRange(Type,Time));}
  bool IsDate()
    {return (QTypeInRange(Type,Date));}
  bool IsMultiSelect()
    {return (Type == Checkbox || Type == ListMulti || Type == ListRank);}
  bool IsBlob()
    {return (QTypeInRange(Type,Blob));}
  bool IsPageBreak()
    {return (Type == PageBreak);}
  bool NeedsUiSelect()
    {return (QTypeInRange(Type,Button) || QTypeInRange(Type,List));}
  //numbers can have responses corresponding to ranges
};

EZFQuestion::EType ConvertQuestionType(EQuestType e);

#define REZSQuestion EZFList
/// usage: EZFList Form[] = ...
struct EZFList
 {
  EZFQuestion::EType Type;
  const char * fname, *text, *desc;
   /// pipe-delimited
  const char * responses;
   /// pipe-delimited
  const char * options;
  /// -1 to calculate it, 0 for no restraints
  int length;
 };

#define TEZSForm EZFForm
struct EZFForm
{
 TStr Filename;
 TList<EZFQuestion> Questions;
 TList<TParameterList> Styles;
 TParameterList Options;
 bool IsModified;

 EZFForm();
 ~EZFForm();
 EZFForm(const EZFList* questions,size_t count=UINT_MAX)
  {LoadQuestions(questions,count);}

 void Reset();
 void LoadFile(Stream& file);
 void LoadQuestions(const EZFList* questions,size_t count=UINT_MAX);
 bool IsPageStart(size_t question);
 EZFQuestion * AddQuestion( EZFQuestion::EType Type,
                   const char * fname,
                   const char *text,
                   const char *desc=0,
                   const char * responses=0,
                   int length=-1, ///< auto-calculate
                   const char * options=0,
                   int before = 0);

 EZFQuestion * FindQuestion(const char * fieldname);
 void operator = (const TEZSForm& o);

 void GetStyle(TParameterList& out, EZFQuestion*q, const char * name);

 void Move(size_t a, size_t b) {Questions.Move(a,b);}
 void Remove(size_t a) {Questions.Destroy(a);}
};

///if (*Callback) if (!Callback(v,i,lastc,"Exporting")) break;
typedef bool (*RSCallback)(void*v,int i, int imax,const char * title);

void SaveEZFFile(EZFForm& Form,Stream & out,RSCallback E,void*v);

bool OpenEZFFile(Stream&in, EZFForm& out,RSCallback E,void*v);

//#include "poppack.h"
#endif


