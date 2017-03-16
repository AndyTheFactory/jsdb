#ifndef _RS_SYSTEM_H
#define _RS_SYSTEM_H


#ifndef _RS_STRING_H
#include "rs/string.h"
#endif
#ifndef _RS_LIST_H
#include "rs/list.h"
#endif
/*--------------------------------------------
           Standard Callback Function
---------------------------------------------*/

typedef bool (*RSCallback)(void*v,int i, int imax,const char * title);

/*--------------------------------------------
                TStringList
---------------------------------------------*/

//#include "pshpack1.h"

class Strings
{public:
 virtual char * operator [] (size_t i) =0;
 virtual size_t Count() = 0;
};

class TWORMList : public Strings//write-once, read-many version of string list
{public:
 struct TData
 {
   TChars data;
   int pos;
   TData* next;
   TData(TData*n,size_t l): data(l), pos(0), next(n) {}
   ~TData();
 };
 TData* tail;
 TList<char> pointers;

 size_t Add(const char* x);
 void Clear();
 size_t Count() {return pointers.Count();}
 char * Get(size_t i) {return pointers.Get(i);}
 char * operator [] (size_t i)  {return Get(i);}
 TWORMList(size_t minsize);
 ~TWORMList();
};

class TStringList : public Strings
   {
    public:
    TList<TStr> Items;
    TStringList(size_t InitialSize=0);
    TStringList(const char * str, const char * delims=0);
    ~TStringList();

    char * Get(size_t i);
    char * operator [] (size_t i)  {return Get(i);}

    int32 GetInt(size_t i,int32 def = 0);

    size_t Add(const char * c,size_t len = 0) ;
    size_t Add(TStr * c);

    bool Set(size_t i, const char * c);
    size_t Count() {return Items.Count();};

    bool Has(size_t i) {return Items.Has(i);}
    bool Has(const char * c,size_t len=0);

    void Replace(const char * old, const char * c)
      { Set(Find(old),c); }

    size_t Find(const char * c,size_t len = 0);

    size_t StartsWith(const char*c);

    void Clear() {Items.Clear();};

    TStringList & operator = (TStringList & c);
#ifdef __BORLANDC__
    operator TStr** ();
#endif
    void Undelimit(const char * source, const char * delimiter,
              bool KeepBlanks=false);

    void Undelimit(const char * source, int delimiter, bool Blanks)
     { char c[2]; c[0]=(char)delimiter;c[1]=0;Undelimit(source,c,Blanks); }

   };

/*--------------------------------------------
                TParameterList
---------------------------------------------*/
/*
struct WNameValuePair
{
 WStr Name, Value;
 WNameValuePair(const uint16 * _name = 0,const uint16 * _value = 0);
 WNameValuePair(const uint16 * n, size_t nl, const uint16 * v, size_t vl);
 ~WNameValuePair();
};
*/
struct TNameValuePair
{
 TStr Name, Value;
 TNameValuePair();
 TNameValuePair(const char * _name,const char * _value);
 TNameValuePair(const char * n, size_t nl, const char * v, size_t vl);
 ~TNameValuePair();
 int Compare(const TNameValuePair &ts)
   {
    int i = Name.Compare(ts.Name);
    return (i ? i : Value.Compare(ts.Value));
   }
 int CompareAlt(const TNameValuePair &ts)
   {
    int i = Value.CompareAlt(ts.Value);
    return (i ? i : Name.Compare(ts.Name));
   }
 operator const char* () {return (const char*)Name;}
};

bool ReplaceText(const char * fval,TChars & gridtext,TNameValueList& r);

class TParameterList: public TNameValueList
{
 public:
 TList<TNameValuePair> Items;
 bool CaseSensitive; //case sensitive
 TPointer<TIndexList> index;

 TNameValuePair* find(const char * name,size_t len=UINT_MAX);

 TParameterList();
 TParameterList(bool CaseSensitive);
 TParameterList(const char *, char delim);
  /* reads from a paired string */

 ~TParameterList();

 void SetAsis(bool CS) {CaseSensitive = CS;}
 void operator = (const TParameterList&o); /* actually appends */
// void Set(TNameValueList&o); /* actually appends */

 /* Reads and parses Windows INI file section. Neat, eh? */
 //moved to TNameValueList in list.h
 //size_t ReadINIFileSection(const char * file, const char * section);
 //void WriteINIFileSection(const char * file, const char* section);

