#ifndef _RS_STRING_H
#define _RS_STRING_H

#ifndef _RS_DEFS_H
#include "rs/defs.h"
#endif

#ifndef _RS_CHAR_H
#include "rs/char.h"
#endif

//#include "pshpack1.h"

#ifdef OWL_EDIT_H //OWL
class UnicodeEditor;
#endif

class TStr;
class TChars;


// simple string manipulation functions
bool strsplit(const char * in, const char * split,TStr& before,TStr& after);

// date and time
bool WriteDate(TChars &out,unsigned y, unsigned m, unsigned d);

// Internet functions
void URLSplit(const char * URL, TStr& service, TStr& user,
              TStr& password, TStr& server, TStr& filename, TStr& query);

/*-----------------------------------------------------------------------
 class TChars
 a sized array of bytes, completely inline. Reallocation is fairly efficient.
-----------------------------------------------------------------------*/

class TChars
 {
 public:
  char * buf;
  size_t size;

 public:
  TChars(const char * Buffer, size_t Size)
     { size = Size; buf = AllocStr(Size); memcpy(buf,Buffer,Size);}

  TChars(size_t Size)
    { size = Size; buf = AllocStr(size); }

  TChars() {size=1024; buf = AllocStr(size); }

  ~TChars() {FreeStr(buf);}

  operator char *() {return (char*)buf;}

  uint16 * w() const {return (uint16*)(void*)buf; }

  inline char* Resize(size_t Size,const char * newbuf=0);

  inline char* Copy(size_t length,const char * c);

  char* Set(const char* c)
   {size_t x = c ? strlen(c) : 0; return x > size ? Resize(x,c) : Copy(x,c); }

  char* Set(const char* c,size_t x)
   {return x > size ? Resize(x,c) : Copy(x,c); }

  char* operator = (const TChars& o)
   { return o.size > size ? Resize(o.size,o.buf) : Copy(o.size,o.buf); }

  char* operator = (const char * c) {return c?Set(c):Set("",0);}

  void Append(const char * c, size_t length)
   {
      size_t y = strlen(buf);
      size_t x = length + y ;
      strncpy((x > size ? Resize(x) : buf) + y, c, length);
      buf[x]=0;
   }

  TChars& operator << (const char *c)
    {
      Append(c,strlen(c));
      return *this;
    }

  friend TChars& operator << (TChars& os, int i);
 };

inline TChars& operator << (TChars& os, int i)
{char c[64];itoa(i,c,10); os << c; return os;}

inline char* TChars::Resize(size_t Size,const char * newtext)
  {
   char * newbuf=AllocStr(Size);
   size_t s = newtext ? Size : min(size,Size);
   if (s)
     memmove(newbuf,newtext ? newtext : (const char*)buf ,s);
   FreeStr(buf);
   buf=newbuf;
   size=Size;
   return buf;
  }

inline char* TChars::Copy(size_t length,const char * c)
  {
   size_t x =length<size?length:size;
   memmove(buf,c,x);
   buf[x]=0;
   return buf;
  }

