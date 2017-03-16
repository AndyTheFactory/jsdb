#include "rslib.h"
#pragma hdrstop
#ifndef NO_EXCHANGE_MAIL

#include <mapi.h>

// hack because of WIN32_LEAN_AND_MEAN=1 defined in tracemonkey
#ifdef XP_WIN
#undef _INC_COMMDLG
#include <Commdlg.h>
#endif

MAPIMailMessage::MAPIMailMessage(MAPIMailLibrary*lib, char id[512])
  :MailMessage()
{
 memcpy(msgid,id,512);
 Deleted=false;
 message = 0;
 library=lib;
}

MAPIMailMessage::~MAPIMailMessage()
 {
  if (message)
  {
   for (size_t i=0; i < message->nFileCount; i++)
   {
    if (!message->lpFiles[i].lpszPathName) continue;
    DeleteFile(message->lpFiles[i].lpszPathName);
    message->lpFiles[i].lpszPathName=0;
   }
  }

   if (!library) return;

   if (Deleted && *msgid)
     library->MAPIDeleteMail(library->sessionID,0,msgid,0,0);

  if (&library->MAPIFreeBuffer)
  if (message)
   library->MAPIFreeBuffer(message);

  library = 0;
 } //deallocates space

void MAPIMailMessage::LoadMessage()
 {
  if (message) return;
  if (!*msgid) return;
  if (!library) return;
  if (!&library->MAPIReadMail) return;

  message = 0;

  library->MAPIReadMail(library->sessionID,0,
      msgid,MAPI_PEEK,
      0,&message);
 }


bool MAPIMailMessage::IsRead()
 {
  if (!message) LoadMessage();
  if (!message) return false;
  return !(message->flFlags & MAPI_UNREAD);
 }
 
char * MAPIMailMessage::Sender()
 {
   if (!message) LoadMessage();
   if (!message) return "";
   if (!message->lpOriginator) return "";
   return message->lpOriginator->lpszAddress;
 }

void MAPIMailMessage::Time(SYSTEMTIME& tm)
 {
   if (!message) LoadMessage();

   memset(&tm,0,sizeof(tm));
   if (!message) return;
   char *dt = message->lpOriginator->lpszAddress;  // YYYY/MM/DD HH:MM
   tm.wYear = uint16(atoi(dt));
   if (strlen(dt) > 5) tm.wMonth = uint16(atoi(dt+5));
   tm.wDayOfWeek =0;
   if (strlen(dt) > 8) tm.wDay = uint16(atoi(dt+8));
   if (strlen(dt) > 11) tm.wHour = uint16(atoi(dt+11));
   if (strlen(dt) > 14) tm.wMinute = uint16(atoi(dt+14));
   if (strlen(dt) > 17) tm.wSecond = uint16(atoi(dt+17));
   tm.wMilliseconds =0;
 }

int32 MAPIMailMessage::Size()
 {
   if (!message) LoadMessage();
   if (!message) return 0;
   if (!message->lpszNoteText) return 0;
   return (strlen(message->lpszNoteText));
 }

char * MAPIMailMessage::Subject()
 {
   if (!message) LoadMessage();
   if (!message) return "";
   if (!message->lpszSubject) return "";
   return message->lpszSubject;
 }

char * MAPIMailMessage::MsgText()
 {
   if (!message) LoadMessage();
   if (!message) return "";
   if (!message->lpszNoteText) return "";
   return message->lpszNoteText;
 }

int MAPIMailMessage::AttachCount()
 {
  if (!message) LoadMessage();
  if (!message) return 0;
  return message->nFileCount;
 }

bool MAPIMailMessage::GetAttachment(size_t i, TStr& title,TStr& file,bool)
 {
  title=0;
  file=0;
  if ( i == NOT_FOUND ) return false;

  if (!message) LoadMessage();
  if (!message) return false;
  if (i >= message->nFileCount) return false;
  

  file = message->lpFiles[i].lpszPathName;
  title = message->lpFiles[i].lpszFileName;
  
  if (!*file) return false;
  if (!*title) title=file;

  return true;
 }


