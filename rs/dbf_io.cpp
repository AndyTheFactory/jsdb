#include "rslib.h"
#pragma hdrstop

#ifdef XP_UNIX
inline void wait1() {usleep(1000000);}
#elif defined (__MWERKS__) || defined(__GNUC__) || defined(__MSC__)
inline void wait1() {Sleep(1000);}
#else
void wait1() //wait a second for file locking
{
struct time start, end;
int endsec;

gettime(&start);
if (start.ti_sec == 59)
   endsec = 0;
else
   endsec = start.ti_sec + 1;

   do {gettime(&end);} while (end.ti_sec == start.ti_sec);
   while (end.ti_sec == endsec && end.ti_hund < start.ti_hund)
   gettime(&end);
}
#endif

long LongPntrWrite(const char * fname,long & FileOffset,
         long ByteCount, void * Mem, bool LockRange)
{  //write to end of file if FileOffset is negative
#ifdef XP_WIN
    SECURITY_ATTRIBUTES sec;
    sec.nLength = sizeof(sec);
    sec.lpSecurityDescriptor = NULL;
    sec.bInheritHandle = true;

    OSVERSIONINFO versioninfo = {sizeof(OSVERSIONINFO),0,0,0,0};
    GetVersionEx(&versioninfo);

   HANDLE File = INVALID_HANDLE_VALUE;

    if (versioninfo.dwPlatformId == VER_PLATFORM_WIN32_NT) //unicode?
     File = CreateFileW( WStr(fname), GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          &sec, OPEN_EXISTING,
                          FILE_FLAG_SEQUENTIAL_SCAN, 0);

    if (File == INVALID_HANDLE_VALUE)
     File = CreateFileA( fname, GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          &sec, OPEN_EXISTING,
                          FILE_FLAG_SEQUENTIAL_SCAN, 0);

if (File == INVALID_HANDLE_VALUE) return 0;

if (FileOffset<0) FileOffset = SetFilePointer(File,0,0,FILE_END);
     else FileOffset= SetFilePointer(File,FileOffset,0,FILE_BEGIN);

if (FileOffset == -1 ) return 0;
int tries = LockRange ? 30 : 0;
//tries 30 times to lock a record before giving up.
bool locked = false;
goto startloop;
while (tries && !locked)
{
 wait1();
 tries--;
startloop:
 locked = LockFile(File,FileOffset,0,ByteCount,0);
}
DWORD written;
WriteFile(File,Mem,ByteCount,&written,0);
if (locked) UnlockFile(File,FileOffset,0,ByteCount,0);
CloseHandle(File);
return written;
#else //non-windows must be a 32-bit OS

#ifdef XP_UNIX
 TStr nfn(fname);
 Replace(nfn,'\\','/');
 fname=nfn;
#endif

int fhandle=open((char*)fname,O_WRONLY,S_IREAD);
if (fhandle < 0) return 0;
if (FileOffset < 0) FileOffset = lseek(fhandle,0,SEEK_END);
 else FileOffset=lseek(fhandle,FileOffset,SEEK_SET);
long ret= write(fhandle,Mem,ByteCount);
close(fhandle);
return ret;
#endif

};

long LongPntrRead(const char * fname,long &FileOffset, long ByteCount, void * Mem)
{
#ifdef XP_WIN
    OSVERSIONINFO versioninfo = {sizeof(OSVERSIONINFO),0,0,0,0};
    GetVersionEx(&versioninfo);
    HANDLE File = INVALID_HANDLE_VALUE;

    if (versioninfo.dwPlatformId == VER_PLATFORM_WIN32_NT) //unicode?
     File = CreateFileW( WStr(fname), GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          0, OPEN_EXISTING,
                          FILE_FLAG_SEQUENTIAL_SCAN, 0);

    if (File == INVALID_HANDLE_VALUE)
     File = CreateFileA( fname, GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          0, OPEN_EXISTING,
                          FILE_FLAG_SEQUENTIAL_SCAN, 0);

if (File == INVALID_HANDLE_VALUE) return 0;
if (FileOffset) FileOffset = SetFilePointer(File,FileOffset,0,FILE_BEGIN);
if (FileOffset == -1 ) return 0;

//LockFile(File,FileOffset,0,ByteCount,0);
DWORD written=0;
ReadFile(File,Mem,ByteCount,&written,0);
//UnlockFile(File,FileOffset,0,ByteCount,0);
CloseHandle(File);
return written;
#else
#ifdef XP_UNIX
 TStr nfn(fname);
 //Replace(nfn,'\\','/');
 fname=nfn;
#endif
int fhandle=open((char*)fname,O_RDONLY,S_IREAD);
if (fhandle < 0) return 0;
FileOffset=lseek(fhandle,FileOffset,SEEK_SET);
long ret= read(fhandle,Mem,ByteCount);
close(fhandle);
return ret;
#endif
};
