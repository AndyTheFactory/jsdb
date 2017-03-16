#include "rslib.h"
#pragma hdrstop

size_t Stream::Write(TStringList& list, const char * delimiter)
{
 size_t ret = 0;
 FOREACH(const char *c,list)
  ret += writestr(c);
  ret += writestr(delimiter);
 DONEFOREACH
 return ret;
}

size_t Stream::Read(TStringList& list,const char* skip,char delim)
{
    size_t i = 0;
    TStr tmp;
    while (readline(tmp,delim))
    {
     if (!skip || !strchr(skip,*tmp))
       {
        list.Add(new TStr(tmp));
       }
     i++;
    }
    return i;
}

size_t Stream::Write(TNameValueList& list)
{
 size_t ret=0;
 FOR(i,list)
  {
   ret += writestr(list.Name(i));
   const char * v = list.Value(i);
   if (v)
    {
     ret += write("=",1);
     ret += writestr(v);
    }
   ret += write("",1);
  }
 ret += write("",1);
 return ret;
}

/* Reads a string of the form name1=value1/name2=value2/...*/
// does write a final delimiter
void Split(char*s,char*&c,int eq)
{
   c = strchr(s,eq);
   if (c)
    {
     *c=0;
     c++;
     while (*c && *c == ' ') c++;
    }
}

size_t Stream::Read(TNameValueList& list)
{
 TStr s;
 char*c =0;
 size_t i=0;
 while (readline(s,0))
  {
   if (s[0]==0) break; // that's the end of the header: \n\n
   Split(s,c,'=');
   list.Set(s,c);
   i++;
  }
 return i;
} //returns number of items read


/* Reads a string of the form name1=value1/name2=value2/...*/

size_t Stream::ReadPaired(TNameValueList& out, char delim, int eq)
{
 TStr s;
 size_t i=0;
 char * c = 0;
 while (readline(s,delim))
  {
   i++;
   Split(s,c,eq);
   out.Set(s,c ? c : "");
  }
 return i;
}

size_t Stream::WritePaired(TNameValueList& in, const char * delim, const char* eq)
{
 size_t ret=0;
 FOR(i,in)
  {
   ret += writestr(in.Name(i));
   const char * v = in.Value(i);
   if (v)
    {
     ret += writestr(eq ? eq : "=");
     ret += writestr(v);
    }
   ret += writestr(delim ? delim : "/");
  }
 return ret;
}


  /* Reads a string of the form name1: value1\nname2: value2\n\n*/
  // does write a final delimiter
  //if a line begins with whitespace, appends to the last value.
size_t Stream::ReadMIME(TNameValueList& out)
{
 TStr s;
 TStr lastname, lastvalue;
 size_t i = 0;
 char * c = 0;

 while (readline(s,'\n'))
  {
   Replace(s,'\r',0);
   if (s[0]==0) break; // that's the end of the header: \n\n

   if (s[0]==' ' || s[0]=='\t') //append to the last value
    {
     char*c = s;
     while (c[0] && (c[0]==' ' || c[0] == '\t')) c++;
     c--; //it does need a space to delimit it from the previous line.
     lastvalue += c;
     out.Set(lastname,lastvalue);
     //else disregard the line
    }
   else
    {
     Split(s,c,':');
     lastname=s;
     lastvalue=c;
     out.Set(s,c);
     i++;
    }
  }
 return i;
 }

size_t Stream::WriteMIME(TNameValueList& in)
{
 size_t r = 0;
 FOR(i,in)
 {
   r += writestr(in.Name(i));
   r += writestr(": ");

   const char* c = in.Value(i);

   if (c && strchr(c,'\n'))
   {
    while (*c)
    {
     if (*c == '\n')
       r += writestr("\n ");
     else
       r += write(c,1);
     c++;
    }
   }
   else
   r += writestr(in.Value(i));
   r += writestr("\n");
 }
 r += writestr("\n");
 return r;
}

