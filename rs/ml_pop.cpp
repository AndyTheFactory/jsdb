#include "rslib.h"
#pragma hdrstop

#ifndef NO_POP_MAIL
#define POPCHANNEL 110

POPMailMessage::POPMailMessage()
  :MailMessage(),
   Attachments(false)
  {
    msgtree = 0;
    textmessage = 0;
    htmlmessage = 0;
    server=0;
    isread=false;
 }

POPMailMessage::~POPMailMessage()
 {
  if (Deleted) server->DelMessage(msgid);
  if (msgtree) delete msgtree;
   //allows undeleting before committing.
 }

const char * POPMailMessage::Header(const char*c)
{
 if (msgtree) return msgtree->header(c);
 if (!header2.Count()) LoadHeader();
 return header2(c);
}

bool POPMailMessage::IsRead()
{
 if (isread) return true;
 return strchr(Header("Status"),'R') != NULL;
}

int POPMailMessage::AttachCount()
{
 if (!stristr(Header("Content-Type"),"multipart"))
   return 0;

 if (!msgtree) LoadMessage();
 return Attachments.Count();
}

char * StripCharsFB(char * c, const char * rem);

bool POPMailMessage::GetAttachment(size_t i, TStr& title,
                                              TStr& file, bool getfile)
{
  if ( i == NOT_FOUND ) return false;
  if (!msgtree) LoadMessage();
  mmatom * ma = Attachments[i];
  if (!ma) return false;

  TStr s = ma->header["Content-Type"];
  char * c = strstr(s,"name=");
  if (c)
    {
     c += strlen("name=");
     goto AtGetTitle;
    }
  else
    {
     s = ma->header["Content-Disposition"];
     c = strstr(s,"filename=");
     if (c)
      {
       c += strlen("filename=");
AtGetTitle:
       title = StripCharsFB(c,"\"");
      }
    }

  if (!*title) title = "untitled.txt";

  bool ret = getfile ? ma->GetFile(file,GetExtension(title)) : true;
   //saves the stream to a file, after decoding it

  return ret;
}

void POPMailMessage::IterateAtoms(mmcomp* c)
  {
   mmatom*ma;
   mmcomp*mc;

   FOREACH(mimemail*node,c->parts)
    ma = TYPESAFE_DOWNCAST(node,mmatom);
    if (ma)
     {
       if (stristr(ma->header["Content-Disposition"],"attachment")
           || ma->header.Has("Content-ID"))
          Attachments.Add(ma);
       else if (stristr(ma->header["Content-Type"],"text/html"))
       {  if (!htmlmessage) htmlmessage = ma; }
       else if (!textmessage) textmessage = ma;
       else
          Attachments.Add(ma);
     }
    else
     {
      mc = TYPESAFE_DOWNCAST(node,mmcomp);
      if (mc)
       {
        IterateAtoms(mc);
       }
     }
   DONEFOREACH
}

void POPMailLibrary::Load(const char* cmd, Stream& out)
{
 if (!socket) return;

 char reply[256];
 if (!socket->sendln(cmd)) return;
 if (!socket->recvln(reply, sizeof(reply))) return;      // skip +OK reply
 while(1)
 {
  reply[0] = 0;
  bool eol = socket->recvln(reply, sizeof(reply));

  if (!eol && !reply[0]) //communication error
    break;

  if (reply[0] == '.' && reply[1] == '\0')  // a single dot ends the mail
    break;

  if (reply[0] == '.' && reply[1] == '.')  // two dots => one dot
    out.writestr(reply+1);
  else
    out.writestr(reply);

  if (eol)
    out.write("\r\n",2);
 }
}

void POPMailMessage::LoadHeader()
{
 if (!server) return;
 MemoryStream hdr;
 TStr commline("TOP ",msgid," 0");
 server->Load(commline,hdr);
 hdr.rewind();
 hdr.ReadMIME(header2);
}

