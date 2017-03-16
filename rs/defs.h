#ifndef _RS_DEFS_H
#define _RS_DEFS_H

#include <string.h>

/* include standard files */
#ifdef XP_WIN
#include <io.h>
// hack because of WIN32_LEAN_AND_MEAN=1 defined in tracemonkey
#undef _WINSOCKAPI_
#include <winsock2.h>
#endif
#include <errno.h>

#ifdef __MWERKS__
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <cstddef>
#include <cmath>
#include <ctype>
#include <c:\cw5\msl\msl_c\msl_common\include\wchar.h>
#else
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef XP_WIN
#include <share.h>
#else
#include <sys/file.h>
#endif
#include <ctype.h>
#endif

#ifdef __BORLANDC__
#define BI_NAMESPACE
#include <dos.h>
#include <dir.h>
#include <except.h>
//#include <cstring.h>
#include <checks.h>
#endif

#ifdef __MSC__
#ifdef XP_WIN

#define _WINSOCK2API_
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */

#include <winsock2.h>
#endif
#endif

#ifdef __GNUC__
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#endif

#ifndef TYPESAFE_DOWNCAST
#define TYPESAFE_DOWNCAST(object,toClass) dynamic_cast<toClass * >(object)
#endif

/* useful macros, like FOREACH */
// For MSC, change the precompiled header mode to automatic (/YX)

#define LMD(a,b,c) ((int)((long)(a) * (long)(b) / (long)(c)))
#define DMD(a,b,c) ((double)(a) * (double)(b) / (double)(c))
#define DDIV(a,b) (b == 0.0 ? 0.0 : (double)(a)/(double)(b))
#define PERCENT(x,y) ((y) > 0 ? double(x) * 100.0 / double(y) : 0)

#define SETBIT(i,bit) (((int)i) |= (1<<bit))
#define CLEARBIT(i,bit) (((int)i) &= ~(1<<bit))
#define GETBIT(i,bit) (((int)i) & (1<<bit))
#define GETFLAG(i,flag) ((((int)i) & (int)(flag)) != 0)

#define COPYSIZE(s1,s2) s1.cx=s2.cx; s1.cy=s2.cy
#define COPYPT(s1,s2) s1.x=s2.x; s1.y=s2.y
#define COPYRECT(d,s) dest.Set(s.left,s.top,s.right,s.bottom);

#define SWAP(T,a,b) {T c = a; a = b; b = c;}
#define _SWAP(a,b) { a = a^b; b = a^b; a=a^b; }
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)<(b)?(b):(a))

#ifdef XP_UNIX
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)<(b)?(b):(a))
#endif

#ifndef XP_WIN
typedef void* HANDLE;
#define LOWORD(x) ((int)(x) & 0xffff)
#define LOBYTE(w) ((BYTE)(w))
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#ifdef _HAS_INT64
typedef int64 __int64;
#endif
#define wsprintf sprintf
extern "C" long GetTickCount();
extern "C" int stricmp(const char *s1, const char *s2);
extern "C" int strnicmp(const char *s1, const char *s2, size_t n);
#endif

/* erroneous functions */
#ifdef __BORLANDC__
#undef isspace
#define isspace(x) IsSpace(x)
#endif

#ifdef __MSC__
#define strnicmp _strnicmp
#define strncmpi _strnicmp
#endif

#endif
