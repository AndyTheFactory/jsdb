#include "rslib.h"
#pragma hdrstop

#ifndef NO_VIM_MAIL

#include <commdlg.h>

#pragma message Be sure to include ML_UI.RC
// functions

//static void Error(const char * s,const char * f=0) {throw xdb(s,f); }
  //THROWMSG(s);}
#define DOLoadFunc(type,func,name) \
 func = (type)GetProcAddress(lib,name);\
 if (*func ==0) {error = new xdb("VIM function missing","name",name); return;}

bool extern GetCCMailDirectory(TStr & dir)
{
  TStr LibName(MAXPATH) ;
  if (RegGetKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths",
                "WMAIL32.EXE",LibName, HKEY_LOCAL_MACHINE))
   {
    GetDirectory(LibName,dir);
    AddBackslash(dir);
    return true;
   }
  if (RegGetKey(".cc1\\Shell\\Open\\Command", "",LibName, HKEY_CLASSES_ROOT))
   {
    GetDirectory(LibName,dir);
    AddBackslash(dir);
    return true;
   }

  return false;
}

VIMMailLibrary::VIMMailLibrary(bool logon,const char* name, const char* pwd,const char* server)
 : MailLibrary(),
  Name(name), Password(pwd), Path(server)
{
 session=0;
 msgContainer=0;

// dspMessageBox("Trying to load VIM.DLL","");
  const char * libname = "VIM32.DLL"; //const char * modname = "VIM32";

 lib = LoadLibrary(libname);
 if (!lib)
 {
  TStr file;
   GetCCMailDirectory(file);
   lib = RSLoadLibrary(TStr(file,libname));
 }
 if (!lib)
 {
  TStr NotesDir,s;
  if (RegGetKey(".nsf\\Shell\\Open\\Command",0, NotesDir,HKEY_CLASSES_ROOT))
   {
    GetDirectory(NotesDir,s);
    AddBackslash(s);
    s += libname;
    lib = RSLoadLibrary(s);
   }
 }



 if (!lib) error = new xdb("Unable to load the VIM32.DLL library. "
                 "It should be in your path. Please check that the file "
                 "is available and try again.","library",libname);
 if (!lib)  return;

 DOLoadFunc(FVIMCloseMessage,          VIMCloseMessage, "VIMCloseMessage");
 DOLoadFunc(FVIMCloseSession,          VIMCloseSession, "VIMCloseSession");
 DOLoadFunc(FVIMCreateMessage,         VIMCreateMessage, "VIMCreateMessage");
 DOLoadFunc(FVIMSetMessageHeader,      VIMSetMessageHeader, "VIMSetMessageHeader");
 DOLoadFunc(FVIMSetMessageItem,        VIMSetMessageItem, "VIMSetMessageItem");
 DOLoadFunc(FVIMSetMessageRecipient,   VIMSetMessageRecipient,"VIMSetMessageRecipient");
 DOLoadFunc(FVIMSendMessage,           VIMSendMessage,"VIMSendMessage");
 DOLoadFunc(FVIMGetDefaultSessionInfo, VIMGetDefaultSessionInfo,"VIMGetDefaultSessionInfo");
 DOLoadFunc(FVIMOpenSession,           VIMOpenSession,"VIMOpenSession");
 DOLoadFunc(FVIMInitialize,            VIMInitialize,"VIMInitialize");
 DOLoadFunc(FVIMTerminate,             VIMTerminate,"VIMTerminate");
 DOLoadFunc(FVIMGetMessageItem        ,VIMGetMessageItem,"VIMGetMessageItem");
 DOLoadFunc(FVIMOpenMessageItem       ,VIMOpenMessageItem,"VIMOpenMessageItem");
 DOLoadFunc(FVIMOpenMessage           ,VIMOpenMessage,"VIMOpenMessage");
 DOLoadFunc(FVIMOpenMessageContainer  ,VIMOpenMessageContainer,"VIMOpenMessageContainer");
 DOLoadFunc(FVIMMarkMessageAsRead     ,VIMMarkMessageAsRead,"VIMMarkMessageAsRead");
 DOLoadFunc(FVIMEnumerateMessageItems ,VIMEnumerateMessageItems,"VIMEnumerateMessageItems");
 DOLoadFunc(FVIMEnumerateMessages     ,VIMEnumerateMessages,"VIMEnumerateMessages");
 DOLoadFunc(FVIMCloseMessageContainer ,VIMCloseMessageContainer,"VIMCloseMessageContainer");
 DOLoadFunc(FVIMRemoveMessage         ,VIMRemoveMessage,"VIMRemoveMessage");
 VIMInitialize();
 if (logon) openSession();
}