void POPMailMessage::LoadMessage()
{
 if (!server) return;
 TStr cmd ("RETR ",msgid);
 server->Load(cmd,contents);
 if (!msgtree) parse();
 if (!msgtree) return;

 mmatom*ma;
 mmcomp*mc;

 ma = TYPESAFE_DOWNCAST(msgtree,mmatom);
 if (ma)
 {
  textmessage = ma;  /* one-part message */
 }
 else
 {
  mc = TYPESAFE_DOWNCAST(msgtree,mmcomp);
  IterateAtoms(mc);
 }
}

void POPMailMessage::parse()
{
   strumail sm;
   contents.rewind();
   sm.parse(contents);
   contents.rewind();
   msgtree = ParseMIMEMessage(sm);
}

/*
void POPMailMessage::savedetails(const char p[])
{
try {
 FileStream Out(p,TRSStream::OMText,TRSStream::WriteOnly);
  contents.rewind();
 Out.Append(contents);
 contents.rewind();
 }
 catch(...){}
}

void POPMailMessage::loaddetails(const char p[])
{
  try {
 FileStream In(p,Stream::OMText,Stream::ReadOnly);
 contents.Clear();
 contents.Append(In);
 contents.rewind();
 }
 catch(...){}
}
*/

char * POPMailMessage::Sender()
{
return (char*)Header("From");
}

char * POPMailMessage::Subject()
{
return (char*)Header("Subject");
}

char * POPMailMessage::MsgText()
{
 if (!msgtree) LoadMessage();
 isread = true;
 if (decodedText.size()) return decodedText;
 if (textmessage)
 {
  if (textmessage->Decode(decodedText))
    return decodedText;
  return textmessage->body;
 }
 return contents;
}

char * POPMailMessage::HTMLText()
{
 if (!msgtree) LoadMessage();
 isread = true;
 if (decodedHTML.size()) return decodedHTML;
 if (htmlmessage)
 {
  if (htmlmessage->Decode(decodedHTML))
    return decodedText;
  return htmlmessage->body;
 }
 return (char*)"";
}

int32 POPMailMessage::Size()
{
 if (contents.size())
 return contents.size();
 return atoi(Header("Content-length"));
}

extern const char *months[]; // in sy_time.cpp

void POPMailMessage::Time(SYSTEMTIME& tm)
{
/* =  // size is 4, not 3 because of the NULL
   {
      "Jan",
      "Feb",
      "Mar",
      "Apr",
      "May",
      "Jun",
      "Jul",
      "Aug",
      "Sep",
      "Oct",
      "Nov",
      "Dec"
   };*/

   char a[180];
   memset(a,0,sizeof(a));
   strncpy(a, Header("Date"),170);

   ParseDateTime(a,tm);
//   TTime tt(TDate(dd, mm, yy), hh, nn, ss);
//   return tt+dhh*3600L+dnn*60L;  // one hour has 3600 seconds
}

/* Server */
// send QUIT when it's destructed
void doSendQuit(InternetStream*ss)
{
 //  char reply[256];
   ss->sendln("QUIT");
//   ss->recvln(reply, sizeof(reply));
}
class sendquit
{
   InternetStream *ss;
   public:
   sendquit(InternetStream *x) : ss(x) { }
  ~sendquit()  {  doSendQuit(ss);  }
};

// send a SMTP command and expect a specific reply. the command is specified
// by a format string with a single string argument. this seems to be
// sufficient for the SMTP commands

const char* POPMailLibrary::GetPassword()
{
// if (!*password)
//  if (!promptPassword(password)) throw xdb("POP3 login failed");
 return password;
};

POPMailLibrary::~POPMailLibrary()
{
 POPLogOut();
}

void POPMailLibrary::POPLogOut()
{
  if (socket) doSendQuit(socket);
}

