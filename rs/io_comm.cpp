#include "rslib.h"
#pragma hdrstop

#ifndef NO_COMM_STREAM

CommStream::CommStream ()
 : Stream(ReadWrite)
{
LastError = 0;
File = 0;
timeout = 128;
};

CommStream::CommStream (const char * _port, const char * _speed, TType Type)
 : Stream(Type)
{
LastError = 0;
File = 0;
timeout = 128;
Init(_port,_speed);
if (error) Type=NotOpen;
};


void CommStream::Init(const char * _port,const char * speed)
{
 Port = _port;
 if (speed == 0) speed = "9600";
if (File) CloseHandle(File);

char ofn[128];
strcpy(ofn,"\\\\.\\");
strncpy(ofn+4,Port,120);
File = CreateFile(
    ofn,
    GENERIC_READ    | GENERIC_WRITE,
    FILE_SHARE_WRITE,
    0,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    0);

if (!File)
 {
  error = new xdb("Port open failed","Port",Port,"speed",speed);
  return;
 }

 DCB dcb;
 //TStr Temp(Port,": baud=",speed," parity=N data=8 stop=1");

 memset(&dcb,0, sizeof(DCB));
 dcb.DCBlength = sizeof(DCB);

 GetCommState(File, &dcb);
 dcb.BaudRate = atoi(speed);     //  baud rate
 dcb.ByteSize = 8;             //  data size, xmit and rcv
 dcb.Parity   = NOPARITY;      //  parity bit
 dcb.StopBits = ONESTOPBIT;
 //BuildCommDCB(Temp, &dcb);
 SetCommState(File, &dcb);

}

CommStream::~CommStream()
{
if (File) CloseHandle(File);
}

bool CommStream::canread()
{
 DWORD mask = 0;
 GetCommMask(File,&mask);
 return (mask & EV_RXCHAR);
// return WaitForSingleObject(File,0) == WAIT_OBJECT_0;
}

int CommStream::read(char * dest,int maxcopy)
{
DWORD count=0;
size_t t = 0;
while (t < timeout && count == 0)
{
 if (ReadFile(File,dest,maxcopy,&count,0) && count)
  return count;
 t ++;
 Sleep(1);
}
return 0;
};

int CommStream::write(const char * src,int maxcopy)
{
DWORD count=0;
if (WriteFile(File,src,maxcopy,&count,0)) return count;
return 0;
};

int CommStream::WriteSlowModemCommand(const char * src,int wait)
{
while (*src)
 {
 write(src,1);
 uint32 dwStart = GetTickCount();   // CPU time from start
 while( (dwStart + wait) > GetTickCount() ); // wait for 100 millisecs
  src++;
 }
return 1;
}

#endif
