/*******************************************************************************
*                      Raosoft Email Classes                                   *
*******************************************************************************/

#ifndef _RS_MAIL_H
#define _RS_MAIL_H

#ifndef _RS_LIST_H
#include "rs/list.h"
#endif
#ifndef _RS_POINTER_H
#include "rs/pointer.h"
#endif
#ifndef _RS_STREAM_H
#include "rs/stream.h"
#endif
#ifndef _RS_SYSTEM_H
#include "rs/system.h"
#endif
// Having problems with cc:Mail?
// Make sure "standard stack frame" and "test stack overflow" are turned on!

#ifdef XP_WIN
#define CMC_USE_MSWIN_NT
#define VIM_USE_MSWIN_NT
#else
#ifndef NO_CMC_MAIL
#define NO_CMC_MAIL
#endif
#ifndef NO_NOTES_MAIL
#define NO_NOTES_MAIL
#endif
#ifndef NO_VIM_MAIL
#define NO_VIM_MAIL
#endif
#ifndef NO_MAPI_MAIL
#define NO_MAPI_MAIL
#endif
#ifndef NO_EXCHANGE_MAIL
#define NO_EXCHANGE_MAIL
#endif
#endif

#ifndef NO_CMC_MAIL
 #ifdef XP_WIN
  #if defined( __BORLANDC__) && (__BORLANDC__ <= 0x500)
   #include <win32/xcmc.h>
   #include <win32/xcmcext.h>
   #include <win32/xcmcmsxt.h>
  #else
   #include <xcmc.h>
   #include <xcmcext.h>
   #include <xcmcmsxt.h>
  #endif
 #else
  #include "xcmc.h"
  #include "xcmcext.h"
  #include "xcmcmsxt.h"
 #endif
 #define CMC_API FAR PASCAL
 #define CMC_CALLBACK CMC_API *
 #include "rs/ml_cmc.h"
#endif

#ifndef NO_VIM_MAIL
 #include "vimlcl.h"
 #include "vim.h"
 #include "vimext.h"
 #define VIM_API VIMAPIENTRY
 #include "vimfunc.h"
#endif

#ifndef NO_POP_MAIL
 #include "rs/ml_mime.h"
#endif

#ifndef NO_MAPI_MAIL
 #include <mapi.h>
#endif

/** \file mail.h
*                       Mail utility functions
*/

/** the only types which really matter are POP3, NOTES, CCMAIL, EXCHANGE, VIM, CMC */

enum EMailType { email_None        =0
                 ,email_TryAll     =1

                 ,email_ccMail5    =2
                 ,email_BeyondMail =4
                 ,email_VIM        =7

                 ,email_Notes      =3

                 ,email_MSMail     =5
                 ,email_MAPI       =5
                 ,email_Exchange   =6
                 ,email_ccMail8    =8
                 ,email_ccMail     =8
                 ,email_GroupWise  =11
                 ,email_CMC        =9

                 ,email_POP3       =10
                 };


/// i and imax are -1 to report an error.
typedef bool (*MailCallback)(void*v,int i, int imax,const char * title);

class MailMessage;
class MailLibrary;
class TMailList;

/*******************************************************************************
*                       CMC login & DLL loading                                *
*     all mail types are supported except VIM and BeyondMail                   *
*******************************************************************************/

class MailMessage
   {
     public:
      bool Deleted;
      MailMessage();// {Deleted = false;}

      virtual ~MailMessage();//  {}; //deallocates space

      virtual char * Sender()=0;
      virtual char * Subject()=0;
      virtual char * MsgText()=0;
      virtual char * HTMLText() {return 0;}
       ///returns 0 for err. Lets you access extended header fields.
      virtual const char * Header(const char * key) {return 0;}
      virtual void Time(SYSTEMTIME& tm)=0;
      virtual int32 Size()=0;
      virtual bool IsRead()=0;
      virtual int AttachCount() {return 0;};
       ///start i at zero, then keep calling GA() while GA() returns true.
      virtual bool GetAttachment(size_t i,TStr& title,TStr& file,bool getfile)
                  {return false;};
      void DeleteMessage() {Deleted = !Deleted;}
  };

