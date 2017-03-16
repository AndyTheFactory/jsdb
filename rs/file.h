#ifndef _RS_FILE_H
#define _RS_FILE_H

#ifndef _RS_STRING_H
#include "rs/string.h"
#endif
#ifndef _RS_SYSTEM_H
#include "rs/system.h"
#endif
// file name management

//1 read, 2 write, 3 read-write, 4 directory
int FileAttributes(const char* filename);

void AddBackslash(char* temp);

char * GetFilename(char * filename);

inline const char * GetFilename(const char * filename)
 {return GetFilename((char*)filename);}

bool GetNewFilename(const char * ext,const char * destdir,TStr& filename);

long GetWildCardFileNames(const char * filespec, TStringList & strings);

char * GetFileBackupName(const char * SrcFileName, TStr& BackupName);

char * GetExtension(char* filename);

///chops off the extension and returns a pointer to the extension. For example:
///  "myfile.dat" ->  "myfile" "dat"
char * ClipExtension(char * filename);

void GetDirectory(const char * Filename,TStr& dir);

void MakeDirectoryExist(const char *DestDir);

///returns the title part of a file name
char * ClipFilename(char * filename);
///switches to the appropriate slash
void FixFilename(char* s);

inline const char * FileExtName(char * filename) {return GetExtension(filename);}

inline const char * FileFileName(char * filename) {return GetFilename(filename);}

// file and directory management

bool ChangeDirectory(const char * newdir);

bool CreateTempFile(TStr& s,const char * ext=0);

#ifndef XP_WIN
bool CopyFile(const char * source, const char * dest, bool DontErase);
// DontErase = false to replace the destination
#endif

bool AppendFile(const char * source, const char * dest);

// file information

long FileSize(const char * fname);
inline long GetFileSize(const char * fname) {return FileSize(fname);}

bool FileExists(const char * fname);
double  GetDiskFree(const char * FileName);

#endif