class WStr
  {public:
   TChars data;

   WStr (const char* x,size_t max=UINT_MAX); /* convert from UTF-8 */

   WStr (const uint16* x, size_t length) : data((const char*)(const void*)x,length*sizeof(uint16)) {}

   WStr (size_t length) : data(length*sizeof(uint16)) {}

   WStr (const uint16* x) : data((const char*)(const void*)x,x?(ucslen(x)+2)*sizeof(uint16):4)
    { memset(data.buf + data.size - 2*sizeof(uint16), 0 , 2*sizeof(uint16)); }

   WStr () : data(2 * sizeof(uint16))
    { memset(data.buf, 0, data.size); }

   ~WStr()
    { }

   operator uint16* () {return data.w(); }
#ifndef __MSC__
   operator wchar_t* () {return (wchar_t*)data.w(); }
#endif
   WStr & operator = (const uint16 * c)
    {data.Set((const char*)(const void*)c,ucslen(c)*sizeof(uint16)); return *this;}

   WStr & operator = (const WStr & s)
    {data.Set(s.data.buf,ucslen(s.data.w())*sizeof(uint16)); return *this;}

   WStr & operator += (const uint16 * c)
    {size_t ol = ucslen(data.w());
     size_t nl = ucslen(c);
     data.Resize((ol + nl + 2)*sizeof(uint16));
     memmove(data.buf + (ol * sizeof(uint16)), c, nl*sizeof(uint16));
     memset(data.buf + data.size - 2*sizeof(uint16), 0 , 2*sizeof(uint16));
     return *this;
    }

    //! x is length in bytes
   uint16* Set(const uint16* c,size_t x)
   {return (uint16*)(void*)data.Set((const char*)(const void*)c,x); }

   size_t length() {return ucslen(data.w());}
   size_t bytes() {return data.size;}

   void Resize(size_t l)
    { data.Resize(l * sizeof(uint16),data.buf); }

   friend WStr& operator << (TStr& os, const uint16 * str );

#ifdef OWL_EDIT_H //OWL
    uint16 * operator = (TEdit& e)
      {int i = GetWindowTextLengthW(e); data.Resize(i*sizeof(uint16)); GetWindowTextW(e,(wchar_t*)data.w(),i+1); return data.w();}
    uint16 * operator = (UnicodeEditor& e) ;
    uint16 * operator = (TStatic& e)
      {int i = GetWindowTextLengthW(e); data.Resize(i*sizeof(uint16)); GetWindowTextW(e,(wchar_t*)data.w(),i+1); return data.w();}
    uint16 * operator = (TComboBox& e)
      {int i = GetWindowTextLengthW(e); data.Resize(i*sizeof(uint16)); GetWindowTextW(e,(wchar_t*)data.w(),i+1); return data.w();}
#endif
  };

inline WStr& operator << (WStr& os, const uint16 * c)
{os+=c; return os;}

/* don't trust the built-in strcmp or stricmp */
int rsstrcmp(const char* a, const char* b);
int rsstricmp(const char* a, const char* b);

/*!
 Better than the ANSI string, it simply allocates memory when it needs it
*/

class  TStr
{
    public:

    char * str;

    TStr(){str=AllocStr((size_t)0u);}
    TStr(const uint16* w,size_t len=UINT_MAX); /* convert to UTF-8 */
    TStr(const char * c){str=AllocStr(c);}
    //@name concatenate appends a list of strings
    //@{
    TStr(const char ** c);
    TStr(char ** c);
    //@}
    TStr(const char * s1,const char * s2,const char * s3=0,const char * s4=0,
         const char * s5=0,const char * s6=0,const char * s7=0,const char * s8=0,
         const char * s9=0,const char * sA=0,const char * sB=0,const char * sC=0);
    TStr(const char * c, size_t len) {str=AllocStr(len); memcpy(str,c,len);}
    TStr(size_t len,char v) {str=AllocStr(len);memset(str,v,len);}
    TStr(size_t len) {str=AllocStr(len); str[0]=0;}
    TStr(const TStr& ts) {str = AllocStr(ts.str);}
    ~TStr();

    //whole bunch of inline functions:
    int Compare(const TStr&ts) {return strcmp(str,ts.str);}
    int CompareAlt(const TStr &ts)
    { //first numbers, then blanks, then letters
     int j =  atoi(str) - atoi(ts.str) ;
     return (j ? j : Compare(ts));
    }
    bool operator == (const TStr& ts){return (strcasecmp(str,ts.str)==0);}
    bool operator == (const char * c){return c ? (strcasecmp(str,c)==0) : (*str==0);}
    bool operator != (const TStr & ts) {return !(*this == ts);}
    bool operator != (const char * c) {return !(*this == c);}
    bool operator < (const TStr & ts){return (strcasecmp(str,ts.str)<0);}
    bool operator < (const char * c){return c ? (strcasecmp(str,c)<0) : false;}
    operator char* () const {return str;}
    //operator bool() {return str[0] != 0;}

    void Resize(size_t len,const char * c=NULL);
    void Set(const char * c, size_t length);
    void Truncate(size_t len);
    void Exchange(TStr&ts); //swap strings, eliminates unnecessary delete[] calls.
    bool IsEmpty() {return str[0] == 0;}
    bool operator  ++()   //actually a left-shift operator
      {return str[0] ? (memmove(str,str+1,strlen(str)),true) : false;}
    bool operator --(int) //truncates
      {return str[0] ? (str[strlen(str)-1]=0,true):false;}
    const char * Substring(size_t s)
      {return s > strlen(str) ? "" : str+s;}

