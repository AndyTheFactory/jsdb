#include "rslib.h"
#pragma hdrstop
//#include "rs/char.h"
//#include "rs/system.h"

bool  IsNumberCode(char * Str)
 {
    while (*Str)
     {
       if (!strchr("1234567890XxYyZz+-. ", Str[0]) ) return false;
       Str++;
     }
   return true;
 }

char * CleanQuestName(char * Str)
   {
     StripPunct(Str);
     for (int i=0; Str[i]; i++) Str[i]=toupper(Str[i]);
     Str[10] = 0;
     return Str;
   }


bool strmatch( register const char * c1,const char * c2, size_t len)
{
  if (len) do
  {
  if (*c1 != *c2) return false;
  c1++;
  c2++;
  len--;
  //if (end of token AND end of comparison)
  if ((*c2==0) && (!len||*c1==0||*c1 == ','||*c1==' ')) return true;
  } while (len);
  return false;
}

void ToggleMultResp(char * c, const char * code, size_t length)
{
 if (!length) length=strlen(c);
 if (HasMultResp(c,code,length))
  ClearMultResp(c,code,length);
 else
  SetMultResp(c,code,length);
}

void SetMultResp(char * c, const char * code, size_t length)
{
  if (!length) length=strlen(c);
  if (HasMultResp(c,code,length)) return;
  bool hasany = (!isspace(*c));
  while (*c && !isspace(*c) && length) {c++; length--;}
  if (!length) return;
  if (hasany) {*c = ','; c++; length--;}
  size_t cl = strlen(code);
  if (cl > length) return;
  memmove(c,code,cl);
}

void ClearMultResp(char * c,const char * code, size_t length)
{ //almost the same as in sum_calc
  if (!length) length=strlen(c);
  while (length)
  {
   if (*c == ',') { c++; length--; }
   else
   {
    if (strmatch(c,code,length))
     {
      size_t cl = strlen(code);
      if (c[cl]==',') cl++; //don't bother to check the length here.
      if (length >= cl)
       {
        length -= cl;
        memmove(c,c+cl,length);
        memset(c+length,' ',cl);
       }
      return;
     }
    while (length && *c != ',') {c++; length--;}
   }
  }
} // wasn't that clever?

bool HasMultResp(register const char * data,
                      register const char * compare,
                 size_t datalength,
                      size_t tokensize)
{ register size_t i = 0;
//  const char * stop = data + datalength;
  while (i < datalength)
  {
   if (data[i] == 0) break;
   if (data[i] == ',' || data[i] == ' ')
    {
      i++;
    }
    else
    {
      size_t j=i;
      while (j < datalength && data[j] != ','
             && data[j] != ' ' && data[j] != 0 ) j++;
         //comma, space, zero
      if (i + tokensize == j)
       {
        if (memcmp(data+i,compare,tokensize)==0) return 1;
       }
      i = j;
    }
  }
  // old-style survey 2.7 fields: single character codes w/o commas
//  if (tokensize==1 && !strnchr(data,',',datalength))
//    return strnchr(data,*compare,datalength);
  return 0;
 }

bool HasMultResp(const char * c, const char * code, size_t length,int*loc)
{
 if (loc) *loc = 1;
 size_t i = 0;
     if (!c) return false;
 if (!length) length=strlen(c);

  while (i < length && (c[i]==',' || c[i]==' ')) {i++; }

  while (i < length)
  {
   if (c[i] == ',') { i++; if (loc) (*loc)++;}
   else
   {
    if (strmatch(c+i,code,length)) return true;
    while (i<length && c[i] != ',') {i++; }
   }
  }
  //old method -- if there are no commas, look for an instance
  //of the 1-character code anywhere in the string.
//  if (code[1]==0 && !strnchr(c,',',length)) return strnchr(c,*code,length);
  return false;
}

bool HasToken(const char* field, const char* code)
{
 size_t l, m = strlen(code);
 while (*field)
 {
  l = strcspn(field,",;| \t\r\n");
  if (l)
  {
   if (l==m && !strncasecmp(field,code,l)) return true;
   field += l;
  }
  field++;
 }
 return false;
}

bool ReplaceText(const char * fval,TChars & gridtext,TNameValueList& r)
{
// gridtext="";
 TChars find;
 if (!r.Count()) return false;
 bool any = false;
 while (*fval)
  {
   while (*fval && strchr(",; ",*fval)) fval++;
   //while it's a space or punctuation, scan forward

   size_t i = 0;
   while (fval[i] && !strchr(",; ",fval[i])) i++;
   //find the character that ends this token

   if (!i) continue;

   if (any) gridtext << ",";
   any = true;
   //delimit the output

   find.Set(fval,i);
   const char * c = r(find);
   //find the field value

   if (*c)
    gridtext << c;
   else
    gridtext << find;

   fval+= i;

 }
  return true;
}
