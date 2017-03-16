#include "rslib.h"
#pragma hdrstop

#ifdef XP_UNIX
#include <termios.h>
#include <unistd.h>

#include <sys/wait.h>
PipeStream::PipeStream(const char* fn,bool detached) : Stream(Stream::ReadWrite),
FileName(fn)
{
 if (pipe(sin)) goto error;
 if (pipe(sout)) goto error;
 if (pipe(serr)) goto error;

 pid = fork();
 if (pid == -1) goto error;
 if (pid) //parent
 {
  StdErr = new AnonymousStream(serr[0],0);
  close(sin[0]);
  close(sout[1]);
  close(serr[1]);
 }
 else
 {
  //sout[0] = read end of stdout
  dup2(sin[0], 0); // child stdin is the read end.
  dup2(sout[1],1); // child stdout is the write end.
  dup2(serr[1],2); // child stderr is the write end.

    struct termios old;
    //ioctl(1, TCGETA, &old);
    tcgetattr(STDOUT_FILENO,&old);
    old.c_lflag &= ~(ICANON);
    //old.c_iflag &= ~(ISTRIP | INPCK);
    //ioctl(1, TCSETAW, &old);
    tcsetattr(STDOUT_FILENO,0,&old);

  execl("/bin/sh", "sh", "-c", (char*)FileName, (char *)0);
  //execlv(fn);

  _exit(0);
 }
 return;

error:
   throw xdb("unable to open a pipe",fn);
}

PipeStream::~PipeStream()
{
  close(sin[1]);
  close(sout[0]);
  close(serr[0]);
}

bool canread(int s)
{
    fd_set incoming;
    FD_ZERO(&incoming);
    FD_SET(s,&incoming);

    timeval t;
    t.tv_sec=0;
    t.tv_usec = 0;

    if (select(s+1,&incoming,0,0,&t) <= 0) return false;
    return FD_ISSET(s,&incoming);
}

AnonymousStream::AnonymousStream(int r, int w): hread(r), hwrite(w),
   Stream( (TType)((r?ReadOnly:0) | (w?WriteOnly:0)))
{
}

AnonymousStream::~AnonymousStream()
{
}

bool AnonymousStream::canread()
{
    return hread ? ::canread(hread) : false;
}

int AnonymousStream::read(char * dest,int maxcopy)
{
 if (!hread) return 0;
 int n = ::read(hread,dest,maxcopy);
 if (n) return n;
 if (maxcopy && ::canread(hread))
  hread = 0;
 return 0;
}

bool AnonymousStream::canwrite()
{
 return (hwrite != 0);
}

int AnonymousStream::write(const char * dest,int maxcopy)
{
 return hwrite? ::write(hwrite,dest,maxcopy) : 0;
}

bool PipeStream::canread()
{
    if (Type == Stream::NotOpen) return false;
    return ::canread(sout[0]);
}

int PipeStream::read(char * dest,int maxcopy)
{
    int n = ::read(sout[0],dest,maxcopy);
    if (n) return n;
    if (maxcopy && canread()) //at EOF, read() returns 0 and select() returns true
     Type = Stream::NotOpen;
    return 0;
}

int PipeStream::write(const char * src,int maxcopy)
{
    return ::write(sin[1],src,maxcopy);
}

bool PipeStream::canwrite()
{
    int status = 0;
    return (waitpid(pid, &status, WNOHANG) != pid);
}

#endif

#ifdef XP_WIN
#define IFCloseHandle(a) { if (a) {CloseHandle(a); a=NULL;} }