const char* POPMailLibrary::GetSMTPServer()
{
#if 0
//def XP_WIN
//search in historical order
  smtpserver.Resize(MAXPATH);
  GetProfileString("Mail","SMTP_Server","",smtpserver,MAXPATH);
  if (!*smtpserver)   //IE 4
     RegGetKey("Software\\Microsoft\\Internet Account Manager\\Accounts\\00000001",
               "SMTP_Server",smtpserver,HKEY_CURRENT_USER);
  if (!*smtpserver)  //NC 3
     RegGetKey("Software\\Netscape\\Communicator\\Services",
               "SMTP_Server",smtpserver,HKEY_CURRENT_USER);
  if (!*smtpserver)  //NN 4
     RegGetKey("Software\\Netscape\\Netscape Navigator\\biff\\users\\default",
               "defaultServer",smtpserver,HKEY_CURRENT_USER);

  if (!*smtpserver)  //NN 3
     RegGetKey("Software\\Netscape\\Netscape Navigator\\Services",
               "SMTP_Server",smtpserver,HKEY_CURRENT_USER);
  if (!*smtpserver)  //IE 3
     RegGetKey("Software\\Microsoft\\Internet Mail and News\\Mail",
               "Default SMTP Server",smtpserver,HKEY_CURRENT_USER);
#endif

return smtpserver;
}

POPMailLibrary::POPMailLibrary(const char*Smtpserver,
                 const char*Popserver,
                 const char*Username,
                 const char*Password,
                 const char*cp,
                 const char*addr)
                 : MailLibrary(),
                   smtpserver(Smtpserver),
                   popserver(Popserver),
                   username(Username),
                   password(Password),
                   codepage(cp),
                   address(addr),
                   host(1024),
                   socket(0)
{
  gethostname(host,1024);
  if (!*host) host = smtpserver;
  if (!*address)
  {
   if (strchr(username,'@'))
    address = username;
   else
    address = TStr(username,"@",popserver);
  }
 /* char * c = strchr(host,':');
  if (c) *c=0;

  c = strchr(username,'@');
  if (c && smtpserver == popserver)
   {
    *c = 0;
    c++;
    popserver = c;
   }
  */
}

int POPMailLibrary::sendrecv(InternetStream &ss, int code, const char fmt[], const char arg[])
{
  char commline[256];

  sprintf(commline, fmt, arg);  // compose the command

  ss.sendln(commline);          // send the command

  return GetSMTPReply(ss, code);    // expect that reply
}

// send a POP command and expect a +OK reply. the command is specified
// by a format string with a single string argument. this seems to be
// sufficient for the SMTP commands
int POPMailLibrary::sendrecv(InternetStream &ss, TStr &Error, const char fmt[], const char arg[])
{
  char commline[256];

  sprintf(commline, fmt, arg);  // compose the command

  ss.sendln(commline);          // send the command

  return GetPOPReply(ss, Error);// expect a +OK reply
}

/*
void ThrowWinsockError(int i,const char * msg)
{
const char * c;
switch (i)
{
case 10013: c="Permission denied."; break;
case 10048: c="Address already in use."; break;
case 10049: c="Cannot assign requested address."; break;
case 10047: c="Address family not supported by protocol family."; break;
case 10037: c="Operation already in progress."; break;
case 10053: c="Software caused connection abort."; break;
case 10061: c="Connection refused."; break;
case 10054: c="Connection reset by peer."; break;
case 10039: c="Destination address required."; break;
case 10014: c="Bad address."; break;
case 10064: c="Host is down."; break;
case 10065: c="No route to host."; break;
case 10036: c="Operation now in progress."; break;
case 10004: c="Interrupted function call."; break;
case 10022: c="Invalid argument."; break;
case 10056: c="Socket is already connected."; break;
case 10024: c="Too many open files."; break;
case 10040: c="Message too long."; break;
case 10050: c="Network is down."; break;
case 10052: c="Network dropped connection on reset."; break;
case 10051: c="Network is unreachable."; break;
case 10055: c="No buffer space available."; break;
case 10042: c="Bad protocol option."; break;
case 10057: c="Socket is not connected."; break;
case 10038: c="Socket operation on non-socket."; break;
case 10045: c="Operation not supported."; break;
case 10046: c="Protocol family not supported."; break;
case 10067: c="Too many processes."; break;
case 10043: c="Protocol not supported."; break;
case 10041: c="Protocol wrong type for socket."; break;
case 10058: c="Cannot send after socket shutdown."; break;
case 10044: c="Socket type not supported."; break;
case 10060: c="Connection timed out."; break;
case 10035: c="Resource temporarily unavailable."; break;
case 11001: c="Host not found."; break;
case 10093: c="Successful WSAStartup not yet performed."; break;
case 11004: c="Valid name, no data record of requested type."; break;
case 11003: c="This is a non-recoverable error."; break;
case 10091: c="Network subsystem is unavailable."; break;
case 11002: c="Non-authoritative host not found."; break;
case 10092: c="WINSOCK.DLL version out of range."; break;
case 10094: c="Graceful shutdown in progress."; break;
default: c = "Unknown error";
}
xdb(msg,c,i).raise();
}
*/

