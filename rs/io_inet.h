#ifdef XP_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

class  InternetStream : public Stream   /* simple socket */
{public:
 unsigned int s; ///SOCKET s
 TStr hostinfo;
 TStr hostname;
 int32 hostaddr;
 int port;
 char putback;
 int buffer;
// TPointer<Stream> start;

 InternetStream();
 InternetStream(unsigned int socket, int host);
 InternetStream(const char* url,TNameValueList* headers = 0);
 InternetStream(const char host[],int port);
 ~InternetStream();

 void NoDelay();
 int GetLastError();
 int SkipHeaders(); ///returns the status
 int GetHeaders(TNameValueList &n); ///returns the status

 virtual bool eof();
 virtual bool canwrite();
 virtual bool canread();
 bool wait(int ms);

 bool init(const char *host, int port,TStr* error=NULL);
 virtual int write(const char *b, int n);
 virtual int read(char *b, int n);
 const char* gethostinfo();
 virtual const char* filename() {return gethostinfo();}
 bool sendln(const char s[]);
 bool recvln(char s[], int maxlen);
};

class InternetServer
{public:
 TPointer<xdb> error;
 unsigned int s;
 int port;
 sockaddr_in myaddr;
 TStr hostinfo;
 TStr hostname;
 TStr address;

 InternetServer(int port);
 ~InternetServer();
 InternetStream* Accept();

 bool AnyoneWaiting(int ms =-1);

 protected:
 bool Startup();

};
