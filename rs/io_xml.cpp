#include "rslib.h"
#pragma hdrstop

/*
char * SkipcChars(char * c, const char * skip)
{
 while (*c && strchr(skip,*c)) c++; return c;
}

bool NeedsQuote(const char * c)
{
 while (*c) if (isspace(*c) || *c == '>' || *c == '<') return true; else c++;
 return false;
}
*/

#define EOTAG ('>')
#define ENDLINE "\n"

void Stream::PutBackTag(const char * tag,bool *EndTag )
 {
  putback(strlen(tag)+(EndTag ? 3 : 2));
 }

 //would like to use an encoding function as a parameter?

void Stream::WriteTag(const char * type, TParameterList & Options,bool EndTag)
{
  TNameValuePair * n;
 // TStr value;
  write("<",1);
  writestr(type);
    FOREACHPTR(n,Options)
      if (*n->Name)
      {
       if (*n->Value)
       {
   //     URLEncode(value,n->Value,0,true);
        *this << " " << n->Name << "=\"";
        HTMLEscape(*this,n->Value);
        *this << "\"";
        }
       else
       {
        *this << " " << n->Name << "=\"\"";
       }
      }
    DONEFOREACH

  if (EndTag) *this << " /";
  *this << ">";
}

int Stream::ReadUntilTag(const char * tag,Stream * SkipText,bool *EndTag)
{
 int c;
 TStr comp;

 while ((c = StartTag(comp,SkipText,EndTag)) != EOF)
 {
  if (comp == tag) return c;

  if (SkipText && strlen(comp))
    {
     SkipText->writestr("<");
     SkipText->writestr(comp);
     char d = (char)c;
     SkipText->write(&d,1);
     if (c != EOTAG && c != EOF)
      {
        c = ReadUntilChar(">",SkipText);
        if (c != EOF)
         {
          char d = (char)c;
          SkipText->write(&d,1);
         }
      }
    }
 }
 return c;
}

int Stream::StartTag(TStr& type,Stream * SkipText,bool *EndTag,const char* allowed)
{
  // <PAGE/>
 MemoryStream name(128);
 int c;

 if (EndTag) *EndTag = false;

 bool noNegative = allowed && *allowed && !strchr(allowed,'/');

 while (1)
 {
 if ( ReadUntilChar("<",SkipText) != '<' ) return EOF;
 c = ReadUntilChar(" \t\r\n><",&name) ;
 if (c == EOF)
  return EOF; //oops

 char * x = name;
 size_t length = name.pos();

 if (c == '>' && length)
   if (x[length - 1] == '/')
    {
     if (EndTag) *EndTag = true;
     x[length - 1] = 0;
    }

 if (noNegative && x[0] == '/') x++;

 if (allowed)
  if (*allowed)
   if (strncasecmp(name,"![CDATA[",8) && !HasToken(allowed,x))
 {
  if (SkipText)
  {
   SkipText->write("<",1);
   SkipText->writestr(name);
   char out = (char)c;
   SkipText->write(&out,1);
   if (c != '>')
    {
     out = ReadUntilChar(">",SkipText);
     SkipText->write(&out,1);
    }
  }
  name.Clear();
  continue;
 }

  type = (char*)name;
  return c;
 }

}

static int eatwhite(Stream& in,int last)
{
 if (!IsSpace(last)) return last;
 char c;
 while (in.read(&c,1))
  if (!IsSpace(c)) return c;
 return EOF;
}

void Stream::SkipTag(int c, Stream* TagText)
{
 if (c == EOTAG)
   {if (TagText) TagText->write(">",1); }
 else if (c == EOF) return;
 else
   {
    char x = c;
    if (TagText) TagText->write(&x,1);
    x = ReadUntilChar(">",TagText);
    if (x!= EOF) if (TagText) TagText->write(&x,1);
   }
}

bool Stream::FinishTag(int StartResult, TParameterList* params,bool *EndTag,bool decode)
{
 if (StartResult == EOTAG) return true;
 if (StartResult == EOF) return false;
 MemoryStream name, value;
 int c = StartResult;
 //sadly, we have to do all that processing, just to check
 //for >'s embedded in quotes.
 char * n,*v;

 while (c != EOTAG && c != EOF)
 {
  // name
  name.Clear();
  value.Clear();
  if (!IsSpace(c)) {char ch =c; name.write(&ch,1); }
  c = ReadUntilChar("= \t\r\n>",&name);
  n = StripCharsFB(name," \t\r\n");
  c = eatwhite(*this,c);
  if (c == '=') //reading a value
   {
    char ch;
    read(&ch,1);
    c = ch;
    c = eatwhite(*this,c);
    if (c == '\"' || c == '\'')
     {
      char stop[2] = {0,0}; stop[0]=c;
      c = ReadUntilChar(stop,&value);
      read(&ch,1);
      c = ch;
      c = eatwhite(*this,c);
      v = value;
     }
    else
     {
      char ch = c;
      value.write(&ch,1);
      c = ReadUntilChar(" \t\r\n>",&value);
     // c = eatwhite(in,c);
      v = StripCharsFB(value," \t\r\n");
     }
   }
  else
   {
    v = 0;
   }

  if (c == EOTAG && !strcmp(n,"/"))
   { //  <TAG NAME=VALUE />
    if (EndTag) *EndTag = true;
   }
  else
   {
    if (decode && v) HTMLUnEscape(v);
    else if (!v) v = (char*)"";
    if (*n && params) params->Set(n,v);
    c = eatwhite(*this,c);
   }
  }
 return c != EOF;
}
