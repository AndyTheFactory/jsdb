#include "rslib.h"
#pragma hdrstop

char * AllocStr(size_t len)
 {
  char * c;
  
 try {
  /* align on 4-byte boundary, plus a few extra */
  c = new char[(len & (~0x03))+8];
  } catch(...) {c=0;}

  if (!c) throw xdb("Out of memory","size",len);

  c[0]=0;
  memset(c+len,0,4);
  return c;
 }

char * AllocStr(const char * s)
 {
  size_t len = s ? strlen(s) : 0;
  char * c = AllocStr( len);
  if (len) memcpy(c,s,len);
  return c;
 }


