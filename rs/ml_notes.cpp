#include "rslib.h"
#pragma hdrstop

#ifndef NO_NOTES_MAIL

#include "notes.h"
#define LIBNAME "NNOTES.DLL"

extern HINSTANCE RSLoadLibrary(const char * path); //ml_cmc.cpp

NSFMailLibrary::NSFMailLibrary(const char* servername,const char * dbname,
   					 const char* name, const char* password):
 MailLibrary(),
 ServerName(servername),
 MailFile(dbname),
 MailFilePath(MAXPATH),
 UserName(name),
 hNSFLibrary(NULL)
{
  Notes = new TNotesLib;

  TStr Sender(MAXPATH);
  GetPrivateProfileString("Notes","Location","",Sender,MAXPATH,"Notes.ini");
  if (*Sender) {
   char * c = strchr(Sender,',');
   if (c) Sender=c+1;
  } //else load default sender from POP config?

  if (!*ServerName)
   {
    ServerName.Resize(MAXPATH);
    GetPrivateProfileString("Notes","MailServer","",ServerName,MAXPATH,"Notes.ini");
   }
  //if (!*ServerName) throw XMSG("No mail server found");

//  Feedthrough = (TPOPMailSystem*)OpenMailSystem(email_POP3,false,0,0);

  TStr NotesDir(MAXPATH);

  if (!*UserName)
   {
    GetPrivateProfileString("POSTMASTER","NotesUser","",UserName,MAXPATH,"raosoft.ini");
    if (!*UserName) GetPrivateProfileString("Notes","USERNAME","",UserName,MAXPATH,"Notes.ini");
   }

  if (!*MailFile)
   {
    if (*UserName)
     {
      if (!strchr(UserName,'\\') && !strchr(UserName,'/')) MailFile = "mail\\";
      MailFile += UserName;
     }
   }

  if (!*MailFile)
  {
   error = new xdb("Unable to open Notes mail. Please specify a server.");
   return;
  }

  TStr s(LIBNAME);
  if (RegGetKey(".nsf\\Shell\\Open\\Command",0, NotesDir,HKEY_CLASSES_ROOT))
   {
    GetDirectory(NotesDir,s);
    AddBackslash(s);
    s += LIBNAME;
    hNSFLibrary = RSLoadLibrary(s);
   }
  else hNSFLibrary = LoadLibrary(LIBNAME);

  if (hNSFLibrary == NULL) return; //use passthrough

  Notes->Init(hNSFLibrary);

  char * c[2];
  c[0]=s;
  c[1]=0;

  if (*Notes->NotesInitExtended == NULL)
  {
   FreeLibrary(hNSFLibrary);
   hNSFLibrary=0;
   return;
  }

  Notes->NotesInitExtended(1,c);
  Notes->OSPathNetConstruct( NULL, ServerName, MailFile, MailFilePath);

};

NSFMailLibrary::~NSFMailLibrary()
{
// if (Feedthrough) delete Feedthrough;
 if (hNSFLibrary)
   if (*Notes->NotesTerm)
     Notes->NotesTerm();
 if (hNSFLibrary) FreeLibrary(hNSFLibrary);
 delete Notes;
};

bool NSFMailLibrary::SendDocuments(const char * recip, const char * file,
                      const char * subject, const char * textnote, bool quiet,
                              MailCallback CB,void*v)
 {
  TStr Error;
  if (hNSFLibrary)
    if (DoSendDocuments((char*)recip,(char*)file,(char*)subject,
      (char*)textnote,Error))
      return true;
  if (*Error) error = new xdb("Notes Error",Error);
//  if (*Error && !quiet) dspMessageAlert(Error,"Notes Error");
//  if (Feedthrough)
//     return Feedthrough->SendDocuments(recip,file,subject,textnote,CB,v);
  return false;
 }

bool NSFMailLibrary::GetMessages(TList<MailMessage> &msgs,bool UnreadOnly,TStr &Error,
MailCallback CB,void*v)
 {
  bool r = hNSFLibrary ? DoGetMessages(msgs,UnreadOnly,Error,CB,v) : false;
//  if (!r && Feedthrough)
//     return Feedthrough->GetMessages(msgs,UnreadOnly,Error,CB,v);
  return r;
 }