VIMMailLibrary::~VIMMailLibrary()
{
 if (msgContainer) closeInbox();
// VIMCloseSession (session);
// this causes problems sometimes -- better to not close the session,
// and leave the resource leak open
 VIMTerminate();
 if (lib) FreeLibrary(lib);
}

bool VIMMailLibrary::GetMessages(TList<MailMessage> &msgs,bool UnreadOnly,
                            TStr &Error,MailCallback CB,void*v)
{
 if (!session) if (!openSession()) {Error = "Login failed"; return false;}
 openInbox();
 return doGetMessages(msgs,UnreadOnly,Error,CB,v);
}


bool VIMMailLibrary::SendDocuments (const char * mailrecip, const char * file,
                const char * subject, const char * textnote,
                bool quiet,MailCallback ,void*)
{

 if (!session) if (!openSession()) return false;
 return doSendDocuments(mailrecip,file,subject,textnote);
}

bool VIMMailLibrary::doSendDocuments (
                const char * mailrecip,  const char * file,
                const char * subject, const char * textnote)
{
    vimStatus  vimSts=0;      /* status returned from VIM calls */
    vimMsg     msg=0;         /* message identifier             */

    vimRecipient * recip = new vimRecipient;
    vimBuffFileDesc * buffFileDesc = new vimBuffFileDesc;
    vimBuffFileDesc * buffMsgDesc = new vimBuffFileDesc;
    char  *pRecipName;

    TStr MailRecip(mailrecip), Subject(subject);
    TStr TextItem("Text Item"), Title(MAXPATH), TextNote(textnote);

    /* Create the message */
try{
    vimSts = VIMCreateMessage (
               session,              /* VIM Session            */
               VIM_MAIL,             /* Type of message        */
               &msg);                /* returned identifier    */

    if (vimSts != VIMSTS_SUCCESS)
     {
      error = new xdb("Unable to create a new message");
//      dspMessageBox("Unable to create a new message",__FILE__  );
      return false;
     } // Fill in recipient structure, one name at a time.

    pRecipName = strtok(MailRecip,",;"); //  Comma is used in some names

    int sendtype = VIMSEL_TO;

    while (pRecipName)
    {
     if (!strcasecmp(pRecipName,"BCC"))
      {
       sendtype = VIMSEL_BCC;
      }
     else if (!strcasecmp(pRecipName,"CC"))
      {
       sendtype = VIMSEL_CC;
      }
     else
      {
        recip->Type = VIMSEL_ENTITY;
        recip->DName.Type = VIMSEL_NATIVE;
        recip->DName.AddressBook[0] = 0; // default address book
        strncpy(recip->DName.Value, pRecipName,sizeof(recip->DName.Value));
        recip->Address.Type = VIMSEL_NATIVE;
        recip->Address.Value[0] = 0;

        vimSts = VIMSetMessageRecipient (
                    msg,
                    sendtype,
                    recip);

        if ( VIM_STATUS (vimSts) != VIMSTS_SUCCESS )
        {
      error = new xdb("Unable to add a recipient","name",pRecipName);
//         dspMessageBox(TStr("Unable to add a recipient: ",pRecipName),__FILE__  );
         return false;
        }
      }
      pRecipName = strtok(0,",;");   //  remove comma

    } //while (pRecipName != NULL);

   //subject

    size_t subjectlength = strlen(subject);
    vimSts = VIMSetMessageHeader (
               msg,
               VIMSEL_SUBJECT,
               subjectlength,
               (void*)(char*)Subject);

    if ( VIM_STATUS (vimSts) != VIMSTS_SUCCESS )
    {
    error = new xdb("Unable to set the message subject");
//        dspMessageBox("Unable to set the message subject",__FILE__  );
        return false;
    }

    //message text
    buffMsgDesc->Buffer = (void*)(TextNote);
    buffMsgDesc->Size = strlen(TextNote);
    buffMsgDesc->Offset = 0L;
    buffMsgDesc->FileName = NULL;
    vimSts = VIMSetMessageItem(
               msg,             /* opened message identifier   */
               VIMSEL_NOTE_PART,   /* class of item            */
               VIM_TEXT,           /* type of item             */
               VIM_NO_FLAGS,    /* attachment flags         */
               (char*)TextItem,        /* note title               */
               buffMsgDesc);     /* contents of item         */

    if (VIM_STATUS(vimSts) != VIMSTS_SUCCESS)
    {
     error = new xdb("Unable to set the message text");
//        dspMessageBox("Unable to set the message text",__FILE__  );
        return false;
    }

    if (file && *file)
    {
    TStringList fnames(file,",;");
    FOREACH(char * File,fnames)
        buffFileDesc->Buffer = NULL;
        buffFileDesc->Size = 0L;
        buffFileDesc->Offset = 0L;
        buffFileDesc->FileName = File;

        /* extract the filename from the entire path */

        GetFileTitle(File,Title,MAXPATH);

        vimSts = VIMSetMessageItem(msg, /* opened message identifier */
                        VIMSEL_ATTACH,  /* class of item             */
                        NULL,           /* type of item              */
                        VIMSEL_NATIVE,  /* attachment flags          */
                        Title,      // filename stored with attachment
                        buffFileDesc); /* contents of item          */

        if (VIM_STATUS(vimSts) != VIMSTS_SUCCESS)
        {
         error = new xdb("Unable to add the file attachment","name",File);
//            dspMessageBox(TStr("Unable to add the file attachment: ",File),__FILE__  );
            return false;
        }
    DONEFOREACH
    }

    vimSts = VIMSendMessage (
               msg,             /* opened message identifier   */
               NULL,            /* pointer to callback param   */
               NULL);           /* callback function pointer   */

    if ( VIM_STATUS (vimSts) != VIMSTS_SUCCESS )
    {
         error = new xdb("Unable to send the message");
//        dspMessageBox("Unable to send the message",__FILE__  );
        return false;
    }

    msg = 0; //don't VIMClose the message, as that's done
             //automatically by VIMSend

   } //try
  catch (...) {}

  if (msg) VIMCloseMessage (msg);
  delete recip;
  delete buffFileDesc;
  delete buffMsgDesc;
//  if (session) f->VIMCloseSession (session);
    return vimSts == VIMSTS_SUCCESS;
}

