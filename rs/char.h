#ifndef _RS_CHAR_H
#define _RS_CHAR_H

#include <stddef.h>
#include <wchar.h>

// sy_alloc
char * AllocStr(size_t len);

char * AllocStr(const char * s);

inline void FreeStr(const char * s) {delete [] s;}

//sy_crc

/// Start with 1 or 0xffffffff.
uint32 crc32(uint32 start,const char * x);
uint32 crc32(uint32 start,const void * x,size_t length);

//sy_double

bool  StrToDouble(const char * Str, double & X);

bool  IsDouble(const char * Str);

void  DoubleToStr(double X, char * Str);

//sy_form

char * CleanQuestName(char * Str);

//sy_token

int    ParseStr(char * Name, char ** pStr, char Ch, int Max);

int    ParseNameOffsetLen(char * Name, long * Offset, long * Len,
                long  * QuestType,  char ** pStr);

// external functions, requred in the exe file by some functions

void TimeOut(char * Str);

void MemoryOut(char * Str);

void HtmlHelp();

//sy_int32

int    CharToInt(char * Str);

char * IntToChar(int X, char * Str);

char * IntToStr32(int s3, char * Str);

int    Str32ToInt(const char * Str);

int    CompareStr32(const char * Str1, const char * Str2);

char * MakeHashStr(char * buf, char * Str);

const char * HexToStr(char * Str, long * X, int Max);

int  StrToHex(const char * Str, long * X, int Max);

// sy_char

/* GCC thinks wchar_t is 32 bits */
#ifdef XP_WIN
#define ucslen(a) lstrlenW((const wchar_t*)a)
#define ucscat(a,b) (uint16*)lstrcatW((wchar_t*)a,(const wchar_t*)b)
#define ucscpy(a, b) (uint16*)lstrcpyW((wchar_t*)a,(const wchar_t*)b)
#define ucscmp(a, b) lstrcmpW((const wchar_t*)a,(const wchar_t*)b)
#else
size_t ucslen(const uint16 *a);
uint16 *ucscat(uint16 *a, const uint16 *b);
uint16 * ucscpy(uint16 *a, const uint16 *n);
#define ucscmp(a,b) wcscmp((const wchar_t*)a,(const wchar_t*)b)
#endif

inline bool IsSpace(int x)
 {return (x!=0 && (x == ' ' || x == '\t' || x=='\n' || x=='\r'));}

/* missing functions */

const char * _stristr(const char *s1, const char *s2);

inline const char * stristr(const char *s1, const char *s2)
{return _stristr(s1, s2);}

inline char * stristr(char *s1, const char *s2)
{return (char*)_stristr(s1, s2);}

#ifndef __BORLANDC__
#ifdef XP_WIN
#ifdef _HAS_INT64
typedef __int64 int64;
#endif
#endif
char * itoa(int value, char* string, int radix);
char * ultoa(int value, char* string, int radix);
#ifdef _HAS_INT64
char * _i64toa(int64 value,char* string, int radix);
char * _ul64toa(int64 value,char* string, int radix);
int64 _atoi64(const char *strP);
#endif

#define ltoa(a,b,c) itoa(a,b,c)
int stricmp(const char *a, const char * b);
int strnicmp(const char*a,const char*,size_t len);
#endif

bool IsNumberCode(char * Str);

bool IsBlank( const char * c);

bool IsWhitespace(const char * Str, size_t length);

bool StartsWith(const char * Str1, const char * Str2, int ToUpper);

char * StripPunct(char * Str);

char * BStripB(char * Str, bool NoLeft=0);

char * AlphaNum(char * Str) ; //  Strip non AlphaNumeric chars

char * StripChar(char * Str, const char * pat);

char * StripCharsFB(char * c, const char * rem);

char * StripCharsFB(char * c, const char * rem,size_t len);

void Replace(char * str, char old, char n);

void Replace(char * str, const char* remove, char replace);

inline void Replace(uint16 *str, char o, char n)
  {if (str) for(;*str;str++) if (*str==o) *str=n;}

int RemoveChar(char * str, char remove, int length); // returns the number of bytes removed

