#ifndef _RS_MIMEMAIL_H
#define _RS_MIMEMAIL_H

#ifndef _RS_STREAM_H
#include "rs/stream.h"
#endif
#ifndef _RS_SYSTEM_H
#include "rs/system.h"
#endif

inline bool chkCHAR(int c) {return c >= 0 && c <= 127;}
inline bool chkALPHA(int c){return (c >= 65 && c <= 90) || (c >= 97 && c <= 122);}
inline bool chkDIGIT(int c){return c >= 48 && c <= 57;}
inline bool chkCTL(int c)  {return c >= 0 && c <= 31;}
inline bool chkSPACE(int c){return c == 32;}
inline bool chkHTAB(int c) {return c == 9;}
inline bool chkQUOTE(int c){return c == 34;}
inline bool chkLWSP(int c) {return chkSPACE(c) || chkHTAB(c);}

class strumail                              /* structured mail */
{
   public:
   TParameterList header;
   MemoryStream body;

   strumail();
   ~strumail();

   bool parse(Stream &s);
   
   protected:

   bool parseheader(Stream &s);
   void parsebody(Stream &s);
};


class mimemail
{
 public:
   TParameterList header;

   // we need to delete an object of its subclass via a mimemail pointer
   virtual ~mimemail() { }

   // is it a compound mime mail?
   virtual bool IsCompound() = 0;

};

/* given a structured mail, creates a mime mail */
mimemail * ParseMIMEMessage(strumail &sm);

class mmcomp: public mimemail  // compound mime mail
{
 public:
   TRow<mimemail> parts;  // sub mails (ie, attachments)
                          //the 'body' stream isn't used
   mmcomp(TParameterList* header=0);
   ~mmcomp();

   bool IsCompound() { return true; }
};

class mmatom: public mimemail // atomic mime mail
{
//   bool b64decode(Stream& p);

   public:
   MemoryStream body;

   mmatom(TParameterList* header=0, Stream* body=0);
   ~mmatom();
   bool Decode(Stream &p);
   bool GetFile(TStr &file,const char * ext);
//   bool getdecoding(TStr &d);

   bool IsCompound() { return false; }
};


#endif

