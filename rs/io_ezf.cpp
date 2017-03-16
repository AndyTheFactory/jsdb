#include "rslib.h"
#pragma hdrstop

XMLStream::XMLStream(const char * filename, const char * secname,
                        const char * DTD, TType type,
                   bool OmitTitle,int32 index )
 : Stream(type),system(DTD),Parent(NULL),AutoDelete(false)
{
 if (type == WriteOnly)
 {
   error = new xdb("Cannot open XML files in write-only mode");
   Type = NotOpen;
   return;
 }

 try {
 Parent = new FileStream(filename,Stream::OMBinary,type);
 } catch(xdb& err)
 {
  error = new xdb(err);
  Type = NotOpen;
  Parent = 0;
 }

 if (error) throw xdb(*error);
 AutoDelete = true;
 Init(secname,OmitTitle,index);
}

XMLStream::XMLStream(Stream* x,bool ad,
      const char * secname,bool OmitTitle,int32 index )
 : Stream(x->Type)
{
 Parent = x;
 AutoDelete = ad;

 Init(secname,OmitTitle,index);
}

void XMLStream::Init(const char * secname,bool OmitTitle, int32 index)
{
 if (!Type) return;
 if (!secname) return;
 data = NULL;

 startptr=0;
 endptr=size();
 int Position = 0;

 if ((Type != ReadOnly) && eof()) //writing a new file
 {
  if (!OmitTitle)
  {
   char* fn = (char*)filename();
   if (fn && stristr(fn,".HTM"))
   {
    writestr("<HTML>\r\n<!--Generated by Raosoft EZSurvey � -->\r\n");
    trailer << "</HTML>\r\n";
   }
   else
   {
    writestr("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n");
    if (*system)
    {
     writestr("<!DOCTYPE ");
     writestr(secname);
     writestr(" SYSTEM \"");
     writestr(system);
     writestr("\">\r\n");
    }
   }
  }
 }
 else //not writing a new file
 {
  TStr Current;
  while (Current != secname && !index) // && Current != "/XML") ignore end-tags.
  {
// <FORM BINARY=1 SIZE=bytes>
//  (compressed form data)
// </FORM>
// <Next tag>
    if (Options.GetInt("BINARY"))
     {
      goforward(Options.GetInt("SIZE"));
     }

    int c = StartTag(Current,0);
    if (c == EOF) {Current=0; break;}

    Position = pos();
    Options.Clear();

    if (Current == "!DOCTYPE")
    {
     TStr line;
     readline(line,'>');
     char* s = stristr(line,"SYSTEM");
     if (s) { s += 7; system=StripCharsFB(s,"\""); Replace(system,"\"",0);}
    }
    else
    {
     FinishTag(c,&Options);
    }

    if (Current == secname && index) index--;
  }

  if (Current == secname)
  {
   Position -= strlen(Current);
   Position -= 2; // the < and >

   if (Options.Has("CMP")) //decompress the section
    {
     //if (!strchr("\r\n",get()) putback(1); //read the newline
     // b64decode can handle the newline
     startptr = pos();
    }
   else
    {
     startptr = Position;
    }
  }
  else
  {
   Options.Clear();
  }

  //we've found an EOF or the location of the section of interest

  if (Type != ReadOnly)
  {
    TStr End("</",secname,">");

    if (Options.Has("CMP")) //decompress the section
    {
     endptr = startptr + Options.GetInt("SIZE");
     seek(endptr);
    }

    Parent->pos();

    if (ReadUntilWord(End))
    {
     endptr = pos(); //could be the end of the file
     char c;
     read(&c,1);
     trailer.Clear();
     if (c != '\n' && c != '\r') trailer.write(&c,1);
     trailer.Append(*this);
    }
  }

  seek(Position);

  if (Options.Has("CMP")) //decompress the section
   {
    MemoryStream * x = new MemoryStream();
    ReadSection(*x);
    seek(Position);
    x->rewind();
    data = x; //now data overrides all the read functions
   }

 } //of open an old file
}

XMLStream::~XMLStream()
{
 if (Type != ReadOnly)
  {
   trailer.seek(0);
   Parent->Append(trailer);
   FileStream * f = TYPESAFE_DOWNCAST(Parent,FileStream);
   if (f) f->SetEndOfFile(f->pos());
  }
 if (AutoDelete && Parent) delete Parent;
}

bool XMLStream::WriteCompressedSection(const char * objname,const char* title,Stream& in,
                  bool compress)
{
 if (!Type) return false;
 if (Type == ReadOnly) return false;

 MemoryStream m;

 if (compress)
 {
  int32 CRC=0;
  ZCompress(in,m,&CRC);
  m.rewind();
  *this << "<" << objname;
  if (title && *title) *this << " NAME=" << title ;
  *this << " BINARY=1 CMP=ZL64 CRC=" << CRC << ">\n";
  b64encode(m,*this);
 }
 else
 {
  *this << "<" << objname;
  if (title && *title) *this << " NAME=" << title ;
  *this << " BINARY=1 CMP=B64 >\n";
  b64encode(in,*this);
 }


 *this << "</" << objname << ">\r\n";

 return true;
}

int XMLStream::read(char * dest,int maxcopy)
  {return data? data->read(dest,maxcopy) : Parent->read(dest,maxcopy);}

int XMLStream::write(const char * src,int maxcopy)
  {
   return Parent->write(src,maxcopy);
  }

int32 XMLStream::ReadSection(Stream& out)
{
 if (!Type) return 0;
 if (startptr && endptr)
  {
   seek(startptr);
   if (!strcasecmp(Options.Get("CMP"),"ZL64"))
    {
     MemoryStream d;
     int32 CRC=Options.GetInt("CRC");
     {
      MemoryStream m;
      ReadUntilChar('=',&m);
      ReadUntilChar('\n',&m);

      m.rewind();

      while (IsSpace(m.peek())) m.get();

      b64decode(m,d);
      d.rewind();
     }
     int32 CRC2=0;
     int32 r = ZExpand(d,out,&CRC2);
     if (r == 0)
      {
       d.rewind();
       d.goforward(2);
       CRC2=0;
       r = ZExpand(d,out,&CRC2);
      }
     return CRC ? CRC != CRC2 ? 0 : r : r;
    }
   else if (!strcasecmp(Options.Get("CMP"),"B64"))
    {
     return b64decode(*this,out);
    }
   else
    {
     return out.Append(*this,endptr-startptr);
    }
  }
 return 0;
}

