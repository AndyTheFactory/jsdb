#include "rslib.h"
#pragma hdrstop

xdb::xdb():w((char*)NULL),i((char*)NULL)
{
};

xdb::xdb(const xdb& o): w(o.w),i(o.i)
{
}

xdb::xdb(const xdb* o): w(o->w),i(o->i)
{
}


xdb::xdb(const char * message,const char * a,const char* b,
                              const char * c,const char* d,
                              const char * e,const char* f)
   : w(message),i(a,"=",b,",",c,"=",d,e?",":0,e,"=",f)
 {
 }

xdb::xdb(const char * error,const char * a,const char* b,
                          const char * c,EXCEPTION_LONG  d,
                          const char * e,EXCEPTION_LONG  f)
   : w(error)
 {
  size_t l = 128+(a?strlen(a):0 )+ (b?strlen(b):0) + (c?strlen(c):0)+(e?strlen(e):0);
  i.Resize(l);
  l = wsprintf((char*)i,"%s=%s",a,b);
  if (c)
   l += wsprintf(((char*)i)+l,",%s=%ld",c,(long)d);
  if (e)
   wsprintf(((char*)i)+l,",%s=%ld",e,(long)f);
 }


xdb::xdb(const char * error,const char * a,EXCEPTION_LONG  b,
                          const char * c,EXCEPTION_LONG  d,
                          const char * e,EXCEPTION_LONG  f,
                          const char * g,EXCEPTION_LONG  h)
   : w(error)
 {
  size_t l = 128+(a?strlen(a):0) + (c?strlen(c):0) + (e?strlen(e):0)+(g?strlen(g):0);
  i.Resize(l);
  l = wsprintf((char*)i,"%s=%ld",a,(long)b);
  if (c)
   l += wsprintf(((char*)i)+l,",%s=%ld",c,(long)d);
  if (e)
   wsprintf(((char*)i)+l,",%s=%ld",e,(long)f);
  if (g)
   wsprintf(((char*)i)+l,",%s=%ld",g,(long)h);
 }


//void xdb::operator += (const xdb& o)
// {
//  w=TStr(w,*w?"\n":"",o.w);
//  i=TStr(i,*i?"\n":"",o.i);
// }


xdb::xdb(const char * message,const char* info)
   : w(message),i(info)
 {
 }

/*
void xdb::raise()
 {
  throw *this;
 }*/

xdb::~xdb()
 {

 }
