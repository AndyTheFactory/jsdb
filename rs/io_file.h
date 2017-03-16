#ifndef _RS_FILESTREAM_H
#define _RS_FILESTREAM_H
#ifdef XP_UNIX
#include <unistd.h>
#include <sys/types.h>
#endif

class FileStream : public Stream
{
public:
#ifdef XP_WIN
 HANDLE File; // let the OS do any buffering
#else
 int File;  // system buffers
#endif
protected:
 bool IsText;
 MemoryStream *Buffer;
 enum EBufMode {BufNone,BufRead,BufWrite} BufMode;
 void FlushCache(EBufMode mode);
 long ReadPosition;
 bool canClose;
 long EndFileMarker;
 TStr FileName;
public: //open in read-write mode
 FileStream ();
 FileStream (const char * filename, TOpenMode OpenMode=OMDefault,TType Type=ReadOnly);
// TFileStream (const char * filename,TType Type, bool overwrite=true,
//              bool textmode=false, bool buffered = true);
 void Init(const char * filename,TType Type,bool textmode, bool buffer);
 // Init() can throw an exception
 // if Type == ReadOnly the file must already exist!

 ~FileStream();

 virtual long size();
 virtual long pos();
 virtual bool eof();
 virtual long goforward(long delta);//seeks forward
 virtual long putback(long delta);
 virtual long seek(long offset);
 virtual const char* filename() {return FileName;}

 virtual int read(char * dest,int maxcopy);
 virtual int write(const char * src,int maxcopy);

 bool IsValid() {return File != 0;}

 void SetEndMarker(long i) {EndFileMarker = i;}

#ifdef XP_WIN
 BOOL GetFileTime(FILETIME&f) {return ::GetFileTime(File,&f,0,0);}
 BOOL SetFileTime(FILETIME&f) {return ::SetFileTime(File,&f,&f,&f);}
 DWORD GetFileAttributes() {return ::GetFileAttributes(FileName);}
 BOOL SetEndOfFile(long position)
  {seek(position); return ::SetEndOfFile(File); }
#else
 bool SetEndOfFile(long position)
  {seek(position); return (ftruncate(File,position)==0); }
#endif
};

#endif