#define IDD_PATH   102
#define IDD_NAME   104
#define IDD_PASS   106

/************************************************************************/

/************************************************************************

    FUNCTION:   openVIMSession

    PURPOSE:    Determines what information is necessary to open and
                log into session, prompts the user for the necessary
                information and then opens the session.  The session
                pointer is returned.

***********************************************************************/
//extern bool promptVIMLogin (void * xbuf);
//extern bool promptPassword (TStr& password);
//in ml_ui.cpp

/*
const char * VimError(int i)
{
static const char * s[] = {
"FAILURE",
"FATAL",
"ALL_PARAMS_REQUIRED",
"ATTACHMENT_NOT_FOUND",
"BAD_PARAM",
"BUF_TOO_SMALL",
"CONV_NOT_SUPPORTED",
"INSUFFICIENT_MEMORY",
"INVALID_CONFIGURATION",
"INVALID_OBJECT",
"INVALID_PASSWORD",
"INVALID_SELECTOR",
"INVALID_SIGNATURE",
"NAME_EXISTS",
"NAME_NOT_FOUND",
"NOT_SUPPORTED",
"NO_COMMON_CERTIFICATES",
"NO_DEFAULT",
"NO_MATCH",
"NO_SIGNATURE",
"NO_SUCH_ATTRIBUTE",
"OPEN_FAILURE",
"PASS_REQUIRED",
"READ_FAILURE",
"UNSUP_TYPE",
"UNSUP_VERSION",
"WRITE_FAILURE",
"UNABLE_CREATE_DRAFT"};
return (i <= 28) ? s[i-1] : "Unknown Error";
}
  */

