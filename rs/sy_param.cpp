#include "rslib.h"
#pragma hdrstop

TNameValuePair::TNameValuePair() {}
TNameValuePair::TNameValuePair(const char * _name,const char * _value) : Name(_name), Value(_value) {}
TNameValuePair::TNameValuePair(const char * n, size_t nl, const char * v, size_t vl)
     : Name(n,nl), Value(v,vl) {}
TNameValuePair::~TNameValuePair() {}


const char* TNameValueList::operator () (const char*c)
{
     size_t last = Count();
     for (size_t i=0; i<last; i++)
      {
            if (!strcmp(c,Name(i))) return Value(i);

      }
      return "";
}

bool TNameValueList::Has(const char * name)
{
 size_t last = Count();
 for (size_t i=0; i<last; i++)
  {
            if (!strcasecmp(name,Name(i))) return true;
  }
 return false;
}

void TNameValueList::Append(TNameValueList&o)
{
 size_t last = o.Count();
 for (size_t i=0; i<last; i++)
  {
   Set(o.Name(i),o.Value(i));
  }
}

TParameterList::TParameterList() {CaseSensitive=false;}

TParameterList::TParameterList(bool x) {CaseSensitive=x;}

TParameterList::TParameterList(const char * c, char delim)
{
 CaseSensitive=false;
 if (c) Read(c,delim);
}

TParameterList::~TParameterList() {}

void TParameterList::Clear()
 {
  index=0;
  Items.Flush();
 }

size_t TParameterList::FindIndex(const char* key,size_t len)
{
  if (index == 0)
  {
   if (Count() > 128)
    RebuildIndex();
   else
    return NOT_FOUND;
  }

  size_t nelem = index->Count();

  if (Count() < nelem) //must have deleted something
  { index = 0; return NOT_FOUND;}

  size_t low,i,probe;
  int j;

  low = 0;
  if (nelem)
  {
   while (nelem > 0)
   {
    i = nelem >> 1; // *1/2
    probe = low + i; //midpoint

    size_t pos = (*index)[probe];
    const char * c = Name(pos);
    if (!c) c = "";
    j = CaseSensitive ? strncmp(key,c,len) : strncasecmp(key,c,len);

    if (j == 0 && c[len]) j=-1;

    if (j == 0)
      return pos;
    else if (j < 0) //look lower
      nelem = i;
    else //look higher
    {
      low = probe + 1;
      nelem = nelem - i - 1;
    }
   }
   size_t pos = (*index)[probe];
   const char * c = Name(pos);
   if (!c) c = "";
   j = CaseSensitive ? strncmp(key,c,len) : strncasecmp(key,c,len);
   if (j == 0 && c[len]==0) return pos;
  }

  nelem = Count();
  for (i = index->Count(); i< nelem; i++)
  {
   const char * c = Name(i);
   j = CaseSensitive ? strncmp(key,c,len) : strncasecmp(key,c,len);
   if (j == 0&& c[len]==0) return i;
  }
  return NOT_FOUND;
}

class MyListIndexer : public TQuickSorter
{public:
 TParameterList * list;
 MyListIndexer(TParameterList * l,TIndexList* order)
  : TQuickSorter(0,l->Count(),order),list(l) {};
 ~MyListIndexer() {};
 char* Get(size_t i) {return (char*)list->Name(i);}
};

void TParameterList::RebuildIndex()
{
 index = new TIndexList();
 MyListIndexer(this,index).Sort();
}

TNameValuePair* TParameterList::find(const char * Name,size_t len)
 {
  if (!Name) return 0;
  len = min(len,strlen(Name));

  size_t f = FindIndex(Name,len);

  if (f != NOT_FOUND) return Items[f];
  else if (index) return 0;

  TNameValuePair* x;
  FOREACH(x,Items)
   if (strlen(x->Name)==len)
   {
    if (CaseSensitive)
      {
        if (strncmp(x->Name,Name,len)==0) return x;
      }
    else
     {
        if (strncasecmp(x->Name,Name,len)==0) return x;
     }
   }
  DONEFOREACH;
  return 0;
 };