/*
void GetMachineName(TStr& s)
{
 TStr host,domain;
 RegGetKey("System\CurrentControlSet\Services\VXD\MSTCP",
           "HostName",host,HKEY_LOCAL_MACHINE);
 RegGetKey("System\CurrentControlSet\Services\VXD\MSTCP",
           "Domain",domain,HKEY_LOCAL_MACHINE);
 s = TStr(host,*domain?".":0,domain);
}
*/


bool POPMailLibrary::SendMultipart(const char * precip,
                              const char * subject,
                              const char * files,
                              const char * textnote,
                              const char * HTMLnote,
                              bool quiet,MailCallback CB,void*v)
{
  error = 0;

  TStr recip(precip);
  if (!*recip) return false;

  const char * cp = codepage;
  if (!*cp) cp = "US-ASCII";

  if (!files) files = "";

   TStr Errors;
  try
  {
   if (CB)
    if (!CB(v,0,0,"Connecting...")) return false;

   InternetStream ss;
   SMTPLogin(ss);
   if (error) return false;

   {
   sendquit sq(&ss);   // send QUIT when it's destructed

   //sends the RCPT, DATA commands and DATE TO FROM headers
   if (SendRecip(ss,recip,Errors))
   {

   TStr Textboundary(256); // 72616F = RAO
   TStr Fileboundary(256);
   long seed = GetTickCount();
   sprintf(Textboundary,"----=_Mime-Boundary_text-72616F.%lX",seed);
   sprintf(Fileboundary,"----=_Mime-Boundary_file-72616F.%lX",seed);
//   TStr start1"--",boundary);
//   TStr end("--",boundary,"--");

   ss.sendln(TStr("Subject: ",subject));
   ss.sendln("X-Mailer: Raosoft SMTPMTA 1.1");
   ss.sendln("MIME-Version: 1.0");
   if (*files)
     ss.sendln(TStr("Content-Type: multipart/mixed; boundary=\"",Fileboundary,"\""));
   else
     ss.sendln(TStr("Content-Type: multipart/alternative; boundary=\"",Textboundary,"\""));

   ss.sendln("");
   ss.sendln("  This is a multi-part message in MIME format.");
   ss.sendln("");

   if (*files)
   {
      ss.sendln(TStr("--",Fileboundary));
      ss.sendln(TStr("Content-Type: multipart/alternative; boundary=\"",Textboundary,"\""));
      ss.sendln("");
      ss.sendln("");
   }

//   ss.sendln(start);
//   ss.sendln(TStr("Content-Type: multipart/alternative; boundary=\"",boundary,"\""));
//   ss.sendln("");
//   ss.sendln("");


   if (textnote)
   {
   ss.sendln(TStr("--",Textboundary));
   ss.sendln(TStr("Content-Type: text/plain; charset=\"",cp,"\""));
   ss.sendln("Content-Transfer-Encoding: 8bit");
   ss.sendln("");
   SendText(ss,textnote,CB,v);
   }

   if (HTMLnote)
   {
   ss.sendln(TStr("--",Textboundary));
   ss.sendln(TStr("Content-Type: text/html; charset=\"",cp,"\""));
   ss.sendln("Content-Transfer-Encoding: 8bit");
   ss.sendln("");
   SendText(ss,HTMLnote,CB,v);
   }

   ss.sendln(TStr("--",Textboundary,"--"));

   if (*files)
   {
    ss.sendln("");
    SendAttachments(ss,files,TStr("--",Fileboundary),CB,v);
    ss.sendln(TStr("--",Fileboundary,"--"));
   }

   ss.sendln(".");
   }
   }
   //sendquit
  }
  catch (xdb& x)
   {
    if (CB)
     CB(v,-1,0,TStr(x.why(),"\n",x.info()));
    return false;
   }
  catch (...)
   {
    return false;
   }

 if (!quiet) if (*Errors)
     {error = new xdb("Unable to deliver mail","Recipient",recip,"Error",Errors,"Response",reply);
      return false;
      }
 return true;
}

