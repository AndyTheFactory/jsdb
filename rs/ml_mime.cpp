#include "rslib.h"
#pragma hdrstop

#ifndef NO_POP_MAIL

/*#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <fstream.h>
#include <sys\stat.h>*/
#include "rs/ml_mime.h"
//#include "rs/pointer.h"
//#include "rs/file.h"
//#include "rs/os.h"
/* strumail */


strumail::~strumail() {}

strumail::strumail() {}

bool strumail::parse(Stream &s)
{
// if (!header.ReadMIMEheader(s)) return false;
   if (!parseheader(s))
   {
      return false;
   } //ain't broke, don't fix. Not yet, anyway.
   parsebody(s);

   return true;
}
         /*
void KillCR(char * c)
 {
  while (*c)
   {
    if (*c == '\r')
      *c = 0;
    else
      c++;
   }
 }
    */
bool strumail::parseheader(Stream &s)
{
 s.ReadMIME(header);
 return true;
}

void strumail::parsebody(Stream &s)
{
   body.Clear();
   body.Append(s);
   body.rewind();
}

/* del the sub mails */
mmcomp::mmcomp(TParameterList* h)
{
 if (h) header = *h;
}

mmcomp::~mmcomp()
{
}

/* given a structured mail, create and return the corresponding mime mail */
mimemail *ParseMIMEMessage(strumail &sm)
{
   bool isend,isnew;

   TStr t,start,end;

   TPointer<Stream> q(0);

   t = sm.header["Content-Type"];    /* get value for field Content-Type */

   if (!*t)
   {
      return new mmatom(&sm.header,&sm.body);
   }

   if (stristr(t,"multipart"))                                /* compound */
   {
      mmcomp *mc;
      char* c = stristr(t,"boundary=");
      if (c)
        {
         c += 9;
         if (*c == '\"')
          {
           c++;
           char* d=strchr(c,'\"');
           if (d) *d = 0;
          }
         t = TStr(c,strcspn(c,"; \t"));
        }
      else
       {
        return NULL;
       }

      start += "--";    /* --b */
      start += t;
      end = start;      /* --b-- */
      end += "--";

      mc = new mmcomp(&sm.header);

      TStr line;
      while (sm.body.readline(line))
      {
       Replace(line,'\r',0);
//       KillCR(line);
       isend = false;
       isnew = false;

       if (start == line) /* end current part & start new part */
           isend = isnew = true;
       else if (end == line)     /* end current part & finish */
           isend = true;

       if (isend) // wrap it up!
        {
         if (q) //close the message part
         {
          mimemail *w;
          q->rewind(); //close the file
          strumail x;      // build a structured mail from this stream
          if (x.parse(*q))
           {
            w = ParseMIMEMessage(x); // structured mail -> mime mail
            if (w) mc->parts.Add(w);  // add the compound mail
           }
          else
           {                           //something wrong with it, so just
            q->rewind();
            mc->parts.Add(new mmatom(0,q)); //send the text out
           }
          q=0;
         }  // if q

        if (!isnew) break; //we're done!

        q = new MemoryStream;
        continue;
       }

      if (q)              // we do have a current part
        *q << line << "\r\n";

      //else just read the line

      }

      if (q != 0)   /* current part not ended! */
      {    // so what? Just discard it
       q->rewind();
       mc->parts.Add(new mmatom(0,q)) ;//send the text out
      }
      return mc;
   }
   else                                   /* atomic */
   {
     return new mmatom(&sm.header,&sm.body);
   }
}

/* get the encoding used for this mail */
mmatom::mmatom(TParameterList* h, Stream* b)
{
 if (h) header = *h;
 if (b) body.Append(*b);
 body.rewind();
}

mmatom::~mmatom() {};

/*
bool mmatom::getdecoding(TStr& d)
{
//   char *p;
   TStr t;

   t = header["Content-Transfer-Encoding"];
   if (!*t)
   {
      return NULL;
   }
   TStr p(t);        // istrstream needs to modify the string

   TBytesStream ss(p);
//   istrstream ss(p);

   mimelex ml(&ss);             // we want a symbol stream, not char stream

   if (ml.peeksym() != ml.symtoken)   //expect a token as the encoding
   {
      return false;
   }
   ml.peeksymtoken(d);        // store the encoding

   ml.skip();                  // skip the encoding

   if (ml.peeksym() != ml.symend)    // expect end of stream
   {
      return false;
   }
//   delete [] p;

   return true;
}
            */

bool mmatom::Decode(Stream &of)
{
   TStr d(header["Content-Transfer-Encoding"]);

   if (!*d) return false;

   if (d == "7bit" || d == "8bit" || d == "binary") //no decoding
   {
//      ofstream of(p.c_str());
      of.Append(body);
      body.rewind();
      return true;
   }
   if (d == "quoted-printable")
   {
      bool r = qpdecode(body,of);
      body.rewind();
      return r;
   }
   if (d == "base64")
   {
      bool r = b64decode(body,of);
      body.rewind();
      return r;
   }
   return false;                                           /* unknown encoding */
}


//bool mmatom::b64decode(Stream& h)
//{
// return ::b64decode(body,h);
//}


bool mmatom::GetFile(TStr &file,const char * ext)
{
   CreateTempFile(file,ext);
   FileStream Out(file,Stream::OMBinary,
                        Stream::WriteOnly);
   return Decode(Out);
}

#endif
