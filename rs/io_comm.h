class CommStream : public Stream
{
 HANDLE File; // let the OS do any buffering
 size_t timeout;

 TStr Port;

 public: //open in read-write mode
 const char * LastError;
 CommStream ();
 CommStream (const char * _port,const char * speed = 0, TType Type=ReadWrite);
  //opens at 9600 by default

 void Init(const char * _port,const char * speed = 0);
  // Init() can throw an exception with a useful error message

 ~CommStream();
 const char* filename() {return Port;}
 int read(char * dest,int maxcopy);
 int write(const char * src,int maxcopy);
 int WriteSlowModemCommand(const char * src,int wait = 20);
 bool canwrite() {return true;}
 bool canread();

 //      uint32 dwStart = GetTickCount();
 //      while( (dwStart + MilSec) >GetTickCount() );
};
