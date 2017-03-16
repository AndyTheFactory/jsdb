#include "rslib.h"
#pragma hdrstop

// form functions
EZFForm::EZFForm():
Filename(""),
Options(),
Questions(),
Styles()
 {
 }

EZFForm::~EZFForm()
 {
 }
       /*
void getstr(TStr& out, const char * s)
{
 if (HIWORD(s)) out = s;
#ifdef __BORLANDC__
 else out = LOADSTRING(LOWORD(s));
#else
 out = itos((int)(void*)s);
#endif
}      */

bool TEZSForm::IsPageStart(size_t question)
{
 if (question == 0) return true;

 TEZSQuestion * q = Questions[question];
 if (q == NULL) return false;

 if (q->Type == TEZSQuestion::PageBreak) return true;
 if (q->Type == TEZSQuestion::Section) return true;

// TEZSQuestion * q1 =Questions[question-1];

// if (q->Type == TEZSQuestion::Section &&
// (q1->Type != TEZSQuestion::PageBreak && q1->Type != TEZSQuestion::Section))
// return true;

 if (q->Options.GetInt("PAGE")) return true;

 return false;
}

void TEZSForm::LoadQuestions(const REZSQuestion* s,size_t count)
{
 while (count && s->Type)
 {
  AddQuestion(s->Type,s->fname,s->text,s->desc,s->responses,s->length,s->options);
  s++;
  count--;
 }
}

TEZSQuestion *TEZSForm::AddQuestion( TEZSQuestion::EType Type,
                   const char * fname,
                   const char *text,
                   const char *desc,
                   const char * responses,
                   int length,
                   const char * options,
                   int before)
{
  TEZSQuestion * q;
  q = FindQuestion(fname);
  if (!q)
   {
    q = new TEZSQuestion();
    int pos = Questions.Add(q);
    if (before) Questions.Move(pos,before-1);
   }
  q->FieldName = fname; //HIWORD(s->fname) ? s->fname : (char*)LOADDLLSTRING(LOWORD(s->fname));
  if (text) q->QuesText = text;
  if (desc) q->Description = desc;

  q->Type = Type;
  //if (length == 0 && Type == TEZSQuestion::Text) length = 50;

  TStr temp;

  if (responses)
   {
    q->Responses.Read(responses,'|');
   }

  if (options)
   {
    q->Options.Read(options,'|');
   }

  q->Length = length;
  if (q->Length < 0) q->CalcLength();
  return q;
}

// question functions
void TEZSQuestion::CalcLength()
{
  if (Type == Number) { if (Length <= 0) Length = 10;}
  else if (Type == Date) {if (Length <= 0)Length = 12;}
  else if (Type == Time) {if (Length <= 0)Length = 12;}
  else if (Type == Password) {if (Length <= 0)Length = 12;}
  else if (!HasData()) Length = 0;
  else if (!HasResponses())
   {
    if (Length <= 0) Length = 60; //default one line of text
   }
  else
   {
    int oldlength = Length;
    Length =0;
    FOREACH(TNameValuePair * n,Responses)
     if (n->Name == "")
      {
       char s[10]; itoa(i,s,10);
       n->Name = s;
      }
     if (IsMultiSelect() )
       {
        Length  += strlen(n->Name);
        Length  ++;
       }
     else
       {
        Length = max((int)Length,(int)strlen(n->Name));
       }
//         dspMessageBox(n->Name,itos(Length));
    DONEFOREACH
    if (Type == ListCombo) Length = max(Length,oldlength);
   }
  if (Length<0) Length = 1;
}

TEZSQuestion::EType ConvertQuestionType(EQuestType e)
{
 switch (e)
  {
      case eqt_SingleResponse:      return TEZSQuestion::Radio;
      case eqt_Weighted:            return TEZSQuestion::Weighted;
      case eqt_CheckAll:            return TEZSQuestion::Checkbox;

      case eqt_Writein:
      case eqt_WriteinMulti:
      case eqt_WriteinPassword:     return TEZSQuestion::Comment;
      case eqt_Number:
      case eqt_NumberOnly:          return TEZSQuestion::Number;

      case eqt_ListBox:             return TEZSQuestion::ListSingle;
      case eqt_ListBoxMulti:
      case eqt_ComboBox:
      case eqt_ListBoxRanked:       return TEZSQuestion::ListMulti;

      case eqt_Date:
      case eqt_DateFirst:
      case eqt_DateLast:
      case eqt_DateStop:            return TEZSQuestion::Date;

      case eqt_Time:
      case eqt_TimeFirst:
      case eqt_TimeLast:
      case eqt_TimeStop:            return TEZSQuestion::Time;

      case eqt_TimeElapsedFirst:
      case eqt_TimeElapsedLast:
      case eqt_TimeElapsedTotal:    return TEZSQuestion::Number;

      case eqt_Hidden:              return TEZSQuestion::Hidden;
      case eqt_PageHeading:         return TEZSQuestion::Section;
      case eqt_Image:               return TEZSQuestion::Image;
      case eqt_RichText:            return TEZSQuestion::RichText;
      default:                      return TEZSQuestion::Text;
  }
}

void TEZSForm::Reset()
 {
  Options.Clear();
  Questions.Flush();
  Filename=0;
  IsModified = false;
 }

TEZSQuestion * TEZSForm::FindQuestion(const char * fieldname)
 {
   TEZSQuestion * q;
   FOREACH(q,Questions)
     if (q->FieldName == fieldname) return q;
   DONEFOREACH
   return 0;
 }

void TEZSForm::operator = (const TEZSForm& o)
  {
   IsModified = true;
   Filename = o.Filename;
   Options = o.Options;
   FOREACH(TEZSQuestion* q, o.Questions)
      TEZSQuestion * n = new TEZSQuestion;
      *n=*q;
      Questions.Add(n);
   DONEFOREACH
  }


//template instance generation

//TRow<TEZSQuestion> _trowtezsquestion;
