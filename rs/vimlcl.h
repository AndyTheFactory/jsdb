#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* VIMLCL.H includes environment-specific local binding definitions     */
/*                                                                      */
/* Define one of the following symbols before including this file       */
/* (typically in your makefile or on the compiler's command line):      */
/*                                                                      */
/*      VIM_USE_DOS         for DOS                                     */
/*      VIM_USE_MSWIN       for Microsoft Windows                       */
/*      VIM_USE_OS2_16      for 16-bit OS/2                             */
/*      VIM_USE_OS2_32      for 32-bit OS/2 (IBM 32-bit OS/2 2.0 C)     */
/*      VIM_USE_MSWIN_NT    for Microsoft Windows/NT                    */
/*      VIM_USE_MAC         for Macintosh                               */
/*      VIM_USE_UNIX        for Unix                                    */
/*      VIM_USE_NLM         for Netware NLM                             */
/*                                                                      */
/************************************************************************/

#ifndef __VIMLCL_H
#define __VIMLCL_H

#if defined(VIM_USE_MSWIN) || defined(VIM_USE_OS2_16) || defined(VIM_USE_DOS)

#define VIMAPIENTRY far pascal
#define VIMCALLBACK VIMAPIENTRY *
#define VIM_FAR far
#define VIM_VOID void
#define VIM_PTR

#elif defined(VIM_USE_OS2_32)

#if defined(__BORLANDC__)

#define VIMAPIENTRY
#define VIMCALLBACK VIMAPIENTRY *
#define VIM_FAR
#define VIM_VOID void
#define VIM_PTR

#else  /* !__BORLANDC__ -- assume IBM or equivalent */

#define VIMAPIENTRY _System
#define VIMCALLBACK * VIMAPIENTRY
#define VIM_FAR
#define VIM_VOID void
#define VIM_PTR

#endif  /* !__BORLANDC__ */

#elif defined(VIM_USE_OS2_32_16BIT)

#if defined(__BORLANDC__)

#define VIMAPIENTRY _far16 _pascal
#define VIMCALLBACK VIMAPIENTRY *
#define VIM_FAR _far16
#define VIM_VOID void
#define VIM_PTR

#else  /* !__BORLANDC__ -- assume IBM or equivalent */

#define VIMAPIENTRY _Far16 _Pascal
#define VIMCALLBACK * VIMAPIENTRY
#define VIM_FAR
#define VIM_VOID void
#define VIM_PTR _Seg16

#endif  /* !__BORLANDC__ */

#elif defined(VIM_USE_MSWIN_NT)

#define VIMAPIENTRY _stdcall
#define VIMCALLBACK VIMAPIENTRY *
#define VIM_FAR
#define VIM_VOID void
#define VIM_PTR

#elif defined(VIM_USE_MAC) || defined(VIM_USE_UNIX) || defined(VIM_USE_NLM)

#define VIMAPIENTRY
#define VIMCALLBACK VIMAPIENTRY *
#define VIM_FAR
#define VIM_VOID void
#define VIM_PTR

#else

#error VIM - Environment Symbol such as VIM_USE_MSWIN was not defined, no environment is set.

#endif

#endif  /* __VIMLCL_H */

#ifdef __cplusplus
}
#endif