#define HandleError char temp[25]; sprintf(temp,"\nErr=%d",error)


bool NSFMailLibrary::DoSendDocuments(char * recip, char * file,
                      char * subject, char * textnote,TStr&Error)
{
  DBHANDLE    hMailBox;
  NOTEHANDLE  hMemo;
  TIMEDATE    tdCurrent;

  TStr MailBoxPath(MAXPATH);
  int error;

  if (hNSFLibrary == NULL) return false;
  Notes->OSPathNetConstruct( NULL,ServerName,"MAIL.BOX",MailBoxPath);

  if (error = Notes->NSFDbOpen (MailBoxPath, &hMailBox))
    {
        HandleError;
        Error = TStr("Unable to open ", MailBoxPath);
        return false;
    }

 Notes->OSCurrentTIMEDATE(&tdCurrent);

 if (error = Notes->NSFNoteCreate(hMailBox, &hMemo))
    {   HandleError;
        Error = TStr("Unable to create memo in MAIL.BOX ",temp);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

 if (error = Notes->NSFItemSetText( hMemo,
                             FIELD_FORM,     /* "Form" */
                             MAIL_MEMO_FORM, /* "Memo" = Standard memo */
                             MAXWORD))
    {   Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }


TStringList Recipients(recip,",;");
if (Recipients.Count() > 1)
 {
   Notes->NSFNoteClose(hMemo);
   Notes->NSFDbClose(hMailBox);
   return false;   // for now, single recipient only
 }
else
 {
   if (error = Notes->NSFItemSetText( hMemo,
                                MAIL_SENDTO_ITEM,  /* "SendTo" */
                                (char*)recip,
                                MAXWORD))
    {   Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

  if (error = Notes->NSFItemSetText( hMemo, /* use NSFItemCreateTextList if > 1*/
                                MAIL_RECIPIENTS_ITEM,   /* "Recipients" */
                                (char*)recip,
                                MAXWORD))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }
 }

   if (error = Notes->NSFItemSetText( hMemo,
                                MAIL_FROM_ITEM,     /* "From" */
                                (char*)UserName,
                                MAXWORD))
    {
        Error = "Your Notes user name is incorrect";
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->NSFItemSetText( hMemo,
                                MAIL_SUBJECT_ITEM,     /* "Subject" */
                                (char*)subject,
                                MAXWORD))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->NSFItemSetText( hMemo,
                                MAIL_DELIVERYPRIORITY_ITEM, /* "DeliveryPriority" */
                                "Normal",
                                MAXWORD))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->NSFItemSetText( hMemo,
                                MAIL_DELIVERYREPORT_ITEM, /* "DeliveryReport" */
                                "Basic",
                                MAXWORD))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->NSFItemSetText( hMemo,
                                MAIL_RETURNRECEIPT_ITEM,  /* "ReturnReceipt" */
                                "No",
                                MAXWORD))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->NSFItemSetTime( hMemo,
                                MAIL_COMPOSEDDATE_ITEM, /* "ComposedDate" */
                                &tdCurrent))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->NSFItemSetTime( hMemo,
                                MAIL_POSTEDDATE_ITEM, /* "PostedDate" */
                                &tdCurrent))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }
