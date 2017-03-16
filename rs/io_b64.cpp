#include "rslib.h"
#pragma hdrstop

static char basis_64[] =
 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


int output64chunk(uint8 c1, uint8 c2, uint8 c3, int pads, Stream& outfile)
{
  char out[4];
  int pos = 0;
    out[pos++] = basis_64[c1>>2];
    out[pos++] = basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)];

    if (pads == 2) {
        out[pos++]='=';
        out[pos]='=';
    } else if (pads) {
        out[pos++] = basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)];
        out[pos] = '=';
    } else {
        out[pos++] = basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)];
        out[pos] = basis_64[c3 & 0x3F];
    }
  return outfile.write(out,4);
}

int32 b64encode(Stream& in, Stream& out)
{
  //  int limit = 76;
    uint8 c[3];// 1, c2, c3,
    int ct=0;
    int32 written=0;

//    if (limit && limit < 73) return false;

    int read;
    memset(c,0,sizeof(c));

    while ((read = in.read(c,3)) != 0)
    {
     written += output64chunk(c[0],c[1],c[2],3-read,out);
     ct += 4;

     if (ct > 71)
     {
      written += out.write("\n",1);
      ct = 0;
     }
     memset(c,0,sizeof(c));
    }

    if (ct)
    {
      written += out.write("\n",1);
    }
    return written;
}

/* base64 decoding. 4 6bit characters representing 3 8bit bytes */
/*                                                              */
/* 123456 123456 123456 123456                                  */
/* ---------**** ****++ ++++++                                  */
/*                                                              */


/* given a char, return the index into the base64 table */
int b64c2idx(int c)
{
   if (c >= 'A' && c <= 'Z')    // 'A' is 0, 'Z' is 25
   {
      return c-'A';
   }
   if (c >= 'a' && c <= 'z')    // 'a' is 26, 'Z' is 51
   {
      return c-'a'+26;
   }
   if (c >= '0' && c <= '9')    // '0' is 52, '9' is 61
   {
      return c-'0'+52;
   }
   if (c == '+')                // '+' is 62
   {
      return 62;
   }
   if (c == '/')               // '/' is 63
   {
      return 63;
   }
   return -1;                 // unknown char
}

/* get the next valid base64 char */
int b64getnextch(Stream& body)
{
   while (1)
   {
     int c = body.get(); //read one character
     if (c == EOF) return EOF;
     if (c == '=' || b64c2idx(c) != -1)  // one of those 65 valid characters?
         return c;
   }
}

int32 qpdecode(Stream& In, Stream& h)
{
  char c;
  char hex[3];
  hex[2]=0;
  int32 written=0;

  while (In.read(&c,1) == 1)
  {
   if (c == '=')
    {
     if (In.read(hex,1) != 1) break;
     if (hex[0] == '\r')
       {
        if (In.read(hex,1) != 1) break;
       }
     if (hex[0] == '\n') continue; //end of line

     if (In.read(hex+1,1) != 1) break;
     c = strtol(hex,0,16);
     written += h.write(&c,1);
    }
   else written += h.write(&c,1);
  }

  return written;
}

int32 b64decode(Stream& In, Stream& h)
{
//   int i;
//   int j;
   int c1;
   int c2;
   int c3;
   int c4;
   int i1;
   int i2;
   int i3;
   int i4;
   int32 written = 0;

   unsigned char b[3];

   while (1)
   {
      c1 = b64getnextch(In);
      c2 = b64getnextch(In);
      c3 = b64getnextch(In);
      c4 = b64getnextch(In);

      if (c1 == EOF || c4 == EOF) // no more, or not 4 characters
      {
         break;
      }

      if (c1 != '=' && c2 != '=' && c3 != '=' && c4 != '=')
      {
         i1 = b64c2idx(c1);
         i2 = b64c2idx(c2);
         i3 = b64c2idx(c3);
         i4 = b64c2idx(c4);
         b[0] = (unsigned char)(((i1 << 2) & 0xff) + (i2 >> 4));
         b[1] = (unsigned char)(((i2 << 4) & 0xff) + (i3 >> 2));
         b[2] = (unsigned char)(((i3 << 6) & 0xff) + (i4 >> 0));
         written += h.write(b, 3);
         continue;
      }
      if (c1 != '=' && c2 != '=' && c3 != '=' && c4 == '=')
      {
         i1 = b64c2idx(c1);
         i2 = b64c2idx(c2);
         i3 = b64c2idx(c3);
         b[0] = (unsigned char)(((i1 << 2) & 0xff) + (i2 >> 4));
         b[1] = (unsigned char)(((i2 << 4) & 0xff) + (i3 >> 2));
         written += h.write(b, 2);
         break;
      }
      if (c1 != '=' && c2 != '=' && c3 == '=' && c4 == '=')
      {
         i1 = b64c2idx(c1);
         i2 = b64c2idx(c2);
         b[0] = (unsigned char)(((i1 << 2) & 0xff) + (i2 >> 4));
         written += h.write(b, 1);
         break;
      }
      /* illegal sequence. ignore it */
   }
//   close(h);

   return written;
}
