#include "rslib.h"

#ifdef XP_WIN
class AnonymousStream: public Stream
{public:
 HANDLE hread, hwrite;
 AnonymousStream(HANDLE r, HANDLE w);
 ~AnonymousStream();

 virtual int read(char * dest,int maxcopy);
 virtual int write(const char * src,int maxcopy);
 virtual bool canwrite();
 virtual bool canread();
 bool eof() {return !canwrite() && !canread();}
};

class PipeStream: public Stream
{public:
  STARTUPINFO sinfo;
  PROCESS_INFORMATION pinfo;
  HANDLE p1i,p1o,p2i,p2o,p3i,p3o;
  HANDLE hread, hwrite,herr;
  SECURITY_ATTRIBUTES sa;

  TStr FileName;
  TPointer<AnonymousStream> StdErr;

  PipeStream(const char* fn,bool detached=true);
  ~PipeStream();

  virtual int read(char * dest,int maxcopy);
  virtual int write(const char * src,int maxcopy);
  virtual bool canwrite();
  virtual bool canread();
  bool eof() {return !canwrite() && !canread();}

  const char * filename() {return FileName;}
};
#endif
#ifdef XP_UNIX

class AnonymousStream: public Stream
{public:
 int hread, hwrite;
 AnonymousStream(int r, int w);
 ~AnonymousStream();

 virtual int read(char * dest,int maxcopy);
 virtual int write(const char * src,int maxcopy);
 virtual bool canwrite();
 virtual bool canread();
 bool eof() {return !canwrite() && !canread();}
};

class PipeStream: public Stream
{public:
  int sin[2], sout[2], serr[2]; //read end, write end
  pid_t pid;

  TStr FileName;
  TPointer<AnonymousStream> StdErr;

  PipeStream(const char* fn,bool detached=true);
  ~PipeStream();

  int read(char * dest,int maxcopy);
  int write(const char * src,int maxcopy);
  bool canwrite();
  bool canread();
  bool eof() {return !canwrite();}

  const char * filename() {return FileName;}
};
#endif
