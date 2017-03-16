#ifndef _RS_DEFS_H
#include "rs/defs.h"
#endif

#ifndef _RS_STRING_H
#include "rs/string.h"
#endif

#ifndef _RS_FILE_H
#include "rs/file.h"
#endif

#ifdef XP_WIN
int    GetVolumeNumber();

bool RegGetKey(const char * section, const char * key, TStr& out,
               HKEY base=HKEY_LOCAL_MACHINE);

bool RegSetKey(const char * section, const char * key,
               const char* value,HKEY base=HKEY_LOCAL_MACHINE);

bool IsRegKeySet(const char * section, const char * key,
                 HKEY base=HKEY_LOCAL_MACHINE);

// section = SOFTWARE\\Raosoft\\EZSurvey\\blah...

// module management

HINSTANCE RSLoadLibrary(const char * path);

#endif