class  MailLibrary// : public TDataSource
 {public:
   MailLibrary();
   virtual ~MailLibrary();
   TPointer<xdb> error;

   virtual const char * filename() {return "";}

   bool SendMessage(TNameValueList& message,bool quiet=false,MailCallback CB=NULL,void*v=NULL);

   virtual bool SendDocuments(const char * recip, const char * file,
                              const char * subject, const char * textnote,
                              bool quiet=false,
                              MailCallback CB=NULL,void*v=NULL);

   virtual bool SendMultipart(const char * recip,
                              const char * subject,
                              const char * files,
                              const char * textnote,
                              const char * HTMLnote,
                              bool quiet=false,
                              MailCallback CB=NULL,void*v=NULL);

   virtual bool HasSendUI() {return false;}

   virtual bool GetMessages(TList<MailMessage> &msgs,
                            bool UnreadOnly, TStr &Error,
                             MailCallback CB=NULL,void*v=NULL);

//   inline bool GetMessages(TMailList& msgs,bool UnreadOnly, TStr &Error,
//                             MailCallback CB=NULL,void*v=NULL);
// remove this to prevent duplication. TMailList calles GetMessages in
// its constructor.

 }; //end of TMailUser class

/*******************************************************************************
*                       Mail inbox                                             *
*******************************************************************************/

class TMailList : public DataTable
{
 public:
 MailLibrary* lib;
 bool AutoDelete;
 TList<MailMessage> msgs;
 TChars TempBuffer;

 TMailList(EMailType MailType,const char * username,
           const char* password,const char * server,
           MailCallback CB=NULL,void*v=NULL);
 TMailList(MailLibrary* lib,bool AutoDeleteLibrary = false,
           MailCallback CB=NULL,void*v=NULL);
 ~TMailList();

 const char * filename() {return lib->filename();}

 int Count() {return msgs.Count();}
 MailMessage* operator [] (size_t i) {return msgs[i];}
 void Clear() {msgs.Clear();}

 void Add(MailMessage* d) {msgs.Add(d);}
 bool Has(size_t i) {return msgs.Has(i);}

 const char * GetDataC(count_t i, size_t j);
 bool SetDataC(count_t i, size_t j,const char * c);

 count_t RowCount() {return msgs.Count();}
 size_t ColumnCount() {return 5;}

 const char * ColumnTitle(size_t index);
 void GetConnectString(TStr& x)  {};

 bool Refresh(MailCallback CB,void*v);
};

//inline bool MailLibrary::GetMessages(TMailList& msgs,bool UnreadOnly,
//                                     TStr &Error, MailCallback CB,void*v)
//{
// return GetMessages(msgs.msgs,UnreadOnly,Error,CB,v);
//}

/*******************************************************************************
*                      Open Mail System                                        *
*******************************************************************************/

MailLibrary* OpenMailSystem(EMailType MailType=email_TryAll,
                            bool Login = true,
                            const char * login = NULL,
                            const char * password = NULL,
                            const char * server = NULL,
                            const char * smtpserver = NULL,
                            const char * language = NULL,
                            const char * address = NULL);

/*******************************************************************************
*                      UFill style attachments                                 *
*******************************************************************************/

int PmAppendData(const char * DefDatFileName,
                 TPointer<Stream>& Pmff,
                 Stream& log);

int PmAppendData(const char * DefDatFileName,const char * PmFileName,
                Stream& log);

/*******************************************************************************
*                      Specific mail types                                     *
*******************************************************************************/


/*******************************************************************************
*                       VIM Mail message                                           *
*******************************************************************************/
#ifndef NO_VIM_MAIL

class  VIMMailLibrary : public MailLibrary
 {public:
   FVIMCloseMessage          VIMCloseMessage;
   FVIMCloseSession          VIMCloseSession;
   FVIMCreateMessage         VIMCreateMessage;
   FVIMSetMessageHeader      VIMSetMessageHeader;
   FVIMSetMessageItem        VIMSetMessageItem;
   FVIMSetMessageRecipient   VIMSetMessageRecipient;
   FVIMSendMessage           VIMSendMessage;
   FVIMGetDefaultSessionInfo VIMGetDefaultSessionInfo;
   FVIMOpenSession           VIMOpenSession;
   FVIMInitialize            VIMInitialize;
   FVIMTerminate             VIMTerminate;
   FVIMGetMessageItem        VIMGetMessageItem;
   FVIMOpenMessageItem       VIMOpenMessageItem;
   FVIMOpenMessage           VIMOpenMessage;
   FVIMOpenMessageContainer  VIMOpenMessageContainer;
   FVIMMarkMessageAsRead     VIMMarkMessageAsRead;
   FVIMEnumerateMessageItems VIMEnumerateMessageItems;
   FVIMEnumerateMessages     VIMEnumerateMessages;
   FVIMCloseMessageContainer VIMCloseMessageContainer;
   FVIMRemoveMessage         VIMRemoveMessage;

   TStr Path,Name,Password;
   HINSTANCE lib;
   vimSes session;
   vimMsgContainer msgContainer; // message container identifier

   VIMMailLibrary(bool Login = true, const char* name=NULL,
                  const char* password=NULL, const char* server=NULL);

   ~VIMMailLibrary();

   const char * filename() {return "vim://";}

   bool SendDocuments(const char * recip, const char * file,
                      const char * subject, const char * textnote,
                      bool quiet=false,
                      MailCallback CB=NULL,void*v=NULL);

   bool GetMessages(TList<MailMessage> &msgs,bool UnreadOnly,
                            TStr &Error,
                              MailCallback CB=NULL,void*v=NULL);
   protected:
   bool openSession();
   bool doSendDocuments(const char * recip, const char * file,
                        const char * subject, const char * textnote);

   bool doGetMessages(TList<MailMessage> &msgs,bool UnreadOnly,
                            TStr &Error,MailCallback CB,void*v);
   bool openInbox();
   bool closeInbox();
 }; //end of TVIMMailUser class

