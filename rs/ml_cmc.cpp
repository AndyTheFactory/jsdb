#include "rslib.h"
#pragma hdrstop

#ifndef NO_CMC_MAIL
//CMC most functions moved to SENDMAIL.CPP and READMAIL.CPP

/*
const char * _CMCMAILERRLIST [] =
{
"AMBIGUOUS RECIPIENT","ATTACHMENT NOT FOUND","ATTACHMENT OPEN FAILURE",
"ATTACHMENT READ FAILURE","ATTACHMENT WRITE FAILURE",
"COUNTED STRING UNSUPPORTED","DISK FULL","FAILURE","INSUFFICIENT MEMORY",
"INVALID CONFIGURATION","INVALID ENUM","INVALID FLAG","INVALID MEMORY",
"INVALID MESSAGE PARAMETER","INVALID MESSAGE REFERENCE","INVALID PARAMETER",
"INVALID SESSION ID","INVALID UI ID","LOGON FAILURE","MESSAGE IN USE",
"NOT SUPPORTED","PASSWORD REQUIRED","RECIPIENT NOT FOUND","SERVICE UNAVAILABLE",
"TEXT TOO LARGE","TOO MANY FILES","TOO MANY RECIPIENTS",
"UNABLE TO NOT MARK READ","UNRECOGNIZED MESSAGE TYPE","UNSUPPORTED ACTION",
"UNSUPPORTED CHARACTER SET","UNSUPPORTED DATA EXT","UNSUPPORTED FLAG",
"UNSUPPORTED FUNCTION EXT","UNSUPPORTED VERSION","USER CANCEL",
"USER NOT LOGGED ON"};

const char* EXPORTFUNC CMCMailError(CMC_return_code error)
{
 if (error ==0) return "No mail!";
 if (error < 38) return _CMCMAILERRLIST[error-1];
 return "CMCMail: UNKNOWN MAILER ERROR";
}
*/

/* now in sy_mswin.cpp
HINSTANCE RSLoadLibrary(const char * path);
{
     char drive[MAXDRIVE],dir[MAXDIR],name[MAXFILE],ext[MAXEXT];
    char temp[MAXPATH];
    fnsplit(path,drive,dir,name,ext);

    strcpy(temp,drive);
    strcat(temp,dir);

    char olddir[MAXPATH];
    getcwd(olddir,MAXPATH);

    ChangeDirectory(temp);
    HINSTANCE h = LoadLibrary(path);
    ChangeDirectory(olddir);

    return h;
}         */

//cmc Message-----------------------------------------

CMCMailMessage::CMCMailMessage(CMCMailLibrary*lib, CMC_message_summary * sum)
  :MailMessage(),autofree(0)
{
// AutoFree = true;
 Deleted=false;
 message = 0;
 library=lib;
 summary=sum;
}
/*
CMCMailMessage::CMCMailMessage(CMCMailLibrary* lib,bool unread)
  :TMailMessage() {
  Init(lib,0,unread);
 }

CMCMailMessage::CMCMailMessage(CMCMailMessage* msg,bool unread)
  :TMailMessage() {
 if (msg->message)
  Init(msg->library,msg->message->message_reference,unread);
 else
  Init(msg->library,0,true);
 //Can't seek for unread messages after a message reference
 //for a message that has been read!
 }

CMCMailMessage * CMCMailMessage::Next(bool UnreadOnly)
 {
  try {
       return new CMCMailMessage(this,UnreadOnly);
      }
  catch(...)
      {
       return 0;
      }
 }

void CMCMailMessage::Init(CMCMailLibrary * lib,
                        CMC_message_reference* lookafter,
                        bool UnreadOnly)
 {
  library = lib;
  message = 0;
  summary = 0;
  AutoFree= true;
  Deleted = false;

  CMC_return_code cmcSts;

  CMC_uint32 Count = 1;

  cmcSts = lib->CMCList(
              library->cmcSessionID,    // Session handle
              "CMC: IPM",           // normal messages only
              UnreadOnly ? CMC_LIST_UNREAD_ONLY : 0,
              lookafter,            // Starting after message #
              &Count,               // Input/output message count
              0,                    // don't use UI
              &summary,             // Return message summary list.
              NULL);                // No extensions

  if (!Count)
   THROWMSG("You do not have any mail"); // no mail

  if (!summary || cmcSts != CMC_SUCCESS)
   THROWMSG(CMCMailError(cmcSts));
 }
  */