char * TParameterList::Get(const char * Name,size_t slen)
 {
  TNameValuePair* x = find(Name,slen);
  if (x) return x->Value;
  return (char*)"";
 };

char * TParameterList::GetOption(const char * name, char * def)
 {
  TNameValuePair* x = find(name,strlen(name));
  if (!x) return def;
  return x->Value;
 }

void tocolorref(int32& i)
{
 char * c = (char*)(void*)(&i);
 char x = c[2];
 c[2]=c[0];
 c[0]=x;
}

#ifdef _HAS_INT64

void TNameValueList::Set64(const char * Name, int64 Value)
{
 char s[100];
 _i64toa(Value,s,10);
 Set(Name,s);
}

int64 TNameValueList::GetInt64(const char * name,int64 def)
{
     const char * value = (*this)(name);
     if (!value) return def;
     if (!*value) return def;
     return _atoi64(value);
}
#endif

int32 TNameValueList::GetInt(const char * name,int def)
   {
     //TNameValuePair* x = find(name,strlen(name));
     //if (!x) return def;
     const char * value = (*this)(name);
     if (!value) return def;

    switch (*value)
    {
     case '#': {int32 l = strtol(value+1,0,16); tocolorref(l); return l;}
     case 0: return def;
     default : return strtol(value,0,0);
    }
   }

double TNameValueList::GetDouble(const char * name,double def)
   {
    const char * value = (*this)(name);
    //TNameValuePair* x = find(name,strlen(name));
    if (!value) return def;
    if (!*value) return def;
    return strtod(value,0);
   }

void TParameterList::Set(const char * Name, int32 Value)
{char s[100];
 ltoa(Value,s,10);
 Set(Name,s);
}

void TParameterList::Unset(const char * Name)
{
  TNameValuePair* x = find(Name,UINT_MAX);
  if (x) Items.Destroy(Items.IndexOf(x),true);
  index=0;
}
bool TParameterList::Set(size_t index, const char * Value)
{
 if (index < Items.Count())
 {
  Items[index]->Value.Set(Value,Value ? strlen(Value) : 0);
  return true;
 }
 return false;
}
void TParameterList::Set(const char * Name, size_t nl, const char * Value, size_t vl)
{
  if (!Value || !Name) return;
  TNameValuePair* x = find(Name,nl?nl:UINT_MAX);
  if (x)
   {
    x->Value.Set(Value,vl);
    return;
   }
  if (!nl) nl = strlen(Name);
  x = new TNameValuePair(Name,nl,Value,vl);
  Replace(x->Name,'=','~');
  Items.Add(x);
};

bool  TParameterList::Set(const char * Name, const char * Value)
 {
  if (!Value || !Name) return false;
  Set(Name,0,Value,strlen(Value));
  return true;
 };

#ifdef XP_WIN
void TNameValueList::WriteINIFileSection(const char * file,const char * section)
{
  FOREACHITER(*this)
   WritePrivateProfileString(section,Name(i),Value(i),file);
  DONEFOREACH
}
#else