inline char * BlankChar(char * Str, const char * pat)
  { Replace(Str,pat,' '); return Str; }

char * RightJust(char * Str, int Len);

char * RsStrncpy(char * Dest, const char * Src, int Len, bool IsPad=false);

const char * strnchr(const char * c, int comp, size_t length);

char * RsStrcat(char * str,  const char * str0,
                   const char * str1=0, const char * str2=0,
                   const char * str3=0, const char * str4=0,
                   const char * str5=0, const char * str6=0);

char * StrAdd(char * str,  const char * str0,
                   const char * str1=0, const char * str2=0,
                   const char * str3=0, const char * str4=0,
                   const char * str5=0, const char * str6=0);

char * SkipChars(char * c, const char * skip);

char * GetSubStr(char * Str1,size_t Start, size_t Len, char * Str2);

bool ExtractToken(char * Str, char * Token, char Ch);

//Unicode functions, in sy_string (where they get used the most)

///looks for high-bit non-utf-8 sequences in a string
bool IsANSI(const char* s);
///allocate 4x as many bytes as src. returns the number actually used
size_t ANSItoUTF8(const char* src, char* dest);
///returns the number of UTF-8 bytes
size_t UTF8ToUCS2C(const char* in,uint16 &c);
///returns the number of characters (not bytes) needed  (not including the final null)
size_t UTF8ToUCS2Length(const char* in,size_t max=UINT_MAX);
///returns the number of characters (not bytes) converted (not including the final null)
size_t UTF8ToUCS2(const char* in,uint16 * out,size_t max=UINT_MAX);
///returns the number of bytes needed to encode the letter
size_t UCS2ToUTF8C(unsigned in, char* out);
///returns the number of bytes needed (not including the final null)
size_t UCS2ToUTF8Length(const uint16* in,size_t max=UINT_MAX);
///returns the number of bytes written (not including the final null)
size_t UCS2ToUTF8(const uint16* in,char* out,size_t max=UINT_MAX);

// sy_form

/* Case-insensitive. Delimiters are , \t\r\n;| */
bool HasToken(const char* field, const char* code);

/* Following as case sensitive */
void SetMultResp(char * field, const char * code, size_t fieldlen=0);

void ClearMultResp(char * field,const char * code, size_t fieldlen=0);

bool HasMultResp(register const char * data, register const char * compare,
                 size_t datalength, size_t tokensize);

bool HasMultResp(const char * field, const char * code,
                 size_t fieldlen=0, int*loc=0);

void ToggleMultResp(char* field, const char * code, size_t fieldlen=0);

//bool ReplaceText(const char * fval,TStr & gridtext,TParameterList& responses);


// sy_time

/*!
fmt 0: dbf date and time, 9/7 bytes,    YYYYMMDD   HHMMSS
fmt 1: us date and time, 11/9 bytes,    DD/MM/YYYY HH:MM:SS
fmt 2: world date and time, 11/9 bytes, MM/DD/YYYY HH:MM:SS
fmt 3: unix time format, 22 bytes, 01 JAN 2000 13:02:12
fmt 4: unique stamp, 21 bytes, YYYYMMDDHHMMSS123456 in hex
fmt 5: 15 bytes YYYYMMDDHHMMSS
*/
void GetDateTime(char* Date,char * Time,int fmt=0,bool GMT=false);
#ifndef XP_WIN

typedef struct _SYSTEMTIME {  // st
    uint16 wYear;
    uint16 wMonth;
    uint16 wDayOfWeek;
    uint16 wDay;
    uint16 wHour;
    uint16 wMinute;
    uint16 wSecond;
    uint16 wMilliseconds;
} SYSTEMTIME;
#endif

/// input text is modified
void ParseDateTime(char* date, SYSTEMTIME& tm);

inline char * WhatIsToday(char * Today)
{ GetDateTime(Today,0,3); return Today;}

inline char * GetCurDate(char * Str)
{ GetDateTime(Str,0,0); return Str;}

inline char * GetCurTime(char * Str)
{ GetDateTime(0,Str,0);return Str;}

#endif