CMCMailMessage::TAutoDelete::TAutoDelete() {}
CMCMailMessage::TAutoDelete::~TAutoDelete() {library->CMCFree(summary);}

CMCMailMessage::~CMCMailMessage()
 {
  if (Deleted && summary)
  library->CMCActOn(library->cmcSessionID,           // Session ID
                    summary->message_reference,  // Message to delete
                    CMC_ACT_ON_DELETE,           // Message to read
                    0,                           // No flags
                    0,                           // No UI
                    NULL);                       // No extensions
  if (message) library->CMCFree(message);
 // if (AutoFree && summary) library->CMCFree(summary);
  library = 0;
 } //deallocates space

void CMCMailMessage::LoadMessage()
 {
  if (message) library->CMCFree(message);
  message = 0;

  library->CMCRead(library->cmcSessionID,
      summary->message_reference,
      0, //CMC_DO_NOT_MARK_AS_READ,
      &message,
      0,
      0);
 }

char * CMCMailMessage::Sender()
 {
   if (!summary) return "";
   if (!summary->originator) return "";
   return summary->originator->name;
 }

void CMCMailMessage::Time(SYSTEMTIME& tm)
 {
   memset(&tm,0,sizeof(tm));
   if (!summary) return;
   tm.wYear = WORD(summary->time_sent.year + 1900);
   tm.wMonth = WORD(summary->time_sent.month+1);
   tm.wDayOfWeek =0;
   tm.wDay = summary->time_sent.day;
   tm.wHour = summary->time_sent.hour;
   tm.wMinute = summary->time_sent.minute;
   tm.wSecond = summary->time_sent.second;
   tm.wMilliseconds =0;
 }

int32 CMCMailMessage::Size()
 {
   if (!summary) return 0;
   return summary->byte_length;
 }

char * CMCMailMessage::Subject()
 {
   if (!summary) return "";
   return summary->subject;
 }

char * CMCMailMessage::MsgText()
 {
   if (!message) LoadMessage();
   if (!message) return "";
   return message->text_note;
 }

int CMCMailMessage::AttachCount()
 {
  if (!message) LoadMessage();
  if (!message) return 0;

  CMC_attachment *Attach = message->attachments;

  if (!Attach) return 0;

  int i = 1;

  if (message->message_flags & CMC_MSG_TEXT_NOTE_AS_FILE)
     {
      if (Attach->attach_flags & CMC_ATT_LAST_ELEMENT) return 0;
      else
      Attach++;
     }

  while (!(Attach->attach_flags & CMC_ATT_LAST_ELEMENT))
  {
   i++;
   Attach++;
  }
  return max(i,0);
 }

bool CMCMailMessage::GetAttachment(size_t i, TStr& title,TStr& file,bool)
 {
  title=0;
  file=0;
  if ( i == NOT_FOUND ) return false;

  if (!message) LoadMessage();
  if (!message) return false;

  CMC_attachment *Attach = message->attachments;

  if (!Attach) return false;

  if (message->message_flags & CMC_MSG_TEXT_NOTE_AS_FILE)
     {// skip the text note attachment
      if (Attach[i].attach_flags & CMC_ATT_LAST_ELEMENT) return false;
      else
      i++;
     }

  title = Attach[i].attach_title;
  file = Attach[i].attach_filename;

  if (!*file) return false;
  if (!*title) title=file;

  return true;
 }


//------------------------------
CMCMailLibrary::CMCMailLibrary(EMailType MailType, bool Login,
         const char* uid, const char* pwd, const char* service):
   //may throw an exception on failure to log in
  MailLibrary(),
  hCMCLibrary(0) , cmcSessionID(0),flavor(MailType),
  name(uid),password(pwd),profile(service)
 {

 if (!initCMC(MailType))
  {
    error = new xdb("Unable to load the mail library");
    return;
  }

 if (!*profile) profile = name;
 CMC_return_code cmcSts;

   if (Login)
   {
     cmcSts = doLogon(name,password,profile);

    if (cmcSts != CMC_SUCCESS)
      {
       error = new xdb("Login failed");
       return;
      }
   }
 }

