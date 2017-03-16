#include "rslib.h"
#pragma hdrstop
#include "io_fifo.h"

/** file streams throw exceptions, other streams just set their 'error' flag */
Stream* OpenStream(const char* type,xdb* error)
{
 Stream* ret = NULL;
 if (type == NULL) return NULL;

 const char* name = strstr(type,"://");
 if (!name) return NULL;
 name += 3;

 if (type[0] == 0) return NULL;
 try {

#ifndef NO_INET_STREAM
 if (!strncasecmp(type,"http://",7)) ret = new InternetStream(type);

 if (!strncasecmp(type,"inet://",7) || !strncasecmp(type,"net://",6)|| !strncasecmp(type,"tcp://",6))
 {
  TStr host(strstr(type,"://")+3);
  size_t len = strlen(host);
  if (host[len-1] == '/') host[len-1]=0;
  char *port = strchr(host,':');
  if (port)
   *port++=0;
  ret = new InternetStream(host,port ? atoi(port) : 80);
 }
#endif

#ifndef NO_COMM_STREAM
// else if (!strncasecmp(type,"com",3)) ret = new CommStream(TStr(type,4),name);
else if (!strncasecmp(type,"com",3))
{
 TStr t(type);
 char* c = strchr(t,':');
 if (c) *c = 0;
  ret = new CommStream(t,name);
}
#endif

#ifndef NO_MEMORY_STREAM
 else if (!strncasecmp(type,"text",4))
   {
    ret = new MemoryStream(atoi(name));
// use the ByteStream constructor only if you know that the pointer won't disappear
//    Stream* s = new ByteStream((char*)name);
//    s->Type = Stream::ReadOnly;
//    return s;
  }
 else if (!strncasecmp(type,"temp",4)) ret = new MemoryStream(atoi(name));
 else if (!strncasecmp(type,"fifo",4)) ret = new FIFOStream(atoi(name));
#endif

#ifndef NO_FILE_STREAM
 else if (!strncasecmp(type,"file",4)) ret = new FileStream(name);
#endif
 }
 catch (xdb& x)
  {
   if (error) *error = x;
  }
 catch(...)
  {
   return NULL;
  }

 if (!ret) return NULL;
 if (ret->error) {if (error) *error = *ret->error; delete ret; return NULL;}
 if (ret->Type == Stream::NotOpen) {delete ret; return NULL;}

 return ret;
}


