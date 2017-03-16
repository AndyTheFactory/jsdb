#include "rslib.h"
#pragma hdrstop
#undef isdigit
#include <ctype.h>

#ifndef __BORLANDC__
#ifdef _HAS_INT64
//after the technique in Borland's LONGTOA.c
char* __int64tot (int64 value, char *out, int radix, bool isSigned, char hexStyle)
{
    char buf [66];
    char c, *p, *bufp = buf;

    p = out;

    if (radix >= 2 && radix <= 36)
    {
        if (isSigned && value < 0)
        {
            *p++ = ('-');
            value = -value;
        }

        while(1)
        {
            *bufp++ = (char)((uint64)value % radix);
            if ((value = (uint64)value / radix) == 0)
                break;
        }

        while (bufp != buf)
        {
            if ((c = *--bufp) < 10)
                *p++ = c + ('0');
            else
                *p++ = (c - 10) + hexStyle;
        }
    }

    *p = 0;
    return out;
}

#ifndef __MSC__
int64 _atoi64(const char *strP)
{
    char c;
    bool neg ;
    int64 result = 0;

    while (isspace(*strP)) strP++;

    c = *strP;
    if (c == '+' || c == '-')
    {
        neg = c == '-';
        c = *strP++;
    }
    else
        neg = false;

    while (c >= '0' && c <= '9')
    {
        result = result * 10 + c - '0';
        c = *strP++;
    }

    return neg ? -result : result;
}
#endif
#endif

char * _longtot (long value, char *out, int radix,
                        bool maybeSigned, char hexStyle)
{
    char buf [34];
    char c, *p = out, *bufp = buf;

    if (radix >= 2 && radix <= 36)
    {

        if (value < 0 && maybeSigned)
        {
            *p++ = '-';
            value = -value;
        }

        while (1)
        {
            *bufp++ = (char)((unsigned long)value % radix);
            if ((value = (unsigned long)value / radix) == 0)
                break;
        }

        while (bufp != buf)
        {
            if ((c = *--bufp) < 10)
                *p++ = c + ('0');
            else
                *p++ = (c - 10) + hexStyle;
        }
    }

    *p = 0;
    return out;
}

#undef _TCHAR

#ifdef _HAS_INT64
char *_i64toa(int64 value, char* string, int radix)
{
     return __int64tot( (radix==10)?(int64)value:(int64)((uint64)value),string,radix,(radix==10),'a');
}

char *_ul64toa(int64 value, char* string, int radix)
{
     return __int64tot( value,string,radix,0,'a');
}
#endif

char *itoa(int value, char* string, int radix)
{
     return _longtot( (radix==10)?(long)value:(long)((unsigned)value),
                                        string,radix,(radix==10),'a');
}

char *ultoa(int value, char* string, int radix)
{
     return _longtot( value,string,radix,false,'a');
}
#endif //borlandc

#ifndef XP_WIN
size_t ucslen(const uint16 *__s)
{
    size_t len = 0;

    while ( __s[len] != 0)
        len++;
    return len;
}

uint16 * ucscpy(uint16 *__dst, const uint16 *__src)
{
    size_t len, j;

    len = ucslen (__src);

    for (j = 0;j< len;j++)
    {
        __dst[j] = __src[j];
    }
    __dst[len] = 0;
    return __dst;
}
uint16 *ucscat(uint16 *__dest, const uint16 *__src)
{
    size_t len;

    len = ucslen (__dest);
    ucscpy (__dest+len, __src);
    return __dest;
}
#endif //windows

int RemoveChar(char * str, char remove, int length)
 {
  int count = 0;
  char * c = str;

  for (int i=0; i<length; i++)
    {
     if (remove == c[i]) count++;
     else *str++ = c[i];
    }
  return count;
 }

bool ExtractToken(char * Str, char * Token, char Ch)
     {
       char * Temp = strchr(Str,Ch);
       if (!Temp) return false;   //   Ch not in Str
       Temp[0] = 0; Temp++;
       strcpy(Token,Str);     //  First part goes to Token
       strcpy(Str,Temp);      //  Second part goes to Str
       return true;
     }

const char * strnchr(const char * c, int comp, size_t length)
{
 for (size_t i = 0; i < length; i++)
  if (c[i]==comp) return c+i;
 return 0;
}

const char * _stristr(const char *s1, const char *s2)
{
 size_t l2 = strlen(s2);
 while (*s1)
 {
  if (strncasecmp(s1,s2,l2) == 0) return s1;
  s1++;
 }
 return 0;
}

bool IsBlank( const char * c)
    {
      while (*c) if (*c++ != ' ') return false;
      return true;
    }

bool IsWhitespace(const char * Str, size_t length)
    {
      while (length && *Str)
       {
        if (!isspace(*Str)) return false;
        Str++;length--;
       }
      return *Str == 0;
    }