CMCMailLibrary::~CMCMailLibrary()
 {
   if (cmcSessionID >  0L)
     doLogoff();

//   if (cmcSts != CMC_SUCCESS)
//   {
//      throw xdb("Logoff failure");
//   } this error never happens

   if ((int)hCMCLibrary> 32) FreeLibrary(hCMCLibrary);
 }

HINSTANCE GetMailLibrary(TStr& LibName,EMailType MailType)
{

if (MailType == email_TryAll || MailType == email_Notes || MailType == email_ccMail5)
//first, check for cc:Mail. If that's not availble, look for a
//CMC mail setup
{
 TStr directoryname(MAXPATH);
 if (GetProfileString("LotusMail", "Program", "", directoryname,MAXPATH))
   {
    GetDirectory(directoryname,directoryname);

     const char * libname = "CMC32.DLL";

     if (*directoryname) AddBackslash(directoryname);

     TStr MailLibrary(directoryname,libname);

     HINSTANCE lib = RSLoadLibrary(MailLibrary);

     if ((UINT)lib > 32)
     {
       return lib;
     }
   }
  }

  GetProfileString("Mail", "CMCDLLNAME32", "MAPI32.DLL", LibName, MAXPATH);

// load CMC.DLL
HINSTANCE hCMCLibrary = LoadLibrary(LibName);
if ((UINT) hCMCLibrary >= 32) return hCMCLibrary;

hCMCLibrary = RSLoadLibrary(LibName);
if ((UINT) hCMCLibrary >= 32) return hCMCLibrary;

 LibName="CMC32.DLL";

 hCMCLibrary = LoadLibrary("CMC32.DLL") ;   //lotus
 if ((UINT) hCMCLibrary >= 32) return hCMCLibrary;

 LibName="MAPI32.DLL";

 hCMCLibrary = LoadLibrary("MAPI32.DLL") ;  //microsoft
 if ((UINT) hCMCLibrary >= 32) return hCMCLibrary;

 return 0;
}

bool CMCMailLibrary::initCMC(EMailType MailType)
{
if ((UINT)hCMCLibrary>32) return true;

TStr errmsg(1024);

TStr szCMCLibName(MAXPATH);
hCMCLibrary=GetMailLibrary(szCMCLibName,MailType);

if ( (int)hCMCLibrary <= 32 ) return false;

/*#define DOLoadFunc(type,func,name,namelc)\
func = (type)GetProcAddress(hCMCLibrary,name);\
if (*func ==0) \
 {func = (type)GetProcAddress(hCMCLibrary,namelc);\
 if (*func ==0) \
 {sprintf(errmsg,"CMC Mail: %s\nUnable to load %s address",(char*)szCMCLibName,name);\
 dspMessageBox(errmsg,szCMCLibName);\
 return false;\
 }}    */

#define DOLoadFunc(type,func,name,namelc)\
func = (type)GetProcAddress(hCMCLibrary,namelc);\
if (*func ==0) return false;

DOLoadFunc(PCMCLOGON,CMCLogon,"CMC_LOGON","cmc_logon")   /* Get an entry point to cmc_logon function */
DOLoadFunc(PCMCLOGOFF,CMCLogoff,"CMC_LOGOFF","cmc_logoff")   /* Get an entry point to cmc_logon function */
DOLoadFunc(PCMCSEND,CMCSend,"CMC_SEND","cmc_send")   /* Get an entry point to cmc_logon function */
DOLoadFunc(PCMCREAD,CMCRead,"CMC_READ","cmc_read");
DOLoadFunc(PCMCACTON,CMCActOn,"CMC_ACT_ON","cmc_act_on");
DOLoadFunc(PCMCLIST,CMCList,"CMC_LIST","cmc_list");
DOLoadFunc(PCMCLOOKUP,CMCLookUp,"CMC_LOOK_UP","cmc_look_up");
DOLoadFunc(PCMCFREE,CMCFree,"CMC_FREE","cmc_free");
DOLoadFunc(PCMCQUERYCONFIG,CMCQueryConfig,"CMC_QUERY_CONFIGURATION","cmc_query_configuration");
//DOLoadFunc(PCMCSENDDOCUMENTS,CMCSendDocuments,"CMC_SEND_DOCUMENTS","cmc_send_documents");
CMCSendDocuments=0;
return true;
}

