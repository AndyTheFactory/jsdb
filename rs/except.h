#ifndef _RS_EXCEPT_H
#define _RS_EXCEPT_H

#include "rs/defs.h"
#include "rs/string.h"

#ifdef _HAS_INT64
#define EXCEPTION_LONG int64
#else
#define EXCEPTION_LONG long
#endif

class xdb
 {public:

   TStr w;//error name
   TStr i;//comma-separated parameters

   xdb(const char * error,const char * info=0);
   xdb(const char * error,const char * a,const char* b,
                          const char * c=0,const char* d=0,
                          const char * e=0,const char* f=0);

   xdb(const char * error,const char * a,const char* b,
                          const char * c,EXCEPTION_LONG  d,
                          const char * e=0,EXCEPTION_LONG  f=0);

   xdb(const char * error,const char * a,EXCEPTION_LONG  b,
                          const char * c=0,EXCEPTION_LONG  d=0,
                          const char * e=0,EXCEPTION_LONG  f=0,
                          const char * g=0,EXCEPTION_LONG  h=0);

   xdb();

   xdb(const xdb& o);

   xdb(const xdb* o);

   const char* why() {return w;}
   const char* info() {return i;}

   void operator = (const xdb& o) {w=o.w; i=o.i;}
   operator bool() {return *(char*)(w) != 0;}
   void operator = (int) {w=(char*)NULL; i=(char*)NULL;}
   ~xdb();
  };

#endif
