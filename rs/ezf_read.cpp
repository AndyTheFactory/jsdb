#include "rslib.h"
#pragma hdrstop

/**********************************
*       open files                *
**********************************/

char * FixString(char * str)
{
     str = StripCharsFB(str," \t\r\n");
     Replace(str,'\t',' ');
     Replace(str,'\r',' ');
     Replace(str,'\n',' ');
     return StripCharsFB(str," ");
}

void CopyOptions(TParameterList& in, TParameterList& out)
{
 TNameValuePair * n;
 FOREACH(n,in)
   if (n->Name == "NAME") continue;
   if (n->Name == "TITLE") continue;
   if (n->Name == "SIZE") continue;
   if (n->Name == "LENGTH") continue;
   if (n->Name == "MAXLENGTH") continue;
   if (n->Name == "TYPE") continue;
   if (n->Name == "EZT") continue;
   if (n->Name == "MULTIPLE") continue;

   //rename fields to be compatible with EFS
   if (n->Name == "MASK") n->Name = "pdx:MASK";

   out.Set(n->Name,n->Value);
 DONEFOREACH
 }

/*void ANSItoUTF8(TStr& x)
{
 TChars data(MultiByteToWideChar(CP_ACP,0,x,strlen(x),NULL,0) * sizeof(uint16));
 MultiByteToWideChar(CP_ACP,0,x,strlen(x),(wchar_t*)data.w(),data.size/sizeof(uint16));
 x = TStr(data.w());
 */
#define DECODE(x) {if (UsesURLEncoding) URLDecode(x,true); else HTMLUnEscape(x); \
if (UsesANSIEncoding) x.ANSItoUTF8();}

void Decode(TParameterList& list, bool url, bool ANSI)
{
 FOREACH(TNameValuePair*n, list)
  if (url) URLDecode(n->Value,true);
  else HTMLUnEscape(n->Value);
  if (ANSI) n->Value.ANSItoUTF8();
 DONEFOREACH
}