bool VIMMailLibrary::openSession()
{
 vimStatus   vimSts;
 if (session)  VIMCloseSession(session);

 if (!*Name)
  {//try to open it with the default settings
   vimSts = VIMOpenSession (
                   0,// Path,        // message container path
                   0,// Name,        // name of user logging in
                   0,// Password,    // password of user
                    VIM_CURRENT_VERSION,
                    VIMSEL_CP1252,  // Windows character set
                    &session);      // returned session pointer

   if (vimSts != VIMSTS_SUCCESS)
   {
    Path.Resize(MAXPATH);
    Name.Resize(MAXPATH);

    vimSts = VIMGetDefaultSessionInfo (
                    MAXPATH,         /* message container path length   */
                    Path, /* returned message container path */
                    MAXPATH,       /* user name length                */
                    Name);/* returned user name            */

    vimSts = VIMOpenSession (Path,Name,Password,
                             VIM_CURRENT_VERSION,VIMSEL_CP1252,&session);

   }
  }
 else
  {
    vimSts = VIMOpenSession (Path,Name,Password,
                             VIM_CURRENT_VERSION,VIMSEL_CP1252,&session);

    if (vimSts != VIMSTS_SUCCESS)
    {
     if (!*Path)
     {
      Path.Resize(MAXPATH);
      vimSts = VIMGetDefaultSessionInfo (
                    MAXPATH,         /* message container path length   */
                    Path, /* returned message container path */
                    0,       /* user name length                */
                    NULL);/* returned user name            */
     }

     vimSts = VIMOpenSession (Path,Name,Password,
                             VIM_CURRENT_VERSION,VIMSEL_CP1252,&session);
    }
  }

  //did it work?

  if ( VIM_STATUS (vimSts) == VIMSTS_PASS_REQUIRED )
    {
     return false; //wrong password
    }
  else if ( VIM_STATUS (vimSts) == VIMSTS_ALL_PARAMS_REQUIRED )
    {
       // Get the default database path specification and user name
       //   from the user configuration file
       // must've been a problem with the user-supplied parameters.
       //recheck the default ones
    Path.Resize(MAXPATH);
    Name.Resize(MAXPATH);

    vimSts = VIMGetDefaultSessionInfo (
                    MAXPATH,         /* message container path length   */
                    Path, /* returned message container path */
                    MAXPATH,       /* user name length                */
                    Name);/* returned user name            */

    vimSts = VIMOpenSession (
                    Path,
                    Name,
                    Password,
                    VIM_CURRENT_VERSION,
                    VIMSEL_CP1252,             /* Windows character set      */
                    &session);              /* returned session pointer   */
    }
 return (vimSts == VIMSTS_SUCCESS);
}


bool VIMMailLibrary::openInbox()
{
   /* open the mail box */
   if (msgContainer) return true;
   if (!session) openSession();
   msgContainer = 0;
   VIMOpenMessageContainer (
               session,                /* session identifier   */
               NULL,                   /* open default mailbox */
               &msgContainer);         /* container identifier */
   return (msgContainer != 0);
}