char * StripPunct(char * Str)
   {

     int J = 0, I = 0;
     while (Str[I])
         {
           if (!isspace(Str[I]) && !ispunct(Str[I]))
              {
                Str[J] = Str[I];
                J++;
              }
           I++;
         }
     Str[J] = 0;
     return Str;
   }

char * BStripB(char * Str, bool NoLeft)   //  Strip Blanks from both ends
     {
         int I = strlen(Str)-1;
         while ( I >= 0 && ( isspace(Str[I]) ) )
                {
               I--;
             }
       I++;  Str[I] = 0;
       if ( NoLeft || Str[0] != ' ') return Str;
       I = 0;
       int J = 0;
         while (Str[I] != 0)
            { if ( J > 0 || !isspace(Str[I]) )
               {
                    Str[J] = Str[I]; J++;
               }
            I++;
            }
        Str[J] = 0;
       return  Str;
     }

char * AlphaNum(char * Str)     //  Strip non AlphaNumeric chars
     {
       int I = 0;
       int J = 0;
         while (Str[I] != 0)
            {
            if ( isalnum(Str[I]) )
               {
                    Str[J] = Str[I];
                 J++;
               }
            I++;
            }
        Str[J] = 0;
       return Str;
     }


char * RsStrcat(char * str,  const char * str0,
                   const char * str1, const char * str2 ,
                   const char * str3, const char * str4,
                   const char * str5, const char * str6)
{
  strcat(str,TStr(str0,str1,str2,str3,str4,str5,str6,0));
  return str;
}

char * StrAdd(char * str,   const char * str0,
                    const char * str1, const char * str2 ,
                    const char * str3, const char * str4,
                    const char * str5, const char * str6)
{
  strcpy(str,TStr(str0,str1,str2,str3,str4,str5,str6,0));
  return str;
}

char *   RightJust(char * Str, int Len)
     {
       char X[200];
       int I = Len - strlen(Str);
       if ( I <= 0 || Len > 150) return Str;
       memset(X,' ',I+4);
       char * Temp = X + I;
       strcpy(Temp,Str);
       strcpy(Str,X);
       return Str;
     }

char *   RsStrncpy(char * Dest, const char * Src, int Len, bool IsPad)
      {
        strncpy(Dest,Src,Len);
        if (IsPad)
           {
             int J = strlen(Dest);
             while ( J < Len)
                 {
                   Dest[J] = ' ';
                   J++;
                 }
           }
        Dest[Len] = 0;
        return Dest;
      }

char *  StripChar(char * Str, const char * pat)
      {
       int I=0, J=0, K ;
         K = strlen(pat);
         while (Str[I] != 0)
              {
                  for (int L = 0; L < K; L++)
                       {
                     if (Str[I] == pat[L]) goto AT1;
                   }
                  Str[J] = Str[I]; J++;
AT1:           I++;
              }
        Str[J] = 0;
       return Str;
      }
/*
void swapchars(char * str, size_t len,char find, char repl)
{
    register size_t x;
    for(x=0;str[x]&& x<len;x++) if(str[x] == find) str[x] = repl;
}
*/

void Replace(char * str, char old, char n)
{
 char x[2];
 x[0]=old;
 x[1]=0;
 Replace(str,x,n);
}

void Replace(char * str, const char* remove, char replace)
{
 while (*str)
 {
  if (strchr(remove,*str)) *str = replace;
  str++;
 }
}

char * GetSubStr(char * Str1,size_t Start, size_t Len, char * Str2)
{
 if (strlen(Str1) > Start)
 {
    strncpy(Str2,Str1+Start,Len);
    Str2[Len]=0;
 }
 else Str2[0]=0;
 return Str2;
}

void RsCommaNumber(char * Str)
   {
     TStr aString(Str);
     int K  = strlen(Str) - 3;
     char * Temp = strchr(Str,'.');
     if (Temp)
        {
          K = Temp - Str - 3;
        }
     if (K <= 0) return;  // safety
     for (int I = K; I > 0; I -= 3)
        {
          aString.insert(I,",");
        }
     strcpy(Str,aString.c_str());
   }

char * StripCharsFB(char * c, const char * rem)
{
 return StripCharsFB(c,rem,strlen(c));
}


char * StripCharsFB(char * c, const char * rem,size_t j)
{
 int i;
 while (*c && j)
  {
   if (strchr(rem,*c)) {c++; j--;}
   else break;
  }
 for (i = j-1; i >=0; i--)
 {
   if (strchr(rem,c[i])) c[i]=0;
   else break;
 }
 return c;
}


bool  StartsWith(const char * Str1, const char * Str2, int ToUpper)
     {
       int I = 0;
       while (Str1[I] && Str2[I])
          {
            if (!ToUpper)
               {
                 if (Str1[I] != Str2[I]) return false;
               }
            else
               {
                 if (toupper(Str1[I]) != toupper(Str2[I]) ) return false;
               }
            I++;
          }
       return true;
     }