MemoryStream temp;

  if (file && *file)
   {
   try {
    FileStream In(file,Stream::OMBinary,Stream::ReadOnly);

     temp << textnote;
     temp << "\r\n\r\nMIME_ENCODED_FILE: ";
     temp << GetFilename(file);
     temp << "\r\n\r\n";
     b64encode(In,temp);
     textnote = temp;
    }
    catch(...)
    {
     Error = TStr("File attachment failed: ", file);
    }
   }

  if (error = Notes->NSFItemSetText( hMemo,
                                "TempBody",
                                textnote,
                                MAXWORD))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    BLOCKID     bidItem;
    WORD        wDataType;
    BLOCKID     bidValue;
    DWORD       dwValueLen;
    HANDLE      hRichTextBody;
    DWORD       dwRichTextLen;

    if (error = Notes->NSFItemInfo(hMemo, "TempBody",strlen("TempBody"),
                            &bidItem, &wDataType, &bidValue, &dwValueLen))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->ConvertItemToComposite(bidValue, dwValueLen,
                DEFAULT_FONT_ID, "\r\n", PARADELIM_BLANKLINE,
                &hRichTextBody, &dwRichTextLen,
                TRUE, NULL, 0, FALSE))   //FALSE,NULL,0,FALSE
    {
        Error = "Notes cannot create messages longer than 60k";
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    char* RichTextBody = (char*)Notes->OSLockObject(hRichTextBody) ;
    RichTextBody += sizeof(WORD);
    dwRichTextLen -= sizeof(WORD);

    if (error = Notes->NSFItemAppend(hMemo, 0, MAIL_BODY_ITEM,
                    strlen(MAIL_BODY_ITEM), TYPE_COMPOSITE, RichTextBody,
                    dwRichTextLen))
    {
        Notes->OSUnlockObject(hRichTextBody);
        Notes->OSMemFree(hRichTextBody);
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    Notes->OSUnlockObject(hRichTextBody) ;
    Notes->OSMemFree(hRichTextBody);

    if (error = Notes->NSFItemDelete(hMemo, "TempBody",
                            strlen("TempBody")))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->NSFNoteUpdate(hMemo, 0))
    {
        Notes->NSFNoteClose(hMemo);
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->NSFNoteClose(hMemo))
    {
        Notes->NSFDbClose(hMailBox);
        return false;
    }

    if (error = Notes->NSFDbClose(hMailBox))
    {
        return false;
    }

 return true;
};

bool NSFMailLibrary::DoGetMessages(TList<MailMessage>&msgs,bool UnreadOnly,TStr &Error,
MailCallback CB,void*v)
{
  int error;
  HANDLE hMessageFile=0;
  HANDLE hMessageList=0;
  DARRAY * MessageList=0;
  WORD MessageCount=0, Msg;

  if (hNSFLibrary == NULL) return false; //use passthrough

  //message file closed by the last destructor!
    if (error = Notes->MailOpenMessageFile(MailFilePath, &hMessageFile))
    {   HandleError;
        Error = TStr("Unable to open ",MailFilePath,temp);
        return false;
    }

    /* Create message list of messages in the file - just 64K */

    if (error = Notes->MailCreateMessageList(hMessageFile,
                        &hMessageList, &MessageList, &MessageCount))
    {   HandleError;
        Error = TStr( "Unable to create message list",temp);
        Notes->MailCloseMessageFile(hMessageFile);
        return false;
    }

   TEnvelope<NSFMailMessage::TAutoDelete> autofree(new NSFMailMessage::TAutoDelete);

   autofree->hMessageFile = hMessageFile;
   autofree->hMessageList = hMessageList;
   autofree->library = this;

   for (Msg = 0; Msg < MessageCount; Msg++)
    {
     if (CB) if (!CB(v,Msg,MessageCount,"Reading")) return false;

     NSFMailMessage * n = new NSFMailMessage(this,Msg);
     n -> MessageList  = (void*)MessageList;
/*     n -> AutoFree     = (Msg == 0);
     n -> hMessageList = hMessageList;
     n -> hMessageFile = hMessageFile;
*/
     n->autofree = autofree;
     msgs.Add(n);
    }

  if (CB) CB(v,MessageCount,MessageCount,"Done");

 return true;
}

NSFMailMessage::NSFMailMessage(NSFMailLibrary* lib, int num):
  library(lib),
  MailMessage(),
  _Sender(MAXPATH),
  _Subject(MAXPATH),
  autofree(0)
 {
//  AutoFree     = false;
  Msg          = num;
  hMessage     = 0;
//  hMessageList = 0;
//  hMessageFile = 0;
  MessageList  = 0;
  msgsize      = 0;
 };

NSFMailMessage::TAutoDelete::TAutoDelete() {}

NSFMailMessage::TAutoDelete::~TAutoDelete()
 {
   if (hMessageList)
    {
     library->Notes->OSUnlockObject(hMessageList);
     library->Notes->OSMemFree(hMessageList);
    }

   if (hMessageFile)
     library->Notes->MailCloseMessageFile(hMessageFile);
 }

NSFMailMessage::~NSFMailMessage()
{

 if (hMessage) library->Notes->MailCloseMessage(hMessage);
 if (Deleted) // Try to delete -- may or may not work.
		library->Notes->MailPurgeMessage((DARRAY*)MessageList, Msg);

}; //deallocates space