    void itoa(int i) {char x[16]; ::itoa(i,x,10); Set(x,16);}
   // TStr & operator << (int i) {char x[16]; ::itoa(i,x,10); return *this += x;}

    TStr & operator = (const char * c){Set(c,c?strlen(c):0);return *this;}
    TStr & operator = (const TStr & s) {Set(s.str,strlen(s.str)); return *this;}
    TStr & operator = (const TChars & s) {Set(s.buf,s.size); return *this;}
    TStr & operator += (const char * c); //returns *this
    TStr operator + (const char * c) {return TStr(str,c);} //bad function

#ifdef OWL_EDIT_H //OWL
    char * operator = (TEdit& e)
      {int i = e.GetWindowTextLength(); Resize(i); e.GetText(str,i+1); return str;}
    char * operator = (UnicodeEditor& e);
    char * operator = (TStatic& e)
      {int i = e.GetWindowTextLength(); Resize(i); e.GetText(str,i+1); return str;}
    char * operator = (TComboBox& e)
      {int i = e.GetTextLen(); Resize(i); e.GetText(str,i+1); return str;}
    char * operator = (TListBox& e)
      {Resize(MAXPATH); e.GetSelString(str,MAXPATH); return str;}
#endif
#ifdef _WX_TEXTCTRL_H_BASE_ //wx
    char * operator = (wxTextCtrl& e)
      {return (*this)=e.GetValue();}
    char * operator = (wxComboBox& e)
      {return (*this)=e.GetValue();}
    char * operator = (wxListBox& e)
     {return (*this)=e.GetStringSelection();}
#endif
    ///ANSI string functions
    //@{
    char * c_str() const {return str;} //for compatibility
    size_t length() const {return strlen(str);}
    //@}
    ///List functions
    //@{
#ifdef  __BORLANDC__ /* MSC and GNU use the char*() operator instead */
    char& operator [] (size_t i) {return str[i];}
#endif
    size_t Count() const {return strlen(str);}
    //@}

    //If someone else will clean up the memory, and the string will not be used again.
    char* release() {char* x = str; str=0; return x;}

    void insert(int loc,const char * c)
     {TStr s(TStr(str,loc),c,str+loc); Exchange(s);  }

    ///Java string functions
    int indexOf(const char* sub)
     {const char*c = stristr((char*)str,(char*)sub); return c ? c-str : -1;}
    friend TStr operator + (TStr& os, const char * str );

    TStr substring(int start,int stop=INT_MAX)
     {start = start < 0 ? 0 : start;
      stop = stop > (int)strlen(str) ? (int)strlen(str) : stop;
      return TStr(str+start,stop < start ? 0 : stop-start);
     }

    TStr substr(size_t start,size_t stop) {return substring(start,stop);}

    void replace(const char * find, const char * replace);

    void split(const char * split,TStr& before, TStr& after)
     {strsplit(*this,split,before,after);}

    void ANSItoUTF8()
     { TStr ts(::ANSItoUTF8(str,0));
       ::ANSItoUTF8(str,ts);
       Exchange(ts);
     }

    friend TStr& operator << (TStr& os, const char * str );
    friend TStr& operator << (TStr& os, int c);
};

inline TStr itos(int i)
{char x[16]; itoa(i,x,10); return TStr(x);}

#ifdef _HAS_INT64
inline TStr i64tos(int64 i)
{char x[64]; _i64toa(i,x,10); return TStr(x);}
#endif

inline TStr operator +(TStr& os, const char * str )
{return TStr(os,str);}

inline TStr& operator << (TStr& os, const char * c)
{os+=c; return os;}

inline TStr& operator << (TStr& os, int i)
{char c[64];itoa(i,c,10); os+=c; return os;}

#ifdef _HAS_INT64
inline TStr& operator << (TStr& os, int64 i)
{char c[64]; _i64toa(i,c,10);  os+=c; return os;}
#endif
//@{
///protect against memory errors
inline TStr& strcpy(TStr&x, const char* y) {x = y; return x;}
inline TChars& strcpy(TChars&x, const char *y) {x = y; return x;}

inline TStr& strcat(TStr&x, const char* y) {x += y; return x;}
inline TChars& strcat(TChars&x, const char *y) {x.Append(y,strlen(y));return x;}
//@}
//#include "poppack.h"
#endif