bool POPMailLibrary::SendDocuments(const char * precip, const char *attach,
const char *subject, const char* note,bool quiet,MailCallback CB,void*v)
{
  error = 0;

  TStr recip(precip);
  if (!*recip) return false;

  const char * cp = codepage;
  if (!*cp) cp = "utf-8";

  TStr Errors;

  try {
  if (CB)
   if (!CB(v,0,0,"Connecting...")) return false;

  InternetStream ss;
  SMTPLogin(ss);
  if (error) return false;

  {
  sendquit sq(&ss);   // send QUIT when it's destructed

  if (SendRecip(ss,recip,Errors))
  {

  ss.sendln(TStr("Subject: ",subject));
  ss.sendln("X-Mailer: Raosoft SMTPMTA 1.1");
  ss.sendln("MIME-Version: 1.0");

  TStr boundary(256); // 72616F = RAO
  sprintf(boundary,"_Mime-Boundary_72616F.%lX",GetTickCount());
  TStr start("--",boundary);
  TStr end("--",boundary,"--");

  if (attach && *attach)
   {
    ss.sendln(TStr("Content-Type: multipart/mixed; boundary=\"",boundary,"\""));
    ss.sendln("Content-Transfer-Encoding: 7bit");
    ss.sendln("");
    ss.sendln("  This is a multi-part message in MIME format.");
    ss.sendln("");
    ss.sendln(start);
    ss.sendln(TStr("Content-Type: text/plain; charset=",cp));
    ss.sendln("Content-Transfer-Encoding: 8bit");
    ss.sendln("");

    SendText(ss,note,CB,v);

    SendAttachments(ss,attach,start,CB,v);

    ss.sendln(end);
  }
  else
   {
    ss.sendln(TStr("Content-Type: text/plain; charset=",cp));
    ss.sendln("Content-Transfer-Encoding: 8bit");
    ss.sendln("");

    SendText(ss,note,CB,v);
   }

  ss.sendln("");

  if (CB) CB(v,1,1,"Done");

  if (sendrecv(ss, 0, ".", NULL) >= 500) // send a dot to terminate the lines
  {
    return 0;
  }
  }}

  } catch (...) {return false;}

  if (!quiet) if (*Errors)
     {error = new xdb("Unable to deliver mail","Recipient",recip,"Error",Errors,"Response",reply);
     return false;
     }
  return true;
}

bool POPMailLibrary::POPLogIn()
{
  error = 0;

  TStr Error,user;
  if (socket)
   {
    char reply[256];
    if (socket->sendln("STAT")) // will return 6 if the server hasn't disconnected
    if (socket->recvln(reply, sizeof(reply)))   // reply must be +OK according to rfc
    if (strncmp(reply,"+OK",3)==0) return true;
    socket=0; //reconnect, must have timed out
   }

  socket = new InternetStream;

  bool connected = socket->init(popserver,POPCHANNEL);
  if (!connected) connected = socket->init("pop3-server",POPCHANNEL);
  if (!connected) connected = socket->init("pop3",POPCHANNEL);
  if (!connected) connected = socket->init("pop-server",POPCHANNEL);
  if (!connected) connected = socket->init("pop",POPCHANNEL);
  if (!connected) connected = socket->init("mail-server",POPCHANNEL);
  if (!connected) connected = socket->init("mail",POPCHANNEL);
  if (!connected)
  {
    socket=0;
     error = new  xdb("Couldn't connect to the mail server. Please check your email settings.",
               "Server",(char*)socket->hostinfo,"Response",reply);
     return false;
  }

  if (!GetPOPReply(*socket, Error))
  {
     error = new xdb("The mail server isn't responding. Please check your email settings.",
               "Server",(char*)socket->hostinfo,"Response",reply);
     return false;
  }

  user = username;
//  if (strchr(user,'@')) *strchr(user,'@')=0;

  if (!sendrecv(*socket, Error, "USER %s", user))  // USER freemant
  {
    doSendQuit(socket);
    socket=0;
     return 0;
  }
  if (!sendrecv(*socket, Error, "PASS %s", GetPassword()))  // PASS opensesame
  {
    password = 0;
    doSendQuit(socket);
    socket=0;
     return 0;
  }
  return true;
}