void NSFMailMessage::LoadMessage()
 {
  if (hMessage == 0) library->Notes->MailOpenMessage((DARRAY*)MessageList, Msg, &hMessage);
 };

char * NSFMailMessage::Sender()
 {
  WORD end = 0;
  if (!hMessage) LoadMessage();
// alternate:
  library->Notes->MailGetMessageOriginator((DARRAY*)MessageList, Msg, _Sender, MAXPATH, &end);
  library->Notes->MailGetMessageItem(hMessage, MAIL_FROM_ITEM_NUM, _Sender,
                                        MAXPATH, &end);
  _Sender[end]=0;
  return _Sender;
 };

char * NSFMailMessage::Subject()
{
  WORD end = 0;
  if (!hMessage) LoadMessage();
  library->Notes->MailGetMessageItem (hMessage, MAIL_SUBJECT_ITEM_NUM, _Subject,
                                         MAXPATH, &end);
  _Subject[end]=0;
  return _Subject;
}

void NSFMailMessage::Time(SYSTEMTIME& tm)
{
  if (!hMessage) LoadMessage();
  TIME    Time;
  library->Notes->MailGetMessageItemTimeDate(hMessage, MAIL_POSTEDDATE_ITEM_NUM, &Time.GM);
  library->Notes->TimeGMToLocal(&Time);
  tm.wYear = Time.year;
  tm.wMonth = Time.month;
  tm.wDayOfWeek = Time.weekday-1;
  tm.wDay  = Time.day;
  tm.wHour  = Time.hour;
  tm.wMinute  = Time.minute;
  tm.wSecond = Time.second;
  tm.wMilliseconds = Time.hundredth * 10;
}

int32 NSFMailMessage::Size()
{
 return msgsize;
}
char * NSFMailMessage::MsgText()
{
  if (!hMessage) LoadMessage();
  if (*Text) return Text;

  Text = "Message text unavailable";
  TStr s;
  if (!CreateTempFile(s,"TXT")) return Text;

  library->Notes->MailGetMessageBodyText(hMessage,NULL,"\r\n",80,TRUE,s,&msgsize);

  try{
  Text.Resize(msgsize);
  FileStream(s,Stream::OMBinary,Stream::ReadOnly).read(Text,msgsize);
  }catch(...) {}
  return Text;
}

bool NSFMailMessage::IsRead()
  {return false;}

bool NSFMailMessage::GetAttachment(size_t i, TStr& title,TStr& file,bool getfile)
  {
   if (!hMessage) LoadMessage();
//   title.Resize(MAXPATH);
   char * c = MsgText();
   char * s;
   s = strstr(c,"MIME_ENCODED_FILE: ");
   if (s)
    {
     s += 19;
     title = TStr(s,strcspn(s,"\r\n\t"));
     if (!getfile)  return true;

     if (!CreateTempFile(file,GetExtension(title))) return false;
     try {
     s += strlen(title);

     return
     b64decode(ByteStream(s),
               FileStream(file,
                           Stream::OMBinary,
                           Stream::WriteOnly));
     } catch(...) {return false;}

    }

   BLOCKID     bhAttachment;
   if (!getfile)
   {
     return library->Notes->MailGetMessageAttachmentInfo(hMessage, i,
                                  &bhAttachment, title,
                                  NULL, NULL, NULL, NULL, NULL);
   }
   else
   {
        title.Resize(MAXPATH);
     BLOCKID     bhAttachment;
   if (!library->Notes->MailGetMessageAttachmentInfo(hMessage, i,
                                  &bhAttachment, title,
                                  NULL, NULL, NULL, NULL, NULL)) return false;
   if (!CreateTempFile(file,GetExtension(title))) return false;

   library->Notes->MailExtractMessageAttachment(hMessage,&bhAttachment,file);
    //? is bhAttachement direct or a pointer? Try it and find out.
   return true;
   }
  }

int NSFMailMessage::AttachCount()
 {
 if (!hMessage) LoadMessage();

 const char * c = MsgText();
 if (strstr(c,"MIME_ENCODED_FILE:")) return 1;

 //don't look for the notes attachments
 WORD Att=0;
 BLOCKID  b;
 while (library->Notes->MailGetMessageAttachmentInfo(hMessage, Att, &b,0,0,0,0,0,0))
    Att++;
 return Att;
 };