/*mcSts = CMCList(
              cmcSessionID,    // Session handle
              "CMC: IPM",           // normal messages only
              Flags | CMC_LIST_COUNT_ONLY,
              0,            // Starting after message #
              &Count,               // Input/output message count
              0,                    // don't use UI
              0,             // Return message summary list.
              NULL);                // No extensions

  if (!Count || cmcSts != CMC_SUCCESS)
   { Error = CMCMailError(cmcSts); return false; }
  */

bool CMCMailLibrary::GetMessages(TRow<MailMessage> &msgs,bool UnreadOnly,TStr& Error,
MailCallback CB,void*v)
{
  CMC_return_code cmcSts;

  CMC_uint32 Count = 0;
  CMC_flags Flags = (UnreadOnly) ? CMC_LIST_UNREAD_ONLY : 0;

 CMC_message_summary * summary=0;


 cmcSts = CMCList(
              cmcSessionID,    // Session handle
              0,//CMC:IPM           // normal messages only
              Flags,
              NULL,            // Starting after message #
              &Count,               // Input/output message count
              0,                    // don't use UI
              &summary,             // Return message summary list.
              NULL);                // No extensions

  if (cmcSts != CMC_SUCCESS)    //cc:Mail is different from MS Mail!
   {
    cmcSts = CMCList(
              cmcSessionID,    // Session handle
              "CMC: IPM",           // normal messages only
              Flags,
              NULL,            // Starting after message #
              &Count,               // Input/output message count
              0,                    // don't use UI
              &summary,             // Return message summary list.
              NULL);                // No extensions
    }

  if (cmcSts != CMC_SUCCESS)
   { Error.itoa(cmcSts); return false; }

  if (!Count || !summary)
   { Error = ""; return false; }


 TEnvelope<CMCMailMessage::TAutoDelete> autofree(new CMCMailMessage::TAutoDelete);
 autofree->summary = summary;
 autofree->library = this;

 CMCMailMessage * msg;
  for (CMC_uint32 i = 0; i < Count; i++)
   {
    if (CB) if (!CB(v,i,Count,"Reading")) return false;

    msg = new CMCMailMessage(this,summary+i);
    msg->autofree = autofree;
    msgs.Add(msg);
   }

  if (CB) CB(v,Count,Count,"Reading");

  return true;
}