class VIMMailMessage : public MailMessage
   {
     public:
      TStr sender, subject;
      MemoryStream Text;
      bool read;
      SYSTEMTIME time;
      vimMsg msgId;
      vimRef msgRef;
     protected:
      VIMMailLibrary * L;

    public:
      VIMMailMessage(VIMMailLibrary * lib);
      ~VIMMailMessage(); //deallocates space

      char * Sender() {return sender;}
      char * Subject() {return subject;}
      char * MsgText();
      void Time(SYSTEMTIME& tm) {tm=time;}
      int32 Size() {return Text.size();}
      bool IsRead() {return read;}
      int AttachCount();
      bool GetAttachment(size_t i, TStr& title,TStr& file,bool getfile);
  };

#endif

/*******************************************************************************
*                       CMC Mail message                                       *
*******************************************************************************/
#ifndef NO_MAPI_MAIL

bool MAPISendDocuments(const char * recip, const char * file,
 const char * subject, const char * textnote,HWND win,
 const char* name, const char * password, bool quiet);

class MAPIMailLibrary : public MailLibrary
 {protected:
   HINSTANCE dllhandle;

  public:
#ifdef __BORLANDC__
   MAPIFREEBUFFER          *MAPIFreeBuffer;
   MAPIRESOLVENAME         *MAPIResolveName;
   MAPISENDMAIL            *MAPISendMail;
   MAPILOGON               *MAPILogon;
   MAPILOGOFF              *MAPILogoff;
   MAPIREADMAIL            *MAPIReadMail;
   MAPIFINDNEXT            *MAPIFindNext;
   MAPIDELETEMAIL          *MAPIDeleteMail;
#else
   LPMAPIFREEBUFFER        MAPIFreeBuffer;
   LPMAPIRESOLVENAME       MAPIResolveName;
   LPMAPISENDMAIL          MAPISendMail;
   LPMAPILOGON             MAPILogon;
   LPMAPILOGOFF            MAPILogoff;
   LPMAPIREADMAIL          MAPIReadMail;
   LPMAPIFINDNEXT          MAPIFindNext;
   LPMAPIDELETEMAIL        MAPIDeleteMail;
#endif
typedef HRESULT (FAR PASCAL *MAPIINIT)(LPVOID lpMapiInit);
typedef void (FAR PASCAL *MAPITERM)();

   LHANDLE   sessionID;

   TStr name, password, profile;

   MAPIMailLibrary(bool Login = true,const char* login=NULL, const char* password=NULL,
                   const char* service = NULL);

   ~MAPIMailLibrary();

   bool SendDocuments(const char * recip, const char * file,
                      const char * subject, const char * textnote,
                      bool quiet=false,MailCallback CB=NULL,void*v=NULL);

   bool GetMessages(TList<MailMessage> &msgs,bool UnreadOnly,TStr &Error,
                              MailCallback CB=NULL,void*v=NULL);

   bool HasSendUI() {return true;}

   const char * filename() {return "mapi://";}

  protected:
   bool initMAPI();
   unsigned doLogon(char * DefUID=0, char *DefPWD=0, char * POPath=0);
   unsigned doLogoff();
 }; //end of TMailUser class

