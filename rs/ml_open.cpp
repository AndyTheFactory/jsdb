#include "rslib.h"
#pragma hdrstop

#ifndef NO_POP_MAIL
bool GetPop3Config(void* parent, TParameterList* config);
#endif

MailLibrary* OpenMailSystem(EMailType MailType, bool Login,
                             const char  * name,const char *password,
                             const char * server,const char  * smtp,
                             const char * codepage, const char * address)
{
// if (MailType == email_TryAll) MailType = PMGetMailType();
// if not specified, open exchange

 MailLibrary * o = NULL;
#ifndef NO_POP_MAIL
 if (MailType == email_POP3)
   {
 /*
    TStr P(name);       // user:password@pop-server, smtp-server
    TStr srv(server);
    TStr pwd(password);

    if (!*srv && !*pwd && *P)
    {
     char* s = strchr(P,'@');
     char* p;
     if (s)
      {
       *s++=0;
       p = strchr(P,':');
       if (p)
       {
        *p++ = 0;
        if (!*pwd) pwd = p;

       }

       if (!*srv) srv = s;
      }
    }
*/
    if (!smtp || !*smtp) smtp = server;

     try {
      return new POPMailLibrary(smtp,server,name,password,codepage,address);
         } catch (...) {return 0;}
   }
#endif
  if (MailType == email_Notes)
   {
#ifndef NO_NOTES_MAIL
   try {
    o = new NSFMailLibrary(server,0,name,password);
    if (o->error == NULL) return o;
    else delete o;

    } catch(...) {} // ignore it, try to run as a POP3 mailbox instead
#endif
#if !defined(NO_VIM_MAIL)
   try {
    o = new VIMMailLibrary(Login);
    if (o->error == NULL) return o;
    else delete o;
    } catch (...) {}
#endif
#if !defined(NO_VIM_MAIL) && !defined(NO_POP_MAIL)
    TStr server(MAXPATH);
    TStr sender(MAXPATH);
    GetPrivateProfileString("Notes","MailServer","",server,MAXPATH,"Notes.ini");
    GetPrivateProfileString("Notes","Location","",sender,MAXPATH,"Notes.ini");
    char * c = strrchr(sender,',');
    if (c) sender=c+1;

   try {
    o = new POPMailLibrary(server,server,sender,"",codepage);
    if (o->error == NULL) return o;
    else delete o;
    }
    catch(...) {}
#endif
#if !defined(NO_VIM_MAIL)
    return 0;
#endif
   }
#ifndef NO_CC_MAIL
  if (MailType == email_ccMail5 )
  {
#endif
#if !defined(NO_VIM_MAIL) && !defined(NO_CC_MAIL)
   try {
    o = new VIMMailLibrary(Login,name,password);
    if (o->error == NULL) return o;
    else delete o;
    } catch (...) {}
#endif
#if !defined(NO_CMC_MAIL) && !defined(NO_CC_MAIL)
   try {
   o = new CMCMailLibrary(MailType,Login,name,password,server);
   if (o->error == NULL) return o;
   else delete o;
   } catch(...) {}
#endif
#ifndef NO_CC_MAIL
  }
#endif
#ifndef NO_VIM_MAIL
  if( MailType == email_BeyondMail || MailType == email_VIM )
  {
   try{
    o = new VIMMailLibrary(Login,name,password);
    if (o->error == NULL) return o;
    else delete o;
    } catch(...) {}
  }
#endif
#ifndef NO_CMC_MAIL
if (MailType == email_CMC)
{
   try{
     o = new CMCMailLibrary(MailType,Login,name,password,server);
     if (o->error == NULL) return o;
     else delete o;
    } catch(...) {}
}
#endif
#ifndef NO_MAPI_MAIL
//exchange, msmail, ccmail 8, GroupWise
   try {
   o = new MAPIMailLibrary(Login,name,password,server);
   if (o->error == NULL) return o;
   else delete o;
   } catch(...) {}
#endif
  return 0;
}

MailLibrary::MailLibrary() :error(0)
{
}

MailLibrary::~MailLibrary()
{
}

MailMessage::MailMessage() {Deleted = false;}

MailMessage::~MailMessage()  {}; //deallocates space

bool MailLibrary::SendDocuments(const char * recip, const char * file,
                                         const char * subject, const char * textnote,
                                         bool quiet,
                                         MailCallback CB,void*v)
{
    return false;
}

bool MailLibrary::GetMessages(TList<MailMessage> &msgs,
                                       bool UnreadOnly, TStr &Error,
                             MailCallback CB,void*v)
{
    return false;
}

bool MailLibrary::SendMessage(TNameValueList& message, bool quiet,MailCallback CB,void*v)
{
 const char * html = message("HTML");
 if (*html)
   if ( SendMultipart(message("TO"),message("SUBJECT"),message("ATTACH"),
                      message("TEXT"),message("HTML"),quiet,CB,v)) return true;

 return
   SendDocuments(message("TO"),message("ATTACH"),
               message("SUBJECT"),message("TEXT"),quiet,CB,v);

}

bool MailLibrary::SendMultipart(const char * recip,
                              const char * subject,
                              const char * files,
                              const char * textnote,
                              const char * HTMLnote,
                              bool quiet,
                              MailCallback CB,void*v)
{
#ifdef XP_WIN
 TStr temp(MAXPATH);
 GetTempPath(MAXPATH,temp);
 AddBackslash(temp);
 TStr filename(temp,"message.html");
#else
 TStr filename("/tmp/message.html");
#endif
 TStr attach(filename,",",files);

 try{
  FileStream html(filename,Stream::OMText,Stream::IOWrite);
  html << HTMLnote;
 } catch(...) {return false;}

 return SendDocuments(recip,attach,subject,textnote,quiet,CB,v);
}