void _tplWriteSection(FILE *f, const char* start, TNameValueList& Items)
{
  fprintf(f,"%s\n",start);

  FOREACHITER(Items)
   const char* name = Items.Name(i);
   if (!*name) continue;
   fprintf(f,"%s=%s\n",(char*)name,(char*)Items.Value(i));
  DONEFOREACH
}
void TNameValueList::WriteINIFileSection(const char * fname,const char * section)
{
  int size = FileSize(fname);
  char *d, *e;
  TStr start("[",section,"]");
  size_t i;
  FILE * f = fopen(fname,"rt");
  if (!f)
  {
   f = fopen(fname,"wt");
   if (!f) return;
   _tplWriteSection(f, start, *this);
   fclose(f);
   return;
  }

  TChars c(size+2);
  size = fread(c,1,size,f);
  fclose(f);
  c[size]=0;

  d = stristr(c,start);
  if (d) e = stristr(d+1,"\n[");
  else e = 0;

  f = fopen(fname,"wt");
  if (!f)
  {
   return;
  }

  fwrite(c,1,d ? (d-c) : size,f);
  _tplWriteSection(f, start, *this);
  if (e)
   fwrite(e,1,strlen(e),f);

  fclose(f);
}
#endif
#ifdef XP_WIN
size_t TNameValueList::ReadINIFileSection(const char * file,const char * section)
  /* Reads and parses Windows INI file section. Neat, eh? */
{
//TNameValuePair* v;
 char * c;
 char * name;
 TStr str(32000);
 int i,nl,vl;

 GetPrivateProfileSection(section,str,32000,file);
 c = str;
 char * value;
 i = 0;
 while (*c)
 {
  name = c;
  nl=0;
  while (*c && *c != '=') {c++;nl++;}
  c++;

  value = c;
  vl = 0;
  while (*c) {c++;vl++;}
  c++;

  name[nl]=0;
  value[vl]=0;
  Set(name,value);
  i++;
 }
 return i;
}
#else
static void killchar(char*c, char kill)
{
 int i,j;//i is the new str, j is the current str.
 i=0; j=0;

 while (c[j])
 {
     if (c[j] != kill)
     {
         c[i++] = c[j++];
     }
     else
     {
         j++;
     }
 }
     c[i]=0;    //added by jamesrosko 2003-10-06
}

size_t TNameValueList::ReadINIFileSection(const char * filename,const char * section)
{
 int d;
 char * str;
 FILE * file;
 MemoryStream x;

try {
 FileStream file(filename,Stream::OMText,Stream::ReadOnly);

 while ((d = file.ReadUntilChar('[') != -1))
 {
  x.Clear();
  d = file.ReadUntilChar("]\n",&x);
  if (d == ']')
   {
    if (strcasecmp(x,section)==0)
    {
     x.Clear();
     file.ReadUntilWord("\n[",&x);
     killchar(x,'\r');
     x.rewind();
     return x.ReadPaired(*this,'\n','=');
    }
   }
 }
} catch(...) {}
 return 0;
 }
#endif


void TParameterList::operator = (const TParameterList&o)
{
 Append(*(TNameValueList*)&o);
}


size_t TNameValueList::Read(int start, int argc, char ** argn, char ** argv)
{
 for (int i = start; i < argc; i++)
   Set(argn[i],argv[i]);
 return Count();
}

size_t TNameValueList::Read(int start, int argc, char ** argv)
  /* Read values of the form { "name1=value1", "name2=value2", ... } */
{
 char * c;
 int i,len,max;
 max = argc-start;
 for (i = 0 ; i < max ; i++ )
 {
  c = argv[i+start];
  len=0;
  while (*c && *c != '=') {c++;len++;}
  if (*c) {*c = 0; c++; }
  Set(TStr(argv[i+start],len),c);
 }
 return max;
}


void TNameValueList::Write(TStr& str,const char * delim)
{
  FOREACHITER(*this)
   if (i) str += delim;
   str += Name(i); str += "="; str += Value(i);
  DONEFOREACH;
}

size_t TNameValueList::Read(const char *str,char delim)
  /* Reads a string of the form name1=value1/name2=value2/...*/
{
 int i,nl,vl;
 if (!str) return 0;
 const char * c = str;
 const char * name; const char * value;
 i = 0;
 while (*c)
 {
  while ( *c && (strchr(" \t\n\r\v",*c) || *c == delim)) c++;
  name = c;
  nl=0;

  while (*c && *c != '=' && *c != delim) {c++;nl++;}
  if (*c && *c != delim) c++;

  value = c;
  vl = 0;
  while (*c && *c != delim) {c++;vl++;}
  if (*c) c++;
  if (nl == 0) continue;
  Set(name,nl,value,vl);
  i++;
 }
 return i;
}