  /* Reads and parses Windows INI file section. Neat, eh? */
#ifdef XP_WIN
 size_t ReadRegistrySection(HKEY base,const char * section);
 void WriteRegistrySection(HKEY base,const char * section);
#endif

  /* Read values of the form { "name1=value1", "name2=value2", ... } */
 //size_t Read(int start, int argc, char ** argv);
  /* Read values when running as a Netscape plug-in */
 //size_t Read(int start, int argc, char ** argn, char ** argv);

  /* Reads a string of the form name1=value1/name2=value2/...*/
  // does not write a final delimiter
// size_t Read(const char *,char delim);

 //moved to TNameValueList in list.h
 //void Write(TStr& out,const char * delim);

 bool Set(size_t index, const char * Value);
 bool Set(const char * Name, const char * Value);
 void Set(const char * Name, int32 Value = 1);
// #ifdef _HAS_INT64
// void Set64(const char * Name, int64 Value = 1);
// #endif

 void Set(const char * Name, size_t nl, const char * Value, size_t vl);
 void Unset(const char * Name);

 char * GetOption(const char * Name, char * def);

 char * Get(const char * Name, char * def)
  { return GetOption(Name,def);}

 const char * GetOption(const char * Name, const char * def)
  { return GetOption(Name,(char*)def);}

 //int32 GetInt(const char * name,int def = 0);
 //double GetDouble(const char * name,double def = 0.0);

 size_t IndexOf(const char *Name) {return Items.IndexOf(find(Name));}

 bool Has(size_t i) {return Items.Has(i);}
 bool Has(const char * Name) {return find(Name) != 0;}

 char * Get(const char * Name,size_t slen=UINT_MAX);
  /* returns "" if the option is not found */

 const char * Name(size_t i)
   {return i < Items.Count() ? (char*)Items[i]->Name : (char*)NULL;}

 const char* Value (size_t i)
   {return i < Items.Count() ? (char*)Items[i]->Value : (char*)NULL;}

 TNameValuePair* operator [] (size_t i)
   {return Items[i];}

 size_t Count() {return Items.Count();}
 char* operator [] (const char*c) {return Get(c);}
 const char * operator () (const char * Name) {return Get(Name);}

 void Clear();

#ifdef OWL_EDIT_H
 void SetEdit(TEdit& e,const char* tag,const char * def = "")
   {e.SetText(Has(tag) ? Get(tag) : def);}
 void GetEdit(TEdit& e,const char* tag)
   {int i = e.GetWindowTextLength(); TStr s(i); e.GetText(s,i+1); Set(tag,(char*)s);}
 void SetEdit(UnicodeEditor& e,const char* tag,const char * def = "");
 void GetEdit(UnicodeEditor& e,const char* tag);
 void SetCheck(TCheckBox & b, const char * tag,int def=0)
   {b.SetCheck((Has(tag) ? atoi(Get(tag)) : def) ? BF_CHECKED : BF_UNCHECKED);}
 void GetCheck(TCheckBox &b, const char * tag)
   {Set(tag,b.GetCheck()==BF_CHECKED ? "1" : "0");}
#endif

  void RebuildIndex();
  size_t FindIndex(const char* name,size_t len);
};

/*! Unlike TParameterList, WParameterList is always case sensitive, and
returns NULL if an entry is not found in the list */
/*
class WParameterList
{
 public:
 TList<WNameValuePair> Items;

 WNameValuePair* find(const uint16 * name,size_t *index = NULL);

 //always case sensitive
 WParameterList();
 ~WParameterList();

#ifdef _HAS_INT64
 void Set(const uint16 * Name, int64 Value);
#endif

 bool Set(const uint16 * Name, const uint16 * Value);

 void Set(const uint16 * Name, size_t nl, const uint16 * Value, size_t vl);

 void Remove(const uint16 * Name);

 uint16 * Get(const uint16 * Name, uint16 * def = 0)
  { WNameValuePair*x = find(Name); return x ? (uint16*)x->Name : def;}

 bool Has(const uint16 * Name) {return find(Name) != 0;}

 const uint16 * Name(size_t i)
   {return i < Items.Count() ? (uint16*)Items[i]->Name : (uint16*)NULL;}

 const uint16* Value (size_t i)
   {return i < Items.Count() ? (uint16*)Items[i]->Value : (uint16*)NULL;}

 size_t Count() {return Items.Count();}

 const uint16 * operator () (const uint16 * Name) {return Get(Name);}

 void Clear() {Items.Flush();}
};
*/
#endif

