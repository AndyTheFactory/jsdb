#include "rslib.h"
#pragma hdrstop

/**********************************
*       Save to EZS file          *
**********************************/
void SaveEZFFile(EZFForm& Form,Stream & out,RSCallback CB,void*v)
{
 EZFQuestion * q;
 TParameterList* s;
 TNameValuePair * n;

 Form.Options.Set("version","2007");
 out.WriteTag("FORM",Form.Options);
 out << "\n";

 bool FieldSet = false;

 FOREACHPTR(s,Form.Styles)
  out.WriteTag("STYLE",*s,true);
 DONEFOREACH

 FOREACHPTR(q,Form.Questions)
 const char * c="", *type="INPUT", *options="", *datatype = 0;

 bool SaveResponses = true;
 if (CB) CB(v,i,imax,"Saving");

 q->CalcLength(); //always do this when saving

   switch (q->Type)
   {
    case eqt_NumberOnly:
    case TEZSQuestion::Number:     c = "N"; datatype="TEXT"; break;

    case TEZSQuestion::Radio:      c = "R"; datatype="SINGLE"; break;
    case TEZSQuestion::Checkbox:   c = "C"; datatype="MULTIPLE"; break;
    case TEZSQuestion::ListRank:   c = "LR"; datatype="MULTIPLE"; break;
    case TEZSQuestion::ListMulti:  c = "LM"; datatype="MULTIPLE";  break;
    case TEZSQuestion::ListSingle: c = "LS"; datatype="SINGLE";  break;
    case TEZSQuestion::ListCombo:  c = "LC"; datatype="TEXT"; break;
    case TEZSQuestion::Weighted:   c = "W"; datatype="SINGLE"; break;
    case TEZSQuestion::Hidden:     c = "HIDDEN"; datatype="TEXT"; break;

    case TEZSQuestion::Section:    if (FieldSet) out << "</FIELDSET>";
                                   type = "FIELDSET";
                                   SaveResponses = false;
                                   FieldSet = true;
                                   break;

    case TEZSQuestion::RichText:   c = "RT"; break;
    case TEZSQuestion::Image:      c = "IMAGE"; break;
    case TEZSQuestion::ToolButton: c = "TB"; break;
    case TEZSQuestion::PlainText:  c = "PT"; break;
    case TEZSQuestion::PageBreak:  type = "PAGE"; SaveResponses = false; break;
    case TEZSQuestion::Password:   c = "PASSWORD"; datatype="TEXT";

    default:
      if (QTypeInRange(q->Type,TimeNum))
       {
        c = "N"; datatype="TIME";
       }
      else if (QTypeInRange(q->Type,Time))
       {
        c = "T"; datatype="TEXT";
       }
      else if (QTypeInRange(q->Type,Date))
       {
        c = "D"; datatype="TEXT";
       }
   }

   if (datatype) q->Options.Set("DATA",datatype);
   else q->Options.Unset("DATA");
   
   out << "<" << type << options;
   if (*q->FieldName)
    {
      out << " NAME=\"";
      if (strchr(q->FieldName,'_')) out << "\\";
      out << q->FieldName << "\"";
    }

   if (c && *c)
      out << " TYPE=\"" << c << "\"" ;

   if (*q->Description)
      out << " TITLE=\"" << q->Description << "\"" ;

   if (q->Length)
    {char s[32]; itoa(q->Length,s,10);
     out << " SIZE=\"" << s << "\"";
    }

   FOREACH(n,q->Options)
    if (n->Name == "SEQ") continue;
    out << "\n" << n->Name << "=\"";
    //TStr s; URLEncode(s,n->Value,0,true);  out << s;
    HTMLEscape(out,n->Value);
    out << "\"";
   DONEFOREACH

   out << ">" << "\n";

//   TStr temp;

   q->QuesText.replace("<TEXT>","");
   q->QuesText.replace("</TEXT>","");
  // URLEncode(temp,q->QuesText,0,true);
  // if (temp.length())
   out << "<TEXT>";
   HTMLEscape(out,q->QuesText);
   out << "</TEXT>" "\n";

   q->HelpText.replace("<HELP>","");
   q->HelpText.replace("</HELP>","");
//   temp = 0;
//   URLEncode(temp,q->HelpText,0,true);
//   if (temp.length())
   out << "<HELP>";
   HTMLEscape(out,q->HelpText);
   out << "</HELP>" "\n";

   if (SaveResponses)
   {
    FOREACHPTR(n,q->Responses)
      if (*n->Name)
      {
//       temp = 0;
       out << "<OPTION VALUE=\"" << n->Name << "\">";
//       URLEncode(temp,n->Value,0,true);
       HTMLEscape(out,n->Value);
       out << "</OPTION>" "\n";
      }
    DONEFOREACH

   if (q->Skips.Count())
    {
     out.WriteTag("SKIPS",q->Skips,true);
    }

    }

   out << q->UnusedTags;

   if (q->Type != TEZSQuestion::Section)
    out << "</" << type << ">" "\n";
 //endtag

 DONEFOREACH

 if (FieldSet) out << "</FIELDSET>";

 out << "</FORM>" "\n";

 if (CB) CB(v,0,0,"Done");
// Form->Footer.seek(0);
// out.Append(Form->Footer);

}