class MAPIMailMessage : public MailMessage // defined in ml_msg.cpp
   {
     public:
    //  bool AutoFree;

      MapiMessage * message;
      char msgid[512];
      MAPIMailLibrary* library;

     ///used by TCMCMailLibrary's GetMessages() function
     MAPIMailMessage(MAPIMailLibrary*, char id[512]);

     ~MAPIMailMessage(); //deallocates space

     char * Sender();
     char * Subject();
     char * MsgText();
     void Time(SYSTEMTIME& tm);
     int32 Size();
     bool IsRead();
     bool GetAttachment(size_t i, TStr& title,TStr& file,bool getfile);
     int AttachCount();

    protected:
     void LoadMessage();
  };
#endif

#ifndef NO_CMC_MAIL
class CMCMailLibrary : public MailLibrary
 {protected:
   HINSTANCE hCMCLibrary;

  public:
   EMailType flavor;

   PCMCSEND                CMCSend;
   PCMCSENDDOCUMENTS       CMCSendDocuments;
   PCMCACTON               CMCActOn;
   PCMCLIST                CMCList;
   PCMCREAD                CMCRead;
   PCMCLOOKUP              CMCLookUp;
   PCMCFREE                CMCFree;
   PCMCLOGOFF              CMCLogoff;
   PCMCLOGON               CMCLogon;
   PCMCQUERYCONFIG         CMCQueryConfig;

   CMC_session_id   cmcSessionID;

   TStr name, password, profile;

   CMCMailLibrary(EMailType MailType=email_TryAll,
                   bool Login = true,const char* login=NULL, const char* password=NULL,
                   const char* service = NULL);

   ~CMCMailLibrary();

   bool SendDocuments(const char * recip, const char * file,
                      const char * subject, const char * textnote,
                      bool quiet=false,MailCallback CB=NULL,void*v=NULL);

   bool GetMessages(TList<MailMessage> &msgs,bool UnreadOnly,TStr &Error,
                              MailCallback CB=NULL,void*v=NULL);

   bool HasSendUI() {return flavor != email_GroupWise;}

   const char * filename() {return "cmc://";}

  protected:
   bool initCMC(EMailType MailType);
   CMC_return_code doLogon(char * DefUID=0, char *DefPWD=0, char * POPath=0);
   CMC_return_code doLogoff();
 }; //end of TMailUser class

class CMCMailMessage : public MailMessage // defined in ml_msg.cpp
   {
     public:
    //  bool AutoFree;

      CMC_message * message;
      CMC_message_summary * summary;

      class TAutoDelete
      {public:
       CMC_message_summary* summary;
       CMCMailLibrary* library;
       TAutoDelete();
       ~TAutoDelete();
      };

      TEnvelope<TAutoDelete> autofree;
      CMCMailLibrary* library;

     ///used by TCMCMailLibrary's GetMessages() function
     CMCMailMessage(CMCMailLibrary*, CMC_message_summary * summary);

     ~CMCMailMessage(); //deallocates space

     char * Sender();
     char * Subject();
     char * MsgText();
     void Time(SYSTEMTIME& tm);
     int32 Size();
     bool IsRead() { return summary->summary_flags & CMC_SUM_READ; }
     bool GetAttachment(size_t i, TStr& title,TStr& file,bool getfile);
     int AttachCount();

    protected:
     void LoadMessage();
  };
#endif

/*******************************************************************************
*                      POP3/SMTP mail type                                   *
*******************************************************************************/

#ifndef NO_POP_MAIL
class POPMailMessage;
class POPMailLibrary : public MailLibrary
{
 protected:
  TStr smtpserver;
  TStr popserver;
  TStr username;
  TStr password;
  TStr host;
  TStr codepage;
  TStr address;
  char reply[256]; //last server reply

  void POPLogOut();
  bool POPLogIn();
  TPointer<InternetStream> socket;

  void SendList(InternetStream& ss, TStringList& names,const char fmt[], TStringList&errors);
  int sendrecv(InternetStream &ss, int code, const char fmt[], const char arg[]);
  int sendrecv(InternetStream &ss, TStr &Error, const char fmt[], const char arg[]);
  int GetPOPReply(InternetStream &ss, TStr &Error);
  int GetSMTPReply(InternetStream &ss, int code);
  int GetSMTPReply(InternetStream &ss,Stream* result = 0);

  public:

  POPMailLibrary(const char*Smtpserver,
                 const char*Popserver,
                 const char*Username, // complete email address
                 const char*Password,
                 const char*codepage=NULL,
                 const char*address=NULL);
  ~POPMailLibrary();