bool POPMailLibrary::GetMessages(TList<MailMessage> &mails, bool UnreadOnly,
         TStr &Error,MailCallback CB,void*v)
{
  error = 0;

  //char commline[256];
  char reply[256];
  int i;
  TStr user;
  int msgs, lread;
  TIntList idlist;

  try {

  if (CB) if (!CB(v,0,0,"Opening inbox...")) return false;

//  POPLogOut(); //force resetting the server, allowing check for new mail
  //make sure we're still connected, reconnect if necessary
  if (!POPLogIn())
   {
    Error = "Incorrect Username/Password";
    return false;
   }

  socket->sendln("STAT");
  socket->recvln(reply, sizeof(reply));    // reply must be +OK according to rfc
  sscanf(reply, "+OK %d", &msgs);  // see how many emails

  if (msgs == 0)
  {
     return true;
  }
  idlist.Init(msgs);
  idlist.Reset(0);
  socket->sendln("LAST");
  socket->recvln(reply, sizeof(reply));
  sscanf(reply, "+OK %d", &lread);  // see which email was last read

  socket->sendln("LIST");
  socket->recvln(reply, sizeof(reply));  // skip the +OK reply

  for (i=0;;i++)
  {
     socket->recvln(reply, sizeof(reply));

     if (reply[0] == '.')     // a dot end the whole thing
     {
        break;
     }
    sscanf(reply, "%d", &idlist[i]);  // get the i'th msg id
  }
//  k = 0;         // retrieve the emails. k is the # of emails retrieved so far

  for (i=0; i<msgs && idlist[i] ;i++)
  {
   if (CB) if (!CB(v,i,msgs,"Reading messages...")) break;

    if ((!UnreadOnly) || (idlist[i] > lread))  // should we retrieve the i'th mail?
     {
      POPMailMessage * msg = new POPMailMessage();
        mails.Add(msg);
        msg->server = this;                  // setup server and msg id
        sprintf(msg->msgid, "%d", idlist[i]);
     }
  }

  if (CB) CB(v,msgs,msgs,"Done");

  } catch (...) {return 0;}
  return 1;
}