//------------------------------
MAPIMailLibrary::MAPIMailLibrary(bool Login,
         const char* uid, const char* pwd, const char* service):
   //may throw an exception on failure to log in
  MailLibrary(),
  dllhandle(0) , sessionID(0),
  name(uid), password(pwd), profile(service)
 {

 dllhandle = LoadLibrary("MAPI32.DLL");

 initMAPI();

 if (!dllhandle)
 {
  error = new xdb("Unable to load the mail library","file","MAPI32.DLL");
  return;
 }

// MAPIINIT_0 MAPIINIT= { 0, MAPI_MULTITHREAD_NOTIFICATIONS};
// if (MAPIInitialize)
//  MAPIInitialize (&MAPIINIT);

   if (Login && &MAPILogon)
   {
     uint32 mapists = 88;
     if (*profile) mapists  = MAPILogon(0,(char*)profile,(char*)password,MAPI_NEW_SESSION,0,&sessionID);
     if (mapists != SUCCESS_SUCCESS && *name)
      mapists = MAPILogon(0,(char*)name,(char*)password,MAPI_NEW_SESSION,0,&sessionID);
     if (mapists != SUCCESS_SUCCESS)
      mapists = MAPILogon(0,0,0,MAPI_LOGON_UI|MAPI_NEW_SESSION,0,&sessionID);

    if (mapists != SUCCESS_SUCCESS)
      {
         error = new xdb("Log on failed","name",name,"profile",profile);
         return;
      }
   }
 }

MAPIMailLibrary::~MAPIMailLibrary()
 {
  if (sessionID)
   if (&MAPILogoff)
    MAPILogoff(sessionID,0,0,0);

  if (dllhandle) FreeLibrary(dllhandle);
 }