  bool SendDocuments(const char * recip, const char * file,
                     const char * subject, const char * textnote,
                     bool quiet=false,MailCallback CB=NULL,void*v=NULL);

  bool SendMultipart(const char * recip,
                              const char * subject,
                              const char * files,
                              const char * textnote,
                              const char * HTMLnote,
                              bool quiet=false,
                              MailCallback CB=NULL,void*v=NULL);

  bool GetMessages(TList<MailMessage> &msgs,bool UnreadOnly,TStr &Error,
                              MailCallback CB=NULL,void*v=NULL);

  const char * filename() {return smtpserver;}

  int DelMessage(const char msgid[]);
  const char* GetPassword();
  const char* GetSMTPServer();

  protected:

  void SMTPLogin(InternetStream& ss);
  bool SendRecip(InternetStream& ss, const char * recip,TStr &Errors);
  void SendAttachments(InternetStream& ss, const char * files,const char * start,
                        MailCallback CB,void*v);
  void SendText(InternetStream& ss, const char * text,MailCallback CB,void*v);
  void Load(const char* cmd,Stream& out);
  FRIEND POPMailMessage;
};

class POPMailMessage : public MailMessage
{
 protected:
  mimemail* msgtree;
  TList<mmatom> Attachments;
  mmatom* textmessage;
  mmatom* htmlmessage;
  TParameterList header2;
  MemoryStream contents,decodedText,decodedHTML;
  bool isread;

 public:
  POPMailLibrary *server;
  char msgid[10];

  POPMailMessage();
  ~POPMailMessage();

  const char * Header(const char*c);
  char * Sender();
  char * Subject();
  char * MsgText();
  char * HTMLText();
  void Time(SYSTEMTIME& tm);
  int32 Size();
  bool IsRead();
  int AttachCount();
  bool GetAttachment(size_t i, TStr& title,TStr& file,bool getfile);

  protected:
  FRIEND POPMailLibrary;
  void parse();
 // void savedetails(const char *p);
 // void loaddetails(const char *p);
  void LoadMessage();
  void LoadHeader();
  void Load(const char* cmd, Stream& out);
  void IterateAtoms(mmcomp* c);

};

#endif

/*******************************************************************************
*                       Notes Mail message                                  *
*******************************************************************************/
#ifndef NO_NOTES_MAIL

class NSFMailLibrary;

class NSFMailMessage : public MailMessage // defined in ml_msg.cpp
   {
    public:
    class TAutoDelete
      {public:
       HANDLE hMessageFile,hMessageList;
       NSFMailLibrary* library;
       TAutoDelete();
       ~TAutoDelete();
      };

    TEnvelope<TAutoDelete> autofree;

     HANDLE hMessage;
     void *MessageList;
     int Msg;
     DWORD msgsize;
     TStr Text,_Sender,_Subject;

     NSFMailLibrary* library;

     NSFMailMessage(NSFMailLibrary*, int num);

     ~NSFMailMessage(); //deallocates space

     char * Sender();
     char * Subject();
     char * MsgText();
     void Time(SYSTEMTIME& tm);
     int32 Size();
     bool IsRead();
     bool GetAttachment(size_t i, TStr& title,TStr& file,bool getfile);
     int AttachCount();
     protected:
     void LoadMessage();
  };

class TNotesLib;

class NSFMailLibrary : public MailLibrary
 {protected:
   HINSTANCE hNSFLibrary;
   TNotesLib* Notes;

  public:
   TStr ServerName,MailFile,MailFilePath,UserName;

   /// can't prompt for a password -- the API won't take it.
   NSFMailLibrary(const char* servername,const char * dbname,
                   const char* name, const char * password);

   ~NSFMailLibrary();

   bool SendDocuments(const char * recip, const char * file,
                      const char * subject, const char * textnote,
                      bool quiet=false,MailCallback CB=NULL,void*v=NULL);

   bool GetMessages(TList<MailMessage> &msgs,bool UnreadOnly,TStr &Error,
                              MailCallback CB=NULL,void*v=NULL);

   bool HasSendUI() {return false;}

   const char * filename() {return MailFilePath;}

   protected:
   bool DoSendDocuments(char * recip, char * file,
                      char * subject, char * textnote,TStr &Error);
   bool DoGetMessages(TList<MailMessage> &msgs,bool UnreadOnly,TStr &Error,
                              MailCallback CB=NULL,void*v=NULL);
   FRIEND NSFMailMessage;
   FRIEND NSFMailMessage::TAutoDelete;
 }; //end of TMailUser class

#endif

#endif