void POPMailLibrary::SMTPLogin(InternetStream& ss)
{
  error = 0;

  bool connected = ss.init(smtpserver,25);

  /*  if (!connected) connected = ss.init("smtp-server",25);
  if (!connected) connected = ss.init("smtp",25);
  if (!connected) connected = ss.init("mail-server",25);
  if (!connected) connected = ss.init("mail",25);
*/
  if (!connected)  // get connected
  {
    error = new xdb("Couldn't find a mail server. Please check your email settings.",
             "Server",(char*)ss.hostinfo,"Response",reply);
    return ;
  }
  if (!GetSMTPReply(ss, 220))       // expect a return of 220
  {
   error = new xdb("The mail server isn't responding. Please check your email settings.",
             "Server",(char*)ss.hostinfo,"Response",reply);
   return ;
  }

  if (!*host)
   {
    const char* c = strchr(username,'@');
    if (c) host = c + 1;
    else host = smtpserver;
   }

  if (*host)
   {
    char * c = strchr(host,':'); if (c) *c=0;
   }

  //first try EHLO. If that doesn't work, break the connection and try again.

  MemoryStream reply;

  connected = false;

  if (*username)
  {
   TStr greeting("EHLO ",host,"\r\n");
   ss.writestr(greeting);
   int code = GetSMTPReply(ss, &reply);

   if (code < 400)  // HELO macau.ctm.net
   {
    MemoryStream send;
    MemoryStream b64;
    send.write("",1);
    send.writestr(username);
    send.write("",1);
    send.writestr(password);
    send.rewind();
    b64encode(send,b64);

    if (b64.size() <= 240)
     if (strstr(reply,"AUTH PLAIN"))
     {
      if (sendrecv(ss,0,"AUTH PLAIN %s",b64) == 235)
       connected = true;
      else
       error = new xdb("Incorrect login","Name",address,"Server",(char*)ss.hostinfo,"Response",reply);
     }
   }
   else if (code < 500)
   {
        error = new xdb("The mail server returned an error code. Please check your email settings.",
               "Server",(char*)ss.hostinfo,"Response",reply);
        return;
   }

   if (!connected)
   {
    doSendQuit(&ss);
    ss.init(ss.hostinfo,25);
    if (!GetSMTPReply(ss, 220))       // expect a return of 220
    {
     error = new xdb("The mail server isn't responding. Please check your email settings.",
               "Server",(char*)ss.hostinfo,"Response",reply);
     return;
    }
   }
  }

  if (!connected)
  {
     if (sendrecv(ss, 0, "HELO %s", host) >= 500)  // HELO macau.ctm.net
    {
     doSendQuit(&ss);
     error = new xdb("Unable to register with the mail server","Server",(char*)ss.hostinfo,"Response",reply);
     return;
    }
    connected = true;
   }

  const char* fmtstr = "MAIL FROM: <%s>";
  if (strchr(address,'<')) fmtstr = "MAIL FROM: %s";
  if (sendrecv(ss,0 , fmtstr, address) >= 500) // MAIL FROM:<freemant@macau.ctm.net>
  {
    doSendQuit(&ss);
    error = new xdb("The mail server did not accept our MAIL command.","server",(char*)ss.hostinfo,"Reason",reply);
    return;
  }
  else
  {
   if (!connected)
     error = new xdb("The mail server did not accept our HELO command. Perhaps the user name or password are incorrect?","server",(char*)ss.hostinfo,"Reason",reply);
  }
}

//date from to & BCC sender reply-to

void POPMailLibrary::SendList(InternetStream& ss, TStringList& names,
     const char fmt[], TStringList&errors)
{
 MemoryStream to;
  FOREACH(char*c,names)
    const char* n = strchr(c,'<');
    if (n)
     {
      n++;
      to.write(n,strcspn(n,">"));
     }
    else
     to << c;

    if (to.size() > 200)
     errors.Add(to);
    else if (sendrecv(ss,0, fmt,to) >= 500)    //RCPT TO:<billg@microsoft.com>
      errors.Add(to);     //ret code 250 = success

    to.Clear();
   DONEFOREACH
}