bool ReadHyperTextForm(Stream& in,TEZSForm& Form,bool ,RSCallback Callback,void*v)
{
 if (*Callback) Callback(v,0,0,"Reading Form");

 TEZSQuestion * q = 0;
 TStr * NextText = 0;
 char * str;
 bool EndTag;

 MemoryStream qtext,utext;
 TParameterList Options;
 TStr Tag,OpName;
 TStringList MinorTags; //("B|/B|I|/I|U|/U|IMG|A|/A","|");
 //skip all table data
 TStringList SkipTags("TABLE|/TABLE|TR|/TR|TD|/TD","|");

 int streamsize = in.size();
 bool UsesURLEncoding = false;
 bool UsesANSIEncoding = false;

 /*if (!IsSS0)*/
 while (Tag != "FORM")
   {
    int c = in.StartTag(Tag,0);
    if (c == EOF) return false;
    if (Tag == "FORM")
     {
      if (!in.FinishTag(c,&Form.Options,0,false)) return false;
      if (Form.Options.GetInt("version") < 2002)
        UsesURLEncoding = true;
      if (Form.Options.GetInt("version") < 2003)
        UsesANSIEncoding = true;

      XMLStream* s = TYPESAFE_DOWNCAST(&in,XMLStream);
      if (s)
       if (s->system == "http://www.electronicform.org/efs.dtd")
        UsesURLEncoding = false;

      Decode(Form.Options,UsesURLEncoding,UsesANSIEncoding);
     }
    else in.FinishTag(c,0); //don't care -- keep reading
   }

 qtext.Clear();

 //const char * db = GetFilename((char*)Form.Options["DATABASE"]);

 TStr Title("Reading ",in.filename());

// int i = 0;

 int result;

 while ((result = in.StartTag(Tag,&qtext,&EndTag))!=EOF)
 {
  if (*Callback) if (!Callback(v,in.pos(),streamsize,Title)) break;

  if (Tag == "!--")
   {
    if (!in.ReadUntilWord("-->")) break;
//    if (!FinishTag(in,result,0)) break;
    continue;
   }

  if (Tag == "TEXT") // XML only
   {
    if (!in.FinishTag(result,0)) break;
    qtext.Clear();
    if (!in.ReadUntilWord("</TEXT>",&qtext)) return false;
     //file accidentally truncated

     TStr x(qtext);
    DECODE(x)
//    URLDecode(qtext,true);

    if (q) q->QuesText = StripCharsFB(x," \t\r\n");
    NextText = 0;
    continue;
   }

  if (Tag == "HELP") // XML only
   {
    if (!in.FinishTag(result,0)) break;
    qtext.Clear();
    if (!in.ReadUntilWord("</HELP>",&qtext)) return false;
     //file accidentally truncated

     TStr x((char*)qtext);
    DECODE(x);
//    URLDecode(qtext,true);

    if (q) q->HelpText = StripCharsFB(x," \t\r\n");
    NextText = 0;
    continue;
   }
  Options.Clear();

  if (!in.FinishTag(result,&Options,&EndTag,false)) break;
  Decode(Options,UsesURLEncoding,UsesANSIEncoding);

  if (SkipTags.Has(Tag))
   {
    continue;
    //delete the tag and continue
   }

  if (NextText) //read any text immediately following the <INPUT> tag
   {
     (*NextText) += StripCharsFB(qtext," \t\r\n");

     qtext.Clear();
   }

  // Are we currently working with a question?
  if (NextText==0 && q && *OpName) //usually from "/OPTION" tags
   {
     str = FixString(qtext);
     if (!*str) str = OpName;
     TStr x(str);
     DECODE(x);
//     HTMLUnEscape(str);
     q->Responses.Set(OpName,x);
   }

  if (Tag == "/FORM")
    {
     break;
    }

  if (Tag == "H1" || Tag == "H2" || Tag == "H3" || Tag == "H4" || Tag == "H5")
   {
     q = new TEZSQuestion;
     Form.Questions.Add(q);
     q->Type = TEZSQuestion::PlainText;
     NextText = &q->QuesText; //HTML only -- read in section names
     OpName = 0;
     qtext.Clear();
     q=0;
     continue;
   }

  if (Tag == "/OPTION")
   {
     OpName = 0;
     qtext.Clear();
     continue;
   }

  if (Tag == "STYLE")
    {
     TParameterList * p = new TParameterList;
     *p = Options;
     Form.Styles.Add(p);
     continue;
    }

  if (Tag == "P" ||
      Tag == "/INPUT" ||
      Tag == "/SELECT" ||
      Tag == "/PAGE")
   { //resets everything
     NextText = 0;
    // if (q) q->UnusedTags = utext;
     q=0;
     OpName = 0;
     utext.Clear();
     qtext.Clear();
     continue;
   }

  if (q && Tag == "OPTION")
    {
     NextText = 0;
     OpName = FixString(Options["VALUE"]);
     if (OpName == "_") OpName = 0;
     qtext.Clear();

     //for SS0 files
     continue;
    }

  if (q && Tag == "SAMEAS" && Options.Count())
   {
    OpName = 0;
    qtext.Clear();

    TEZSQuestion * oldq = Form.FindQuestion(Options[(size_t)0]->Name);
    q->Responses = oldq->Responses;
    continue;
   }

  if (q && Tag == "SKIPS")
   {
     OpName = 0;
     qtext.Clear();

    q->Skips = Options;
   }

  if (Tag == "INPUT" ||
      Tag == "SELECT" ||
      Tag == "TEXTAREA" ||
      Tag == "PAGE" ||
      Tag == "FIELDSET"||
      Tag == "SECTION"  )
    {
     NextText = 0;
   //  if (q) q->UnusedTags = utext;
     OpName = 0;
     utext.Clear();
     qtext.Clear();

     char * fname = FixString(Options["NAME"]);
     if (!*fname) continue;

     char * respname = 0;

     if (fname[0] == '\\') //escape the fieldname
       fname++;
     else
       respname = strchr(fname,'_');

     if (respname)
       {
        *respname=0;
        respname++;
       }

     //for multiple-response questions

     //special input types
     if (strcasecmp(fname,"DATABASE") ==0)
       {
        Form.Options.Set("DATABASE",Options["VALUE"]);
        continue;
       }

     char * type = FixString(Options.Get("EZT",Options["TYPE"]));

     if (strcasecmp(type,"SUBMIT")==0) continue;
     if (strcasecmp(type,"RESET")==0) continue;

     //end input types

     if (respname && *respname)  OpName = respname;
     else                        OpName = FixString(Options["VALUE"]);
     //HTML, radio and check boxes. Not applicable for other types.
     //Adds the first option when reading HTML files

     q = Form.FindQuestion(fname);

     if (q)
      { //oops, reading HTML. Don't want to reload the question!
       qtext.Clear();
       continue;
      }
     //need a new question

     q = new EZFQuestion;
     Form.Questions.Add(q);
     q->FieldName = fname; //FixString(Options["NAME"]);

     q->Description = FixString(Options["TITLE"]);
     q->QuesText = FixString(Options["TEXT"]);
     q->Length = Options.GetInt("MAXLENGTH",0);
     if (q->Length <= 0) q->Length = Options.GetInt("LENGTH");
     if (q->Length <= 0) q->Length = Options.GetInt("SIZE");

     //important! Old EZS used SIZE=(1,0,-1,-2) to change the height of
     //write-in input boxes, but wrote the field length as LENGTH.
     //new EZS writes the field length as SIZE, and the box height as
     //HTML.SIZE

     CopyOptions(Options,q->Options);

     char * c =  StripCharsFB(qtext," \t\r\n");
     if (!strlen(q->QuesText))
     {
      if (strlen(c)) q->QuesText = c; //HTML
      else NextText = &q->QuesText; //not HTML
     }
     else NextText = 0;

     qtext.Clear();

     if (Tag == "PAGE")
     {
       q->Type = TEZSQuestion::PageBreak;
     }
     else if (Tag == "SECTION")
     {
       q->Type = TEZSQuestion::Section;
     }
     else if (Tag == "FIELDSET")
     {
       q->Type = TEZSQuestion::Section;
     }
     else if (Tag == "SELECT")
     {
       if (Options.Has("MULTIPLE")) q->Type = TEZSQuestion::ListMulti;
       else q->Type = TEZSQuestion::ListSingle;
     }
     else if (*type                     ==0) q->Type=TEZSQuestion::Comment;
     else if (strcasecmp(type,"N")         ==0) q->Type=TEZSQuestion::Number;
     else if (strcasecmp(type,"NUMBER")    ==0) q->Type=TEZSQuestion::Number;
     else if (strcasecmp(type,"R")         ==0) q->Type=TEZSQuestion::Radio;
     else if (strcasecmp(type,"RADIO")     ==0) q->Type=TEZSQuestion::Radio;
     else if (strcasecmp(type,"C")         ==0) q->Type=TEZSQuestion::Checkbox;
     else if (strcasecmp(type,"CHECKBOX")  ==0) q->Type=TEZSQuestion::Checkbox;
     else if (strcasecmp(type,"LC")        ==0) q->Type=TEZSQuestion::ListCombo;
     else if (strcasecmp(type,"LISTCOMBO") ==0) q->Type=TEZSQuestion::ListCombo;
     else if (strcasecmp(type,"LS")        ==0) q->Type=TEZSQuestion::ListSingle;
     else if (strcasecmp(type,"LISTSINGLE")==0) q->Type=TEZSQuestion::ListSingle;
     else if (strcasecmp(type,"SINGLE")==0) q->Type=TEZSQuestion::ListSingle;
     else if (strcasecmp(type,"LM")        ==0) q->Type=TEZSQuestion::ListMulti;
     else if (strcasecmp(type,"LISTMULTI") ==0) q->Type=TEZSQuestion::ListMulti;
     else if (strcasecmp(type,"MULTIPLE") ==0) q->Type=TEZSQuestion::ListMulti;
     else if (strcasecmp(type,"W")         ==0) q->Type=TEZSQuestion::Weighted;
     else if (strcasecmp(type,"WEIGHTED")  ==0) q->Type=TEZSQuestion::Weighted;
     else if (strcasecmp(type,"LR")        ==0) q->Type=TEZSQuestion::ListRank;
     else if (strcasecmp(type,"LISTRANK")  ==0) q->Type=TEZSQuestion::ListRank;
     else if (strcasecmp(type,"HIDDEN")    ==0) q->Type=TEZSQuestion::Hidden;
     else if (strcasecmp(type,"D")         ==0) q->Type=TEZSQuestion::Date;
     else if (strcasecmp(type,"DATE")      ==0) q->Type=TEZSQuestion::Date;
     else if (strcasecmp(type,"T")         ==0) q->Type=TEZSQuestion::Time;
     else if (strcasecmp(type,"TIME")      ==0) q->Type=TEZSQuestion::Time;
     else if (strcasecmp(type,"RT")        ==0) q->Type=TEZSQuestion::RichText;
     else if (strcasecmp(type,"FIELDSET")  ==0) q->Type=TEZSQuestion::Section;
     else if (strcasecmp(type,"SECTION")   ==0) q->Type=TEZSQuestion::Section;
     else if (strcasecmp(type,"RICHTEXT")  ==0) q->Type=TEZSQuestion::RichText;
     else if (strcasecmp(type,"IMAGE")     ==0) q->Type=TEZSQuestion::Image;
     else if (strcasecmp(type,"TB")        ==0) q->Type=TEZSQuestion::ToolButton;
     else if (strcasecmp(type,"BUTTON")    ==0) q->Type=TEZSQuestion::ToolButton;
     else if (strcasecmp(type,"PAGE")      ==0) q->Type=TEZSQuestion::PageBreak;
     else if (strcasecmp(type,"P")         ==0) q->Type=TEZSQuestion::PageBreak;
     else if (strcasecmp(type,"PT")        ==0) q->Type=TEZSQuestion::PlainText;
     else if (strcasecmp(type,"PLAINTEXT") ==0) q->Type=TEZSQuestion::PlainText;
     else if (strcasecmp(type,"PASSWORD")  ==0) q->Type=TEZSQuestion::Password;
     else
      {
       q->Type = TEZSQuestion::Comment;
      }

     if (strcasecmp(type,"RADIO") && strcasecmp(type,"CHECKBOX"))
         OpName = 0; //not a radio or checkbox in a HTML file
         //(EZF files use abbreviated types)
     continue;
    }

    if (NextText || *OpName)
      qtext.WriteTag(Tag,Options,EndTag);
      // part of question or response text
      // this allows <IMG> etc... tags to appear in responses
    else if (q)
     {
      qtext.rewind();
      utext.Append(qtext);
      utext.WriteTag(Tag,Options,EndTag);
     }
      // we didn't care about the tag, so save it as extra XML
 }
 FOREACH(q,Form.Questions)
  q->CalcLength();
//  dspMessageBox(q->FieldName,itos(q->Length));
 DONEFOREACH

 if (*Callback) Callback(v,streamsize,streamsize,"Done.");
 return true;
}

bool OpenEZFFile(Stream&in, EZFForm& out,RSCallback E,void*v)
{
 return ReadHyperTextForm(in,out,false,E,v);
}