/*****************************************************************************/

#ifdef Sorry_This_Doesnt_Work
bool NotesSendDocuments(const char * recip,   const char * file,
                        const char * subject, const char * textnote)
{
 TStr szSendTo(recip);
 TStr szSubject(subject);
 TStr szBody(textnote);
    char        szMailServerName[MAXUSERNAME+1];
    char        szMailFileName[MAXUSERNAME+1];
    char        szMailFilePath[MAXPATH+1];
    char        szMailBoxPath[MAXPATH+1];
    char*      error=0;// = NOERROR;    /* returned ERR_SENDMAIL_... value */
    DBHANDLE    hMailFile;
    DBHANDLE    hMailBox = NULLHANDLE;
    BOOL        fLocal = FALSE;
    NOTEHANDLE  hMsg;
    HANDLE      hRecipientsList = NULLHANDLE;
    HANDLE      hSendToList = NULLHANDLE;
    HANDLE      hCopyToList = NULLHANDLE;
    WORD        wRecipientsSize, wSendToSize, wCopyToSize;
    WORD        wRecipientsCount, wSendToCount, wCopyToCount;
    LIST       *plistRecipients, *plistSendTo, *plistCopyTo;
    char       *szNextName;        /* used when parsing szSendTo, etc. */
    char        szPriority[] = "N"; /* "Normal" */
    char        szReport[] = "B";   /* "Basic" */
    HANDLE      hBodyItem = NULLHANDLE;
    DWORD       dwBodyItemLen;
    DBHANDLE    hOrigDB;
    NOTEID      OrigNoteID;
    OID         OrigNoteOID, NewNoteOID;

    if (!OSGetEnvironmentString(MAIL_MAILSERVER_ITEM, /*"MailServer"*/
             szMailServerName, MAXUSERNAME))
    {   /* Unable to get mail server name */
   error = "ERR_SENDMAIL_SERVERNAME";
   goto Exit;
    }

    if (!OSGetEnvironmentString(MAIL_MAILFILE_ITEM, /*"MailFile"*/
             szMailFileName, MAXUSERNAME))
    {   /* Unable to get mail file name */
   error = "ERR_SENDMAIL_MAILFILENAME";
   goto Exit;
    }

    /*  First try to open the user's mail file on the mail server.
   If unable to open because LAN not available, then ask user if
   they would like to save locally.
     */
    OSPathNetConstruct( NULL, szMailServerName, szMailFileName,
         szMailFilePath);

    status = MailOpenMessageFile(szMailFilePath, &hMailFile);

    if ( (ERR(status) == ERR_NO_NETBIOS) ||
    (ERR(status) == ERR_SERVER_NOT_RESPONDING) )
     /* laptop not connected to server */
    {
   /* Ask user if they would like to deliver msg to local MAIL.BOX */
   status = NOERROR;
        /* Unable to reach Mail Server. Save in local MAIL.BOX? */
   if (IDOK == MessageBox (GetFocus(),
      BuildErrStr(MSG_SENDMAIL_ASKUSELOCAL),
      szMessageBoxTitle, MB_ICONQUESTION | MB_OKCANCEL))
   {
       /* yes - create and save message in local mail file */
       fLocal = TRUE;
       status = MailOpenMessageFile(szMailFileName, &hMailFile);
   }
   else
   {   /* no - user wants to connect to server. Error condition.*/
       status = ERR_SERVER_NOT_RESPONDING;
   }
    }
    if (status)
    {   /* Unable to open user's Notes mail file */
   error = "ERR_SENDMAIL_CANTOPENMAILFILE";
   goto Exit;
    }

    if (status = Notes->MailCreateMessage(hMailFile, &hMsg))
    {   /* Unable to create memo in mail file */
   error = "ERR_SENDMAIL_CANTCREATEMEMO";
   goto CloseFile;
    }

    if (status = ListAllocate(0, 0, TRUE, &hRecipientsList,
      &plistRecipients, &wRecipientsSize))
    {
   /* Unable to allocate list */
   error = "ERR_SENDMAIL_CANTALLOCATELIST";
   goto CloseMsg;
    }
    OSUnlockObject(hRecipientsList);

    if (status = ListAllocate(0, 0, TRUE, &hSendToList,
      &plistSendTo, &wSendToSize))
    {
   error = "ERR_SENDMAIL_CANTALLOCATELIST";
   goto CloseMsg;
    }
    OSUnlockObject(hSendToList);

    if (status = ListAllocate(0, 0, TRUE, &hCopyToList,
      &plistCopyTo, &wCopyToSize))
    {
    error = "ERR_SENDMAIL_CANTALLOCATELIST";
    goto CloseMsg;
    }
    OSUnlockObject(hCopyToList);

    /* check here to see if a name has been entered	in SendTo field */
    if ((WORD)lstrlen(szSendTo) < SEND_TO_NAME)
   	{
      error = "ERR_SENDMAIL_NORECIPIENTS";
      goto CloseMsg;
   	}

    /* Parse SendTo string. Add names to SendTo and Recipients lists. */

    for (szNextName = strtok(szSendTo, ":,");
    szNextName != (char*)NULL;
    szNextName = strtok(NULL, ":,"))
    {
   while (isspace(*szNextName))    // Skip white space before name
       szNextName++;

   if (status = ListAddEntry(hSendToList, TRUE, &wSendToSize,
            wSendToCount++, szNextName,
            (WORD)lstrlen(szNextName)))
   {   /* Unable to add name to SendTo list */
       error = "ERR_SENDMAIL_CANTADDTOSENDLIST";
       goto CloseMsg;
   }

   if (status = ListAddEntry(hRecipientsList, TRUE, &wRecipientsSize,
            wRecipientsCount++, szNextName,
            (WORD)lstrlen(szNextName)))
   {   /* Unable to add name to Recipients list */
       error = "ERR_SENDMAIL_CANTADDTORECIPLIST";
       goto CloseMsg;
   }
    }

    /* Parse CopyTo string. Add names to CopyTo and Recipients list. */

    for (szNextName = strtok(szCopyTo, ":,");
    szNextName != (char*)NULL;
    szNextName = strtok(NULL, ":,"))
    {
   while (isspace(*szNextName))
       szNextName++;

   if (status = ListAddEntry(hCopyToList, TRUE, &wCopyToSize,
            wCopyToCount++, szNextName,
            (WORD)lstrlen(szNextName)))
   {   /* Unable to add name to CopyTo list */
       error = "ERR_SENDMAIL_CANTADDTOCOPYLIST";
       goto CloseMsg;
   }

   if (status = ListAddEntry(hRecipientsList, TRUE, &wRecipientsSize,
            wRecipientsCount++, szNextName,
            (WORD)lstrlen(szNextName)))
   {
       error = "ERR_SENDMAIL_CANTADDTORECIPLIST";
       goto CloseMsg;
   }
    }

    /*  Suggested enhancements: You might want to add code here to verify
   that the name & address book on the mail server contains person or
   group documents for each of the named recipients. See Notes API
   funciton NAMELookup(). Possibly query user to resolve unknown
   recipient names. You might also want to check the recipients list
   to ensure it contains no duplicate names.
     */

    /* Add the Recipients item to the message. */

    if (wRecipientsCount == 0)  /* Mail memo has no recipients. */
    {
   error = "ERR_SENDMAIL_NORECIPIENTS";
   goto CloseMsg;
    }

    if (status = MailAddRecipientsItem( hMsg, hRecipientsList,
               wRecipientsSize))
    {
   /* Unable to set Recipient item in memo */
   error = "ERR_SENDMAIL_CANTSETRECIPIENT";
   goto CloseMsg;
    }
    /*  MailAddRecipientsItem and MailAddHeaderItemByHandle both attach
   the memory used by the list to the message. Set handle to NULL
   after these functions succeed so the code at CloseMsg: below does
   not attempt to free it.
     */
    hRecipientsList = NULLHANDLE;

    /* Add the SendTo and CopyTo items to the message. */

    if (status = MailAddHeaderItemByHandle( hMsg, MAIL_SENDTO_ITEM_NUM,
                   hSendToList, wSendToSize, 0))
    {
   /* Unable to set SendTo item in memo */
   error = "ERR_SENDMAIL_CANTSETSENDTO";
   goto CloseMsg;
    }
    hSendToList = NULLHANDLE;

    if (status = MailAddHeaderItemByHandle( hMsg, MAIL_COPYTO_ITEM_NUM,
                   hCopyToList, wCopyToSize, 0))
    {   /* Unable to set CopyTo item in memo */
   error = "ERR_SENDMAIL_CANTSETCOPYTO";
   goto CloseMsg;
    }
    hCopyToList = NULLHANDLE;

    /* Add the Form item to the message */
    if (status = MailAddHeaderItem( hMsg, MAIL_FORM_ITEM_NUM,
                MAIL_MEMO_FORM,
                (WORD)lstrlen(MAIL_MEMO_FORM)))
    {   /* Unable to set Form item in memo  */
   error = "ERR_SENDMAIL_CANTSETFORM";
   goto CloseMsg;
    }

    /* Add From, Subject, Delivery Priority, & etc. items to the message */
    if (status = MailAddHeaderItem( hMsg, MAIL_FROM_ITEM_NUM,
                szFrom, (WORD)lstrlen(szFrom)))
    {   /* Unable to set From item in memo */
   error = "ERR_SENDMAIL_CANTSETFROM";
   goto CloseMsg;
    }

    if (status = MailAddHeaderItem( hMsg, MAIL_SUBJECT_ITEM_NUM,
                szSubject, (WORD)lstrlen(szSubject)))
    {   /* Unable to set Subject item in memo */
   error = "ERR_SENDMAIL_CANTSETSUBJECT";
   goto CloseMsg;
    }

    if (status = MailAddHeaderItem( hMsg, MAIL_DELIVERYPRIORITY_ITEM_NUM,
               szPriority, (WORD)lstrlen(szPriority)))
    {   /* Unable to set Delivery Priority item in memo */
   error = "ERR_SENDMAIL_CANTSETPRIORITY";
   goto CloseMsg;
    }

    if (status = MailAddHeaderItem( hMsg, MAIL_DELIVERYREPORT_ITEM_NUM,
               szReport, (WORD)lstrlen(szReport)))
    {   /* Unable to set Delivery Report into memo */
   error = "ERR_SENDMAIL_CANTSETREPT";
   goto CloseMsg;
    }

    /*  set "ComposedDate" to tdDate = when dialog box was initialized */
    if (status = MailAddHeaderItem( hMsg, MAIL_COMPOSEDDATE_ITEM_NUM,
                (BYTE*)(&tdDate),
                (WORD)sizeof(TIMEDATE)))
    {   /* Unable to set Composed Date in memo */
   error = "ERR_SENDMAIL_CANTSETCOMPDATE";
   goto CloseMsg;
    }

    if (status = MailCreateBodyItem (&hBodyItem, &dwBodyItemLen))
    {   /* Unable to create body item in message */
      error = "ERR_SENDMAIL_CANTCREATEBODY";
      hBodyItem = NULLHANDLE;
      goto CloseMsg;
    }

    if (status = MailAppendBodyItemLine (hBodyItem, &dwBodyItemLen,
               szBody, wBodyLen))
    {   /* Unable to append text to body */
   error = "ERR_SENDMAIL_CANTAPPENDBODYLINE";
   goto CloseMsg;
    }

    if (status = MailAddBodyItem(hMsg, hBodyItem, dwBodyItemLen, NULL))
    {   /* Unable to add Body item to memo */
   error = "ERR_SENDMAIL_CANTADDBODY";
   goto CloseMsg;
    }

    /* Set "PostedDate" to the current time/date right now */
    OSCurrentTIMEDATE(&tdDate);
    if (status = MailAddHeaderItem( hMsg, MAIL_POSTEDDATE_ITEM_NUM,
                (BYTE*)(&tdDate),
                (WORD)sizeof(TIMEDATE)))
    {
   error = "ERR_SENDMAIL_CANTSETPOSTDATE";
   goto CloseMsg;
    }

   //add attachments
   if (attach && *attach)
   {
    TStringList list(attach);
    FOREACH(char * file,list)
    if (status = MailAddMessageAttachment( hMsg,file,GetFilename(file)))
     {
      error = "Problem with an attachment"; //keep going!
     }
    DONEFOREACH
   }

    /*  Deliver the message. */
    /*  If local, transfer message to the local mail.box */
    if (fLocal)
    {
   if (status = MailTransferMessageLocal(hMsg))
   {
       /* Unable to transfer message to local mail box */
       error = "ERR_SENDMAIL_CANTTRANSFER";
       goto CloseMsg;
   }
   /* else we successfully transferred msg to local mail box */
   /* Save msg to user's mail file and clean up.*/
   if (status = NSFNoteUpdate(hMsg, UPDATE_NOCOMMIT))
   {   /* Unable to update message in local mail file */
       error = "ERR_SENDMAIL_CANTUPDATEFILE";
   }
   goto CloseMsg;
    }

    /*  System is connected to the LAN (fLocal == FALSE). Open the
   router's MAIL.BOX on the mail server, then copy msg there.
     */
    OSPathNetConstruct(NULL, szMailServerName, MAILBOX_NAME, szMailBoxPath);

    if (status = NSFDbOpen(szMailBoxPath, &hMailBox))
    {
   error = "ERR_SENDMAIL_CANTOPENMAILBOX";
   goto CloseMsg;
    }
    /*  Copy the message, which is a note in the user's mail file,
   to the mail box. Perform this copy by changing the origin to
   the mail box then updating. Save the message's DBID, NOTEID,
   and OID. Set the DBID to the MAIL.BOX handle, the NOTEID to zero,
   and the OID to a newly generated OID associated with MAIL.BOX.
   Then update the message. This stores it in the MAIL.BOX file.
   Finally, restore the DBID, NOTEID, and OID.
     */
    Notes->NSFNoteGetInfo(hMsg, _NOTE_ID,  &OrigNoteID);
    Notes->NSFNoteGetInfo(hMsg, _NOTE_DB,  &hOrigDB);
    Notes->NSFNoteGetInfo(hMsg, _NOTE_OID, &OrigNoteOID);

    /* Set the message's OID database ID to match the mail box */
    if (status = NSFDbGenerateOID (hMailBox, &NewNoteOID))
    {
   /* Unable to generate originator ID for mail box */
   error = "ERR_SENDMAIL_CANTGENOID";
   goto CloseMsg;
    }
    Notes->NSFNoteSetInfo(hMsg, _NOTE_DB,  &hMailBox);
    Notes->NSFNoteSetInfo(hMsg, _NOTE_ID,  0);
    Notes->NSFNoteSetInfo(hMsg, _NOTE_OID, &NewNoteOID);

    /* Update message into MAIL.BOX on mail server. */
    if (status = Notes->NSFNoteUpdate(hMsg, UPDATE_NOCOMMIT))
    {
       /* Unable to update message to router mail box */
       error = "ERR_SENDMAIL_CANTUPDATEBOX";
    }
    else/* message successfully copied into router's mail box */
    {   /* restore msg to user's mail file and Update to save it there.*/
   Notes->NSFNoteSetInfo(hMsg, _NOTE_DB,  &hOrigDB);
   Notes->NSFNoteSetInfo(hMsg, _NOTE_ID,  &OrigNoteID);
   Notes->NSFNoteSetInfo(hMsg, _NOTE_OID, &OrigNoteOID);

   status = Notes->NSFNoteUpdate(hMsg, UPDATE_NOCOMMIT);
    }

CloseMsg:

    if (hRecipientsList != NULLHANDLE)  OSMemFree(hRecipientsList);
    if (hSendToList != NULLHANDLE)      OSMemFree(hSendToList);
    if (hCopyToList != NULLHANDLE)      OSMemFree(hCopyToList);

    if (error == 0)
    {
        if (status = Notes->MailCloseMessage(hMsg))
        {   /* Unable to close memo */
            error = "ERR_SENDMAIL_CANTCLOSE";
        }
        else
            if (hBodyItem != NULLHANDLE)  OSMemFree (hBodyItem);
    }
    else
    {
        status = Notes->MailCloseMessage(hMsg);
        if ((status == NOERROR) && (hBodyItem != NULLHANDLE))
            Notes->OSMemFree (hBodyItem);
    }
    if (hMailBox != NULLHANDLE)
        Notes->NSFDbClose (hMailBox);
CloseFile:

    if (status = Notes->MailCloseMessageFile(hMailFile))
    {   /* Unable to close mail file. */
      if (!error) error = "ERR_SENDMAIL_CANTCLOSEFILE";
    }

Exit:
   if (error) dspMessageBox(error,"NotesMail");
   return (error == 0);
}
#endif

#endif