bool VIMMailLibrary::doGetMessages(TList<MailMessage> &msgs,bool UnreadOnly,
                            TStr &Error,MailCallback CB, void *v)
{
   vimStatus   vimSts;           // status returned from VIM calls
   TStr        szErrorString;    // String to be used for error messages
   TStr        szExtErrorString; // String to be used for
                                 //   extended error messages.
   int nResult;
   vimEnumRef refPos;            // message reference position

   vimBool       fMore=0;
   vimWord       msgCount = 1;   // Number messages to be read at a time
   vimWord       wPos;
   TStr          subject(2048);
   TStr          sender(1024);
   vimDate       Time,ReadDate;
   vimRef        msgRef;

   /* three requested attributes:  date, who, subject */
   vimAttrDesc attrs[4];

   attrs[0].Attr = VIMSEL_DATE;
   attrs[0].Size = sizeof(Time);
   attrs[0].Buffer = &Time;
   attrs[1].Attr = VIMSEL_FROM_NAME;
   attrs[1].Size = 1024;
   attrs[1].Buffer = sender;
   attrs[2].Attr = VIMSEL_SUBJECT;
   attrs[2].Size = 2048;
   attrs[2].Buffer = subject;
   attrs[3].Attr = VIMSEL_REF;
   attrs[3].Size = sizeof(msgRef);
   attrs[3].Buffer = &msgRef;
//   attrs[4].Attr =    VIMSEL_READ_DATE;
//   attrs[4].Size = sizeof(ReadDate);
//   attrs[4].Buffer = &ReadDate;

   if (!openInbox())
   {
      return false;
   }

   /* Show each unrea\d message - up to MAXMSG */

   VIM_UND_REF(refPos);             /* initialize refPos       */
   int msgnum = 0;
   do
   {
    if (CB) if (!CB(v,msgnum,0,"Reading")) return false;
    msgnum++;

      /* get information about next unread message */
      vimSts = VIMEnumerateMessages (
                  msgContainer,    /* container identifier     */
                  &refPos,         /* starting position        */
                  1,               /* skip count               */
                  4,//5,               /* num attributes requested */
                  attrs,           /* attribute pointer        */
                  &msgCount,       /* message count            */
                  VIMSEL_NO_FILTER,/* filter                   */
                  NULL,            /* filter data              */
                  UnreadOnly ? VIM_UNREADONLY : NULL,
                  &fMore);          /* more flag               */

      if ( VIM_STATUS (vimSts) != VIMSTS_SUCCESS )
      {
         return false;
      }

       /* translate the date structure to ascii, mm/dd/yy */

       VIMMailMessage * msg = new VIMMailMessage(this);

       vimSts = VIMOpenMessage(
                  msgContainer,
                  msgRef,
                  NULL,                       /* message is not encrypted  */
                  &msg->msgId);                      /* ptr to message identifier */

       msg->msgRef = msgRef;
       msg->subject = subject;
       msg->sender = sender;
       msg->time.wYear = HIWORD(Time.Date);
       msg->time.wMonth=HIBYTE(LOWORD(Time.Date));
       msg->time.wDayOfWeek=0;
       msg->time.wDay=LOBYTE(LOWORD(Time.Date));;
       msg->time.wHour=HIBYTE(HIBYTE(Time.Time));
       msg->time.wMinute=LOBYTE(HIWORD(Time.Time));
       msg->time.wSecond=HIBYTE(LOWORD(Time.Time));
       msg->time.wMilliseconds=LOBYTE(LOWORD(Time.Time))*10;

       msg->read = (ReadDate.Date == VIM_UNKNOWN_DATE);
       msgs.Add(msg);

   } while (fMore);

   if (CB) CB(v,msgnum,msgnum,"Done");

   return true;
}

VIMMailMessage::VIMMailMessage(VIMMailLibrary * lib)
{
 L=lib;
 Deleted = false;
}

VIMMailMessage::~VIMMailMessage()
{
 if (Deleted) L->VIMRemoveMessage(L->msgContainer,msgRef);
 L->VIMCloseMessage(msgId);
}

int VIMMailMessage::AttachCount()
{
   vimEnumRef    refPos;
   VIM_UND_REF(refPos);
   vimSelector cls = VIMSEL_ATTACH;
   vimWord count = 1;
   vimItemDesc itemDesc;
   vimBool fMore=0;
   int i=0;

   do {
   L->VIMEnumerateMessageItems (
                  msgId,
                  &refPos,
                  1,
                  &count,
                  &itemDesc,
                  VIMSEL_CLASS,    /* filter                   */
                  &cls,            /* filter data - attachment */
                  &fMore);         /* more flag                */
   if (count) i++;
  } while (fMore);
 return i;
}

