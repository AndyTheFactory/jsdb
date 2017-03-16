#include "rslib.h"
#pragma hdrstop

TMailList::TMailList(MailLibrary* l,bool ad,
                             MailCallback CB,void*v)
                             :DataTable()
{
 lib = l;
 AutoDelete = ad;
 Refresh(CB,v);
 Style = (ETableStyle)((int) Style | (int)AllowDelRow);
}

TMailList::TMailList(EMailType MailType,const char * username,
          const char* password,const char * server,
          MailCallback CB,void*v)
          :DataTable()
{
 lib = OpenMailSystem(MailType,true,username,password,server);
 AutoDelete = true;
 Refresh(CB,v);
 Style = (ETableStyle)((int) Style | (int)AllowDelRow);
}

bool TMailList::Refresh(MailCallback CB,void*v)
{
 if (!lib)
 { return false; }

 msgs.Clear();

 TStr err;
 if (!lib->GetMessages(msgs,false,err,CB,v))
  {
   if (*err) if (!lib->error) lib->error = new xdb(err);
   if (*err) return false;
  }
  return true;
}

TMailList::~TMailList()
 {
  msgs.Clear();
  if (lib && AutoDelete) {delete lib; lib=0;}
 }


bool TMailList::SetDataC(count_t i, size_t j,const char * c)
{
 MailMessage * msg = msgs[i-1];
 if (!msg || !c || j) return false;

 if (*c == 'D') {msg->Deleted = true; return true; }
 else if (!*c) { msg->Deleted = false; return true;}
 return false;
}

const char * TMailList::GetDataC(count_t i, size_t j)
{
 MailMessage * m = msgs[i-1];
 if (!m) return 0;

 switch (j)
  {
   case 0: if (m->Deleted) return "D";
           if (!m->IsRead()) return "S";
           return "";

   case 1: return m->Sender();

   case 2:{ SYSTEMTIME tm;
            m->Time(tm);
            TempBuffer.Resize(256);
#ifdef XP_WIN
            GetDateFormat(0,DATE_SHORTDATE ,&tm ,0 ,TempBuffer,256);
#else
            sprintf(TempBuffer,"%d/%d/%d %d:%d:%d",
             tm.wMonth,tm.wDay,tm.wYear,tm.wHour,tm.wMinute,tm.wSecond);
#endif
            return TempBuffer;
           }

   case 3: TempBuffer.Resize(56);
           sprintf(TempBuffer,"%d",(int)m->Size());
           return TempBuffer;

   case 4: return m->Subject();

   case 5: TempBuffer.Resize(56);
           sprintf(TempBuffer,"%d",(int)m->AttachCount());
           return TempBuffer;
   default: return 0;
  }
}

const char * TMailList::ColumnTitle(size_t index)
{
 switch (index)
  {
   case 1: return "Sender";
   case 2: return "Time";
   case 3: return "Size";
   case 4: return "Subject";
   case 5: return "Attachments";
   default: return 0;
  }
}