bool MAPIMailLibrary::initMAPI()
{
#ifdef __BORLANDC__
#define DOLoadFunc(type,name) name = (type*)GetProcAddress(dllhandle,#name)
#else
#define DOLoadFunc(type,name) name = (LP##type)GetProcAddress(dllhandle,#name)
#endif
//DOLoadFunc(MAPIINIT,MAPIInit);
//DOLoadFunc(MAPIUNINITIALIZE,MAPIUninitialize);
//DOLoadFunc(MAPILOGONEX,MAPILogonEx);
//DOLoadFunc(MAPIALLOCATEBUFFER  , MAPIAllocateBuffer);
//DOLoadFunc(MAPIALLOCATEMORE , MAPIAllocateMore);
DOLoadFunc(MAPIFREEBUFFER  , MAPIFreeBuffer);
//DOLoadFunc(MAPIADMINPROFILES , MAPIAdminProfiles);
DOLoadFunc(MAPIRESOLVENAME , MAPIResolveName);
DOLoadFunc(MAPISENDMAIL  , MAPISendMail);
DOLoadFunc(MAPILOGON   , MAPILogon);
DOLoadFunc(MAPILOGOFF   ,MAPILogoff);
DOLoadFunc(MAPIREADMAIL   , MAPIReadMail);
DOLoadFunc(MAPIFINDNEXT   , MAPIFindNext);
DOLoadFunc(MAPIDELETEMAIL   , MAPIDeleteMail);

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

bool MAPIMailLibrary::GetMessages(TRow<MailMessage> &msgs,bool UnreadOnly,TStr& Error,
MailCallback CB,void*v)
{
  unsigned ret;
  char msgid[512],msgid2[512];;
  msgid[0]=0;
  
  size_t count=0;

  MAPIMailMessage * msg;
  if (CB) if (!CB(v,0,0,"Reading")) return false;

  while (1)
  {
   strcpy(msgid2,msgid);
   uint32 flags = MAPI_LONG_MSGID;
   if (UnreadOnly) flags |= MAPI_UNREAD_ONLY;
   ret = MAPIFindNext(sessionID, NULL, NULL, msgid2, flags ,0, msgid);
   if (CB) if (!CB(v,count,0,0)) return false;

   if (ret == MAPI_E_NO_MESSAGES )
    break; // out of messages

   if (ret != SUCCESS_SUCCESS)
   { Error.itoa(ret); break; }

   count++;
   msg = new MAPIMailMessage(this,msgid);
   msgs.Add(msg);
  }

  if (CB) CB(v,count,count,"Done");

  return (ret == MAPI_E_NO_MESSAGES);
}

bool MAPIMailLibrary::SendDocuments(const char * recip,  const char * file,
const char * subject,const char * textnote, bool quiet,MailCallback,void*)
//the same as the DoCMCSend in ml_ufill.cpp
{
 uint32 mapists;
 if (!&MAPISendMail) return false;
 if (!&MAPIResolveName) return false;

TAPointer<MapiFileDesc> FileDesc(0);
TAPointer<MapiRecipDesc> addresses(0);

TStringList FL(file?file:"",",;");
TStringList Titles;
size_t FileCount = FL.Count();
if (FileCount)
{
 FileDesc = new MapiFileDesc[FileCount];
 TStr FileTitle(MAXPATH);
 FOREACH(char * c,FL)
  GetFileTitle(c, FileTitle, MAXPATH);
  FileDesc[i].ulReserved   = 0;
  FileDesc[i].flFlags      = 0;
  FileDesc[i].nPosition    = -1;
  FileDesc[i].lpszPathName = (char*) c;
  FileDesc[i].lpszFileName = (char*) Titles[Titles.Add(FileTitle)];
  FileDesc[i].lpFileType  = 0;
 DONEFOREACH
}

TStringList Recipients(recip?recip:"",",;");
TStringList ERRnames;

TList<MapiRecipDesc> recipients(false);

MapiRecipDesc * RecipDesc = 0;
int flags = MAPI_TO;

FOREACH(char*c,Recipients)
    if (!strcasecmp(c,"BCC")) { flags = MAPI_BCC; continue;}
    if (!strcasecmp(c,"CC")) {flags = MAPI_CC; continue;}

   mapists = MAPIResolveName(
          sessionID,
          0UL,
          (char*) c,
          quiet?0:(MAPI_DIALOG|MAPI_LOGON_UI),
          0UL,
          &RecipDesc);

   if (mapists == SUCCESS_SUCCESS && RecipDesc)
   {
    RecipDesc->ulRecipClass = flags;
    recipients.Add(RecipDesc);
    RecipDesc = 0;
   }
DONEFOREACH

addresses = new MapiRecipDesc[recipients.Count()];
FOREACH(MapiRecipDesc* r, recipients)
 memcpy(&addresses[i],r,sizeof(MapiRecipDesc));
DONEFOREACH


MapiMessage Message;
 Message.ulReserved        = 0;
 Message.lpszSubject       = (char*) subject;
 Message.lpszNoteText      = (char*) textnote;
 Message.lpszMessageType   = 0; // "CMC:IPM"
 Message.lpszDateReceived  = 0;
 Message.lpszConversationID= 0;
 Message.flFlags           = 0;
 Message.lpOriginator      = 0;
 Message.nRecipCount       = recipients.Count();
 Message.lpRecips          = Message.nRecipCount ? (MapiRecipDesc*)addresses : (MapiRecipDesc*)0;
 Message.nFileCount        = FileCount;
 Message.lpFiles           = FileCount ? (MapiFileDesc*)FileDesc : (MapiFileDesc*)0;

 mapists = MAPISendMail(sessionID,
                          0,
                          &Message,
                          quiet?0:(MAPI_DIALOG|MAPI_LOGON_UI),
                          0);

if (&MAPIFreeBuffer)
FOREACH(MapiRecipDesc* r, recipients)
 MAPIFreeBuffer(r);
DONEFOREACH
return (mapists == SUCCESS_SUCCESS);
}


#endif