bool POPMailLibrary::SendRecip(InternetStream& ss, const char * recip,TStr &Errors)
{
  TStringList BCCnames;
  TStringList CCnames;
  TStringList TOnames;

  TStringList ERRnames;

  TStringList Recipients(recip,",;");
  TStringList * list = &TOnames;
  FOREACH(char*c,Recipients)
    if (!strcasecmp(c,"BCC")) { list = &BCCnames; continue;}
    if (!strcasecmp(c,"CC")) { list = &CCnames; continue;}
    list->Add(c);
  DONEFOREACH

  if (!TOnames.Count())
   TOnames.Add(address);

  SendList(ss, TOnames,"RCPT TO: <%s>", ERRnames);
  SendList(ss, CCnames,"RCPT CC: <%s>", ERRnames);
  SendList(ss, CCnames,"RCPT BCC: <%s>", ERRnames);

  if (ERRnames.Count())
  {
   MemoryStream x;
   x.Write(ERRnames,",");
   Errors = x;
   return false;
  }

  if (!sendrecv(ss, 354, "DATA", NULL)) // DATA
  {
    Errors += reply;
    return false;
  }

/*
  CC: name1,
      name2,
      name3
*/

  ss.sendln(TStr("From: ",address));

  MemoryStream to;
  to << "TO: ";
  if (TOnames.Count())
  {
   FOREACH(char*c,TOnames)
    if (i) to << "    ";
    to << c;
    if (i != imax - 1) to << ",";
    ss.sendln(to);
    to.Clear();
   DONEFOREACH
  }

  if (CCnames.Count())
  {
   to.Clear();
   to << "CC: ";
   FOREACH(char*c,CCnames)
    if (i) to << "    ";
    to << c;
    if (i != imax - 1) to << ",";
    ss.sendln(to);
    to.Clear();
   DONEFOREACH
  }
/*
  if (BCCnames.Count())
  {
   to.Clear();
   to << "BCC: ";
   FOREACH(char*c,BCCnames)
    if (i) to << "    ";
    to << c;
    if (i != imax - 1) to << ",";
    ss.sendln(to);
    to.Clear();
   DONEFOREACH
  }
*/
   //Date: Thu, 21 May 1998 05:33:29 -0700

  TStr date(32);
  GetDateTime(date,0,3,true);
  ss.sendln(TStr("Date: ",date," GMT"));
  return true;
}


void POPMailLibrary::SendAttachments(InternetStream& ss, const char * files,
const char * start,MailCallback CB,void*v)
{
   TStringList Attachments(files,",;");
   FOREACH(char * fn, Attachments)
   if (CB) if (!CB(v,i,imax,fn)) break;

   try
   {
   if (!FileExists(fn)) continue;
   FileStream in(fn,Stream::OMBinary,Stream::ReadOnly);

   TStr title(GetFilename(fn));
   ss.sendln(start);
   ss.sendln(TStr("Content-Type: application/octet-stream; name=\"",title,"\""));
   ss.sendln(TStr("Content-Disposition: attachment; filename=\"",title,"\""));
   ss.sendln("Content-Transfer-Encoding: base64");
   ss.sendln("");

   //mime encode
   b64encode(in,ss);

   } catch(...) {}

   DONEFOREACH
}

void POPMailLibrary::SendText(InternetStream& ss, const char * c,
MailCallback CB,void*v)
{
  int update = 0;

 while (*c)
  {
   if (CB) if (!CB(v,++update,0,"Sending...")) break;

   int len = strcspn(c,"\r\n");

   TStr line(c,len);

   if (*line == '.') memmove(line+1,line,len+1);
   ss.sendln(line);

   c += len;
   while (*c == '\r') c++;
   if (*c == '\n') c++;
  }

}

int POPMailLibrary::DelMessage(const char msgid[])
{
 // int pos;
  TStr Error;

  if (!socket)
    if (!POPLogIn()) return false;

  if (!sendrecv(*socket, Error, "DELE %s", msgid))  // DELE 10
  {
    doSendQuit(socket);
    socket = 0;
    return 0;
  }
  return 1;
}

// expecting a particular reply code. return true or false
int POPMailLibrary::GetSMTPReply(InternetStream &ss, int code)
{
 if (code)
  return GetSMTPReply(ss) == code;

 return GetSMTPReply(ss);
}

// return the reply code
int POPMailLibrary::GetSMTPReply(InternetStream &ss,Stream* result)
{
  while (ss.recvln(reply, sizeof(reply)))
  {
    if (result) (*result) << reply << "\r\n";
    if (reply[3] != '-') return atoi(reply);
  }
  return 0;
}

// expecting a +OK reply. return true or false
int POPMailLibrary::GetPOPReply(InternetStream &ss, TStr &error)
{
  char reply[256];

  ss.recvln(reply, sizeof(reply));

  if (strncasecmp(reply, "+OK", 3) != 0)      // not +OK, has to be -ERR
  {
     error = reply;  // copy the whole error msg back

     return 0;
  }
  else
  {
     return 1;
  }
}

#endif