bool VIMMailMessage::GetAttachment(size_t i, TStr& title,TStr& file,bool getfile)
{
   vimEnumRef    refPos;
   VIM_UND_REF(refPos);
   vimSelector cls = VIMSEL_ATTACH;
   vimWord count = 1;
   vimItemDesc itemDesc;
   vimBool fMore=0;
   vimStatus vimSts;
   vimBuffFileDesc  fDesc;

   vimSts = L->VIMEnumerateMessageItems (
                  msgId,
                  &refPos,
                  i+1,
                  &count,
                  &itemDesc,
                  VIMSEL_CLASS,    /* filter                   */
                  &cls,            /* filter data - attachment */
                  &fMore);         /* more flag                */

   if ( VIM_STATUS (vimSts) != VIMSTS_SUCCESS )
       return false;

   if (!getfile) return true;

   title = itemDesc.Name;

   CreateTempFile(file);

   fDesc.Buffer   = 0;
   fDesc.Offset   = 0;
   fDesc.Size     = 0;
   fDesc.FileName = file;

   vimSts = L->VIMGetMessageItem (
          msgId,              /* message identifier            */
          itemDesc.Ref,       /* item reference number         */
          NULL,               /* does not apply to attachments */
          VIMSEL_NATIVE,      // write output file in native
                              // file system's format
          &fDesc);            /* output descriptor             */

   if (vimSts != VIMSTS_SUCCESS)
     {
      return false;
     }

   if ( VIM_STATUS (vimSts) != VIMSTS_SUCCESS )
       return false;

   return true;
}

char * VIMMailMessage::MsgText()
{
 if (Text.size()==0)
  {
   vimEnumRef    refPos;
   VIM_UND_REF(refPos);
   vimSelector cls = VIMSEL_NOTE_PART;
   vimWord count = 1;
   vimBool fMore=0;
   vimItemDesc itemDesc;
   vimBuffFileDesc  fDesc;

   //grabs the first text message item only
   vimStatus vimSts = L->VIMEnumerateMessageItems (
                  msgId,            /* message identifier      */
                  &refPos,          /* starting position       */
                  1,                /* skip count              */
                  &count,
                  &itemDesc,        /* item descriptor pointer */
                  VIMSEL_CLASS,     /* filter                  */
                  &cls,             /* filter data - note part */
                  &fMore);          /* more flag               */

    if ( VIM_STATUS (vimSts) != VIMSTS_SUCCESS )
         return 0;

    Text.Resize(itemDesc.Size);
    fDesc.Buffer = Text;
    fDesc.Size   = itemDesc.Size;
    fDesc.Offset = 0;
    vimSts = L->VIMGetMessageItem (
                  msgId,             /* message identifier     */
                  itemDesc.Ref,      /* item reference number  */
                  VIM_TEXT,          /* output data type       */
                  (vimSelector)NULL, /* only for attachments  */
                  &fDesc);           /* output descriptor      */

    L->VIMMarkMessageAsRead(L->msgContainer,msgRef);
    read = true;

    if ( VIM_STATUS (vimSts) != VIMSTS_SUCCESS )
         return 0;
  }
 return Text;
}

bool VIMMailLibrary::closeInbox()
{
   /* open the mail box */
   if (!msgContainer) return true;
   if (!session) return true;
   VIMCloseMessageContainer (msgContainer);
   msgContainer = 0;
   return true;
}

/*    int passwordcount = 3; //try 3 times
    do
      {
        Password[0]=0;
        TStr pass;
        if (!promptPassword (pass))
            return false;
        strncpy(Password,pass,sizeof(Password));

        vimSts = VIMOpenSession (
                    Path,// *Path ? (char*)Path : NULL,
                    Name,// *Name ? (char*)Name : NULL,
                    Password,
                    VIM_CURRENT_VERSION,
                    VIMSEL_CP850,//VIMSEL_CP1252,
                    &session);

      passwordcount --;

      }
    while (passwordcount &&
           (VIM_STATUS (vimSts) != VIMSTS_SUCCESS)) ;
*/

#endif

