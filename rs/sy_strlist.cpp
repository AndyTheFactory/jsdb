#include "rslib.h"
#pragma hdrstop

void TStringList::Undelimit(const char * source, const char * delimiter, bool KeepBlanks)
{
    if (!source) return;
    if (!delimiter) delimiter=",; \t\r\n";
    int len;
    TStr tempstr(source);

    char * currentloc=tempstr;

    while (*currentloc!=0)
      {
       len=strcspn(currentloc,delimiter);
       if (KeepBlanks)
       {
          Add(len ? currentloc : "" , len);
       }
       else
       {
          if (len) Add(currentloc,len);
       }
       if (currentloc[len]==0) break;
       currentloc += len;
       currentloc++;
      }
}

bool TStringList::Set(size_t i, const char * c)
 {
  TStr* ts=Items[i];
  if (!ts) return false;
  *ts = c;
  return true;
 }

void tocolorref(int32& i); //sy_param.cpp

int32 TStringList::GetInt(size_t i,int32 def)
 {
    char*c;
    c = Get(i);
    if (!c) return 0;

    switch (*c)
    {
     case '#': {int32 l = strtol(c+1,0,16); tocolorref(l); return l;}
     case 0  : return def;
     default : return strtol(c,0,0);
    }
 }

char * TStringList::Get(size_t i)
      {
       TStr* ts=Items[i];
      return ts ? ts->str : 0;
      }

TStringList::TStringList(size_t InitialSize)
 :Items(InitialSize,0,InitialSize < 16 ? 8 : InitialSize / 2)
   {
   }

TStringList::TStringList(const char * str, const char * delims)
 : Items(true)
   {
    Undelimit(str,delims);
   }

TStringList::~TStringList()
   {
    Items.Flush();
   }

#ifdef __BORLANDC__
 TStringList::operator TStr** () {return Items.data;}
#endif
size_t TStringList::Add(const char * c, size_t len)
     {  TStr*t= len ? new TStr(c,len) : new TStr(c);
     return Items.Add(t);
     };

size_t TStringList::Add(TStr * c)
    {return Items.Add(c);}

size_t FindRowIndex(TRow<TStr> & Items,const char * comp)
{
  size_t max = Items.Count();
    TStr* t;
    for (size_t i = 0; i<max; i++)
     {
      t=Items[i];
      if (t) if (*t==comp) return i;
     }
     return NOT_FOUND;
}

bool TStringList::Has(const char * c,size_t len)
      {
       return Find(c,len) != NOT_FOUND;
      }

size_t TStringList::Find(const char * c,size_t len)
      {
       if (len) return FindRowIndex(Items,TStr(c,len));
       return FindRowIndex(Items,c);
      }

size_t TStringList::StartsWith(const char*c)
      {
       size_t max = Items.Count();
       size_t l = strlen(c);
       for (size_t i=0; i < max ; i++)
        {
         if (strncasecmp(c,Get(i),l)==0) return i;
        }
       return NOT_FOUND;
      };

TStringList& TStringList::operator = (TStringList & c)
{
 size_t max = c.Count();
 for (size_t i = 0; i < max ; i++ )
  {
   Add(c[i]);
  }
 return *this;
}

