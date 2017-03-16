#include "rslib.h"
#pragma hdrstop
#ifdef XP_WIN
size_t TParameterList::ReadRegistrySection(HKEY base,const char * section)
{
   HKEY Key;
   DWORD type;
   char name[1024];
   char value[1024];

   long ret;
   ret = RegOpenKeyEx(base,section,0,KEY_READ,&Key);

   if (ret != ERROR_SUCCESS) return 0;

    DWORD i=0;

   while (1)
   {
    DWORD n=1024;
    DWORD v=1024;
     value[0]=name[0]=0;
    DWORD e = RegEnumValue(Key,i,name,&n,0,&type,(BYTE*)value,&v);
    if (e !=ERROR_SUCCESS) break;

    Set(name,value);

    i++;
   }

  RegCloseKey(Key);
  return i;
}

void TParameterList::WriteRegistrySection(HKEY base,const char * section)
{
 HKEY Key;
 int ret;
 DWORD disp=0;

 ret = RegCreateKeyEx(base,section,0,"",REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,0,&Key,&disp);
 if (ret !=ERROR_SUCCESS) return;

 FOREACH(TNameValuePair*n,Items)
  DWORD len=strlen(n->Value);
  ret |= RegSetValueEx(Key, n->Name, 0,REG_SZ,(BYTE*)(char*)n->Value,len);
 DONEFOREACH

 RegCloseKey(Key);

 return;
}

bool IsRegKeySet(const char * section, const char * Mykey, HKEY base)
  {
   HKEY Key;
   char out[2000];
   DWORD len=2000, type;

   long ret;

   ret = RegOpenKey(base, section, &Key);
   if (ret != ERROR_SUCCESS) return false;

   ret = RegQueryValueEx(Key, Mykey,0,&type,(LPBYTE)out,&len);
   RegCloseKey(Key);
   return (ret == ERROR_SUCCESS);
  }


bool RegSetKey(const char * section, const char * Mykey, const char* value,HKEY base)
{
 HKEY Key;
 int ret;
 DWORD disp=0;

 ret = RegCreateKeyEx(base,section,0,"",REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,0,&Key,&disp);
 if (ret !=ERROR_SUCCESS) return false;

 ret = RegSetValueEx(Key, Mykey, 0,REG_SZ,(BYTE*)value,strlen(value));

 RegCloseKey(Key);

 return ret==ERROR_SUCCESS;
}

bool RegGetKey(const char * section, const char * key, TStr& out,
               HKEY base)
  { //website
   HKEY Key;
   DWORD end=0,len=0,type;
   long ret;
   ret = RegOpenKey(base,
        section,
        &Key);

   if (ret != ERROR_SUCCESS) return false;


   ret = ERROR_MORE_DATA;

   while (ret == ERROR_MORE_DATA )
   {
    len += MAXPATH;
    out.Resize(len);
    end = len;
    ret = RegQueryValueEx(Key,key,0,&type,(LPBYTE)(char*)out,&end);
   }

   out[end] = 0;
   RegCloseKey(Key);

   return (ret == ERROR_SUCCESS);
  }

HINSTANCE RSLoadLibrary(const char * path)
{
    char olddir[MAXPATH];
    GetCurrentDirectory(MAXPATH,olddir);
    TStr dir;
    GetDirectory(path,dir);

    ChangeDirectory(dir);
    HINSTANCE h = LoadLibrary(path);
    ChangeDirectory(olddir);

    return h;
}

#endif
