#include <stdio.h>

struct aligner { char c; void* a; };

int main(int argc, char** argv)
{
  printf("#ifndef js_config_h__\n");
  printf("#define js_config_h__\n");
  int bpw = sizeof(void*);
  printf("#define JS_BYTES_PER_WORD %d\n",bpw);

  int sow = sizeof(double);
  printf("#define JS_BYTES_PER_DOUBLE %d\n",sow);

  if (bpw == 4)
  printf("#define JS_BITS_PER_WORD_LOG2 5\n");
  else if (bpw == 8)
  {
  printf("#define JS_BITS_PER_WORD_LOG2 6\n");
  printf("#define AVMPLUS_64BIT\n");
  }
  aligner al;
  int aop = (char*)(void*)(&al.a) - (char*)(void*)(&al.c);
  printf("#define JS_ALIGN_OF_POINTER %d\n",aop);

#ifdef _MSC_VER
  printf("#define JS_HAVE___INTN 1\n");
  printf("#define JS_STDDEF_H_HAS_INTPTR_T 1\n");
#endif
#ifdef WIN32
  printf("#define HAVE_GETSYSTEMTIMEASFILETIME 1\n");
  printf("#define HAVE_SYSTEMTIMETOFILETIME 1\n");
#elif defined WIN64
  printf("#define HAVE_GETSYSTEMTIMEASFILETIME 1\n");
  printf("#define HAVE_SYSTEMTIMETOFILETIME 1\n");
#else
  printf("#ifdef XP_MACOSX\n");
  printf("#define HAVE_VA_LIST_AS_ARRAY 1\n");
  printf("#endif\n");
#endif
#ifdef __sun__
/* in js/src/jsstdint.h, insert:
#if defined(JS_HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(JS_HAVE_STDINT_H)
#include <stdint.h>
*/
  printf("#define JS_HAVE_INTTYPES_H\n");
#else
  printf("#define JS_HAVE_STDINT_H\n");
#endif
//  printf("#define JS_TRACER\n");
//  printf("#define FEATURE_NANOJIT\n");
  printf("#define STATIC_JS_API\n");
  printf("#endif\n");
  printf("#define _vprof(v)\n");
  printf("#define _nvprof(n,v)\n");
  printf("#define _hprof(h)\n");
  printf("#define _nhprof(n,h)\n");

}

#if 0
js-config.h


/* Define to 1 if SpiderMonkey should support multi-threaded clients.  */
#undef JS_THREADSAFE

/* Define to 1 if SpiderMonkey should support the ability to perform
   entirely too much GC.  */
#undef JS_GC_ZEAL

/* Define to 1 if the standard <stdint.h> header is present and
   useable.  See jstypes.h and jsstdint.h.  */
#undef JS_HAVE_STDINT_H

/* Define to 1 if the N-byte __intN types are defined by the
   compiler.  */
#undef JS_HAVE___INTN

/* Define to 1 if #including <stddef.h> provides definitions for
   intptr_t and uintptr_t.  */
#undef JS_STDDEF_H_HAS_INTPTR_T

/* Define to 1 if #including <crtdefs.h> provides definitions for
   intptr_t and uintptr_t.  */
#undef JS_CRTDEFS_H_HAS_INTPTR_T

/* The configure script defines these if it doesn't #define
   JS_HAVE_STDINT_H.  */
#undef JS_INT8_TYPE
#undef JS_INT16_TYPE
#undef JS_INT32_TYPE
#undef JS_INT64_TYPE
#undef JS_INTPTR_TYPE
#undef JS_BYTES_PER_WORD

/* Some mozilla code uses JS-friend APIs that depend on JS_TRACER being
   correct. */
#undef JS_TRACER

#endif
#if 0
jscpucfg.h

32-bit
#define IS_LITTLE_ENDIAN 1
#undef  IS_BIG_ENDIAN
#define JS_BYTES_PER_DOUBLE 8L
#define JS_BYTES_PER_WORD   4L
#define JS_BITS_PER_WORD_LOG2   5
#define JS_ALIGN_OF_POINTER 4L

64-bit
#define JS_BYTES_PER_WORD   8L
#define JS_BITS_PER_WORD_LOG2   6
#define JS_ALIGN_OF_POINTER 8L

stdint.h
typedef signed char int8_t;
typedef unsigned char   uint8_t;
typedef short  int16_t;
typedef unsigned short  uint16_t;
typedef int  int32_t;
typedef unsigned   uint32_t;
typedef long long  int64_t;
typedef unsigned long long   uint64_t;
#endif