bool CMCMailLibrary::SendDocuments(const char * recip,  const char * file,
const char * subject,const char * textnote, bool quiet,MailCallback,void*)
//the same as the DoCMCSend in ml_ufill.cpp
{

if (flavor == email_GroupWise)
 {
  bool any=false;
  TStr Recip(recip);
  TStringList Recipients(Recip,",;");
  FOREACH(char*r,Recipients)
    if (!strcasecmp(r,"BCC")) continue;
    any |= MAPISendDocuments(r,file,subject,textnote,0,profile,password,quiet);
  DONEFOREACH
  return any;
 }

TStr Recip(recip),File(file),Subject(subject),Textnote(textnote);
TStringList Recipients(Recip,",;");

if (Recipients.Count() && !strcasecmp(Recipients[0],"BCC"))
{
 bool any = false;
 FOREACH(char*r, Recipients)
    if (!strcasecmp(r,"BCC")) continue;
    any |= SendDocuments(r,file,subject,textnote);
 DONEFOREACH
 return any; /* simulages BCC by sending several messages */
}

bool InvokeSendUI = (Recip[0]==0); //null recipient

CMC_attachment * MyAttachArray=0;
TStringList Attachments(File,",;");
uint32 attachcount = Attachments.Count();

if (attachcount)
{
 MyAttachArray = new CMC_attachment[attachcount];
 FOREACH(char * c, Attachments)
 CMC_attachment * attachment = MyAttachArray+i;
    attachment->attach_title = GetFilename(c);
    attachment->attach_type = CMC_ATT_OID_BINARY;
    attachment->attach_filename = (char*)c;
    attachment->attach_extensions = NULL;
 DONEFOREACH
 MyAttachArray[attachcount-1].attach_flags = CMC_ATT_LAST_ELEMENT;
}

CMC_recipient *MyReciparray = 0;
CMC_recipient *CMCReciparray=0;
uint32 recipcount = 0;

if (!InvokeSendUI)
{
 recipcount = Recipients.Count();
 MyReciparray = new CMC_recipient[recipcount];
 FOREACH(char * c, Recipients)

 CMC_recipient * recipient = MyReciparray+i;
   recipient->name = c;
   recipient->name_type = CMC_TYPE_UNKNOWN;
   recipient->address = NULL;
   recipient->role = CMC_ROLE_TO;
   recipient->recip_flags = NULL;
   recipient->recip_extensions = NULL;
 DONEFOREACH
 MyReciparray[recipcount-1].recip_flags = CMC_RECIP_LAST_ELEMENT;
}

CMC_return_code cmcsts;

if (recipcount)
 {
  cmcsts = CMCLookUp(
                     cmcSessionID,
                     MyReciparray,
                     CMC_LOOKUP_RESOLVE_UI|CMC_LOOKUP_RESOLVE_PREFIX_SEARCH,
                     // //CMC_LOGON_UI_ALLOWED|CMC_ERROR_UI_ALLOWED,
                     0,
                     &recipcount,
                     &CMCReciparray,
                     0
                     );

 if (cmcsts != CMC_SUCCESS && cmcsts != CMC_E_USER_CANCEL)
 {
  error = new xdb("CMC mail error","status",cmcsts);
  return false;
 }
}

CMC_message  message;
   message.message_reference=0;
//   if (flavor == email_GroupWise)
//    message.message_type = "BLT: IPM.NOTE.NGW.MAIL";//"BLT: IPM"; //
//    else
    message.message_type = "CMC: IPM";
   message.subject = Subject;
   memset(&message.time_sent,0,sizeof(message.time_sent));
   message.text_note = Textnote;
   message.recipients = CMCReciparray ? CMCReciparray : MyReciparray;
   message.attachments = MyAttachArray;
   message.message_flags = CMC_MSG_UNSENT;//0;// CMC_MSG_LAST_ELEMENT;
   message.message_extensions = 0;

 cmcsts = CMCSend(
          cmcSessionID,
          &message,
          (InvokeSendUI ? CMC_SEND_UI_REQUESTED : 0)|CMC_ERROR_UI_ALLOWED,
          0,
          0);

 if (MyReciparray) delete [] MyReciparray;
 if (CMCReciparray) CMCFree(CMCReciparray);

 if (cmcsts != CMC_SUCCESS && cmcsts != CMC_E_USER_CANCEL)
 {
  error = new xdb("CMC mail error","status",cmcsts);
 };

 return (cmcsts == CMC_SUCCESS);
}

/*************************************************************************

   FUNCTION:   deInitCMC(void)

   PURPOSE: unload the CMC.DLL

*************************************************************************/
CMC_return_code   CMCMailLibrary::doLogon(char * uid, char * pwd, char * popath)
 {
   if (cmcSessionID >  0L)  return CMC_SUCCESS;

//use the lotus implementation's header information
      CMC_X_COM_support cmcExtSupport[2];
      CMC_extension     cmcExt;

      cmcExtSupport[0].item_code = CMC_XS_COM;
      cmcExtSupport[0].flags = 0;
      cmcExtSupport[1].item_code = CMC_XS_MS;
      cmcExtSupport[1].flags = 0;

      cmcExt.item_code = CMC_X_COM_SUPPORT_EXT;
      cmcExt.item_data = 2;   //num of items supported in array
                              //pointed to by item_reference
      cmcExt.item_reference = cmcExtSupport;
      cmcExt.extension_flags = CMC_EXT_LAST_ELEMENT;
      if (uid) if (!*uid) uid = 0;
      if (pwd) if (!*pwd) pwd = 0;
      if (popath) if (!*popath) popath = 0;

      CMC_return_code ret;
      ret = CMCLogon(popath,      //path to message container
                         uid,            // user name
                         pwd,            // password
                         0,              // default char set
                         0,
                         CMC_VERSION,     // 100
                         CMC_LOGON_UI_ALLOWED | CMC_ERROR_UI_ALLOWED,
                         &cmcSessionID,   // returned sessionID
                         &cmcExt);      // no extensions
     return ret;
}

/*************************************************************************

   FUNCTION:   doLogoff()

   PURPOSE: call cmc_logoff to close the current cmc session

*************************************************************************/
CMC_return_code CMCMailLibrary::doLogoff()
{
   if (cmcSessionID > 0L)
   {
      return CMCLogoff(cmcSessionID,  /* session ID    */
                           0,       /* default UI ID */
                           0,                    /* no flags      */
                           0);                /* no extensions */
   }
   return CMC_SUCCESS;
}

#endif
