#include "rslib.h"
#pragma hdrstop

// file://user:password@c:/path/.../file.dbf?query
// http://user:password@host/filename?query
// notes://user:password@file  (nsf database)
// notes://user:password@      (mail)
// file,dbf,ascii,http,ftp,notes,odbc,sql,comX,temp,text
// pop3://user:password@server      to open a mailbox
// cmc://user:password@profile      to open a mailbox
// mail://user:password@service-type to load a mail library

void URLSplit(const char * URL,
                    TStr& service, // file or http
                    TStr& user,    // user
                    TStr& password,// password
                    TStr& host,    // dbf or host
                    TStr& filename,//c:\path\...\file.dbf or filename
                    TStr& query)   // query
{
 TStr fname(URL);
 URLDecode(fname);
 service = "file";

 int i = fname.indexOf("://");
 if (i != -1)
 {
  fname[i]=0;
  service=fname;
  fname = (char*)fname + (i + 3);

  i = fname.indexOf("?");
  if (i != -1)
   {
    fname[i]=0;
    query = (char*)fname + (1 + i);
   }

  i = fname.indexOf("@");
  if (i != -1)
   {
    fname[i]=0;
    user = fname;
    fname = (char*)fname + (1 + i);

    i = user.indexOf(":");
    if (i != -1)
    {
     user[i]=0;
     password = (char*)user+ (i+1);
    }
   }
 }

 if (service == "file" || service == "dbf" || service == "ascii")
 {//local files
  Replace(fname,"/",'\\');
  filename = fname;
  host = GetExtension(fname);
 }
 else if (service == "notes")
 {//notes://user:password@file
  if (!*fname)
  host.Exchange(fname);
 }
 else
 {
   host = fname;
   char * t = strchr(host,'/');
   if (t)
    {
     *t=0; filename = t + 1;
    }
 }
}

void URLEncode1(WStr& out, const char* tin,size_t len)
{
 const wchar_t conv[] = L"0123456789abcdef";

 unsigned const char * in = (unsigned const char *)tin;
 if (len == 0 && *tin) len = strlen(tin);

 out.Resize(len*3 +1);
 size_t x=0;
 uint16 * o = (uint16*)out;

 for(size_t i=0; i < len; i++)
  {
   register unsigned int c = in[i];
   if (!isalnum(c) && c != '_')
    {
     o[x] = '%';
     o[x + 1] = conv[(c & 0x00f0) / 16];
     o[x + 2] = conv[(c & 0x000f)];
     x += 3;
    }
   else
    {
     o[x]=in[i];
     x++;
    }
  }
 o[x]=0;
}

void URLEncodeURL(TStr& out, const char* tin,size_t len)
{
 const char conv[] = "0123456789abcdef";

 unsigned const char * in = (unsigned const char *)tin;
 if (len == 0) len = strlen(tin);

 out.Resize(len*3 +1);
 size_t x=0;

 for(size_t i=0; i < len; i++)
  {
   register unsigned int c = in[i];

   if (!isalnum(c) && !strchr(":;,.?/~@#$&*-=",c))
    {
     out[x] = '%';
     out[x + 1] = conv[(c & 0x00f0) / 16];
     out[x + 2] = conv[(c & 0x000f)];
     x += 3;
    }
   else
    {
     out[x]=in[i];
     x++;
    }
  }
 out[x]=0;
}


void URLEncodeXML(TStr& out, const char* tin,size_t len)
{
 const char conv[] = "0123456789abcdef";

 unsigned const char * in = (unsigned const char *)tin;
 if (len == 0) len = strlen(tin);

 out.Resize(len*3 +1);
 size_t x=0;

 for(size_t i=0; i < len; i++)
  {
   register unsigned int c = in[i];

   if (c == '&' || c < ' ' || c == '<'  || c == '>' ||
          c == '\"' || c == '%' || c == '+')
    {
     out[x] = '%';
     out[x + 1] = conv[(c & 0x00f0) / 16];
     out[x + 2] = conv[(c & 0x000f)];
     x += 3;
    }
   else
    {
     out[x]=in[i];
     x++;
    }
  }
 out[x]=0;
}

static int x2c(unsigned char *what)
{
    register int digit;

    digit =
      (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *=
      16;
    digit +=
      (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

void URLDecode(char * turl,bool XMLonly) {
    register int x,y;
    unsigned char * url = (unsigned char*) turl;

    //oops -- forgot plus to space
    if (!XMLonly) Replace(turl, "+",' ');

    for(x=0,y=0; url[y]; ++x,++y)
    {
     if (url[y] == '%' &&
          ((url[y+1] >= 'A' && url[y+1] <= 'F')
           || isdigit(url[y+1])))
        {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
     else url[x] = url[y];
    }
    url[x] = 0;
}