PipeStream::PipeStream(const char* fn,bool detached) : Stream(Stream::ReadWrite),
FileName(fn)
{
   bool i;

   p1i=p1o=p2i=p2o=p3i=p3o=NULL;
   hwrite=hread=herr=NULL;
   pinfo.hProcess=NULL;
   pinfo.hThread=NULL;

   memset(&sa,0,sizeof(SECURITY_ATTRIBUTES));
   sa.nLength=sizeof(SECURITY_ATTRIBUTES);
   sa.bInheritHandle=TRUE;

    HANDLE prochand;
    prochand=GetCurrentProcess();

   if (!CreatePipe(&p1i,&p1o,&sa,0))
     goto error;

   if (!CreatePipe(&p2i,&p2o,&sa,0))
    goto error;

   if (!CreatePipe(&p3i,&p3o,&sa,0))
    goto error;

    if (DuplicateHandle(prochand,p1o,prochand,&hwrite,
            0,FALSE,DUPLICATE_SAME_ACCESS)==FALSE)
    goto error;

    if (DuplicateHandle(prochand,p2i,prochand,&hread,
            0,FALSE,DUPLICATE_SAME_ACCESS)==FALSE)
    goto error;

    if (DuplicateHandle(prochand,p3i,prochand,&herr,
            0,FALSE,DUPLICATE_SAME_ACCESS)==FALSE)
    goto error;

   memset(&sinfo,0,sizeof(STARTUPINFO));
   memset(&pinfo,0,sizeof(PROCESS_INFORMATION));

   sinfo.cb=sizeof(STARTUPINFO);
   sinfo.dwFlags=STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
   sinfo.wShowWindow=SW_HIDE;
   sinfo.hStdInput=p1i;
   sinfo.hStdOutput=p2o;
   sinfo.hStdError=p3o;

   IFCloseHandle(p1o);
   IFCloseHandle(p2i);
   IFCloseHandle(p3i);
   i = CreateProcess(NULL,FileName,
    NULL, NULL, TRUE,detached?DETACHED_PROCESS:0,
    NULL, NULL,&sinfo,&pinfo);

   if (i)
   {
   StdErr = new AnonymousStream(herr,0);
   IFCloseHandle(p1i);
   IFCloseHandle(p2o);
   IFCloseHandle(p3o);
   }
   else goto error;
   return;

error:
    IFCloseHandle(p1i);
    IFCloseHandle(p1o);
    IFCloseHandle(p2i);
    IFCloseHandle(p2o);
    IFCloseHandle(p3i);
    IFCloseHandle(p3o);
    IFCloseHandle(hwrite);
    IFCloseHandle(hread);
    IFCloseHandle(herr);
   throw xdb("unable to open a pipe",fn);
}

PipeStream::~PipeStream()
{
    IFCloseHandle(pinfo.hProcess);
   IFCloseHandle(pinfo.hThread);

    IFCloseHandle(p1i);
    IFCloseHandle(p1o);
    IFCloseHandle(p2i);
    IFCloseHandle(p2o);
    IFCloseHandle(p3i);
    IFCloseHandle(p3o);
    IFCloseHandle(hwrite);
    IFCloseHandle(hread);
    IFCloseHandle(herr);
}

static int read(HANDLE hread,char * dest,int maxcopy)
{//hread is the child's stdout. Ignore stderr for now.
 DWORD j=0;
 if (hread) ReadFile(hread,dest,maxcopy,&j,NULL);
 return j;
}

int AnonymousStream::read(char * dest,int maxcopy)
{
 return ::read(hread,dest,maxcopy);
}

int PipeStream::read(char * dest,int maxcopy)
{
 return ::read(hread,dest,maxcopy);
}

static bool canread(HANDLE hread)
{
 char c;
 DWORD read=0,avila=0,waiting=0;
 if (hread) PeekNamedPipe(hread,&c,1,&read,&avila,&waiting);
 return read;
}

bool AnonymousStream::canread()
{
 return ::canread(hread);
}

bool PipeStream::canread()
{
 return ::canread(hread);
}

bool AnonymousStream::canwrite()
{
 if (!hwrite) return false;
 DWORD j=0;
 return WriteFile(hwrite,"",0,&j,NULL);
}

bool PipeStream::canwrite()
{
  return WaitForSingleObject(pinfo.hProcess,0) != WAIT_OBJECT_0;
}

static int write(HANDLE hwrite,const char * src,int maxcopy)
{//hwrite is the child's stdin
 DWORD j=0;
 if (hwrite) WriteFile(hwrite,src,maxcopy,&j,NULL);
 return j;
}

int AnonymousStream::write(const char * src,int maxcopy)
{
 return hwrite ? ::write(hwrite,src,maxcopy) : 0;
}

int PipeStream::write(const char * src,int maxcopy)
{
 return ::write(hwrite,src,maxcopy);
}

AnonymousStream::AnonymousStream(HANDLE r, HANDLE w): hread(r), hwrite(w),
   Stream( (TType)((r?ReadOnly:0) | (w?WriteOnly:0)))
{
}

AnonymousStream::~AnonymousStream()
{
}
#endif
