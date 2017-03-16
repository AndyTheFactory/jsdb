#ifndef _RS_STREAM_H
#define _RS_STREAM_H

#ifdef __MWERKS__
#include <fcntl.h>
#endif
#ifdef __BORLANDC__
//#include <iostream.h>
#include <fcntl.h>
#include <io.h>
#endif

#ifndef EOF
#define EOF (-1)
#endif

#ifndef XP_WIN
#define NO_COMM_STREAM
#endif

class Stream;

//stream names are type://name
//                 http://uri      (inet, send+recv)
//                 http://proxy:port/http://      (inet, send+recv)
//                 file://filename (file, read+write)
//                 temp://size     (memory, read+write)
//                 com1://speed    (com,  send+recv)
//                 text://cdata    (bytestream, read+write)

Stream* OpenStream(const char* url,xdb* error = 0);

//translator functions

//io_print
/*!
Start and end are typically { and }, but could be any character, even the same.
The second version uses words as the start and end sequences, instead of chars.

If the first character of the symbol to be replaced is a blackslash,
a variety of escaping techniques are available.

 - \$ escapes @ and $ for perl
 - \& escapes HTML
 - \% escapes to a URL-encoded string
 - \\ escapes with backslashes (\r, \n, \t, \\, \", \')
*/
void FormatText(TNameValueList& data,Stream& in,Stream& out,
                 int start ='{', int end='}');

void FormatText(TNameValueList& data,Stream& in,Stream& out,
                 const char* start,const char * end);

//io_compress

//int32 CRC32(int32 seed,const char * buf, int size);

//-------------------------------------------------------------------------------

class Stream // : public TDataSource
{public:
enum TOpenMode {OMDefault   = 0,  //binary, buffered
                OMBinary    = 0,
                OMText      = 2,
                OMUnbuffered= 4};

enum TType {NotOpen     = 0,
            ReadOnly    = 1,
            WriteOnly   = 2,
            ReadWrite   = 3,
            AppendOnly  = 4,
            Create      = 5,
            IORead      = ReadOnly,
            IOReadWrite = ReadWrite,
            IOWrite     = WriteOnly,
            IOAppend    = AppendOnly
           ,IOCreate    = Create
            } Type;

xdb* error;
// virtual ancestor functions
Stream(TType _Type = NotOpen);
virtual ~Stream();

// override these if necessary
virtual int read(char * dest,int maxcopy) {return 0;}
virtual int write(const char * src,int maxcopy) {return 0;}
virtual long seek(long offset) {return 0;} //! -1 to go to the end
virtual long goforward(long delta) {return 0;}//! seeks forward
virtual long putback(long delta) {return 0;} //! seeks backwards
virtual long size() {return 0;}
virtual long pos() {return 0;}
virtual bool eof() {return true;}
virtual const char * filename() {return NULL;}
virtual void rewind() {seek(0);}
virtual bool canwrite() {return Type > ReadOnly;}
virtual bool canread() {return Type != WriteOnly && Type != AppendOnly && pos() != size();}

/*
In XML, a tag must always have a terminator. For example, text is:

   This is <b> bold text </b>

The </b> tag terminates the <b> tag. But if you have a tag that stands by
itself, like a page break in EZF files, it needs to be self-terminating:

   <PAGE/>  or  <PAGE />

In case your parser cares about this, we have the EndTag parameter which tells
you if the terminator was included in a tag or not. EndTag will be true for
these types of tags:

  </INPUT>           (StartTag)
  <PAGE/>            (StartTag)
  <PAGE />           (FinishTag)
  <SKIP ... />       (FinishTag)

*/

// Markup/XML read and write
void WriteTag(const char * type,TParameterList & Options,bool EndTag = false);

//start read HTML/XML
int ReadUntilTag(const char * tag,Stream * SkipText=0,bool *EndTag = 0);

int StartTag(TStr& type,Stream * SkipText=0,bool *EndTag = 0,const char* AllowedTags=0);

void PutBackTag(const char * tag,bool *EndTag = 0);

void SkipTag(int StartResult, Stream* TagText=0);

//finish read HTML/XML
bool FinishTag(int StartResult, TParameterList* params=0,bool *EndTag = 0,bool decode = true);

void Decode(TParameterList& list, bool url = false);

// iostream drop-in replacement functions
int peek();
int get()  { char c = 0; return read(&c,1) ? c : EOF;}
void put(char c) {char x = c; write(&x,1);}

//
// macros for typecasting
int write(const wchar_t * src,int maxcopy)  {return write((const char *)src,2*maxcopy);}
int write(const void * src,int maxcopy)  {return write((const char *)src,maxcopy);}
//int read(wchar_t * src,int maxcopy)  {return read((char *)src,2*maxcopy);}
int read(void * src,int maxcopy)  {return read((char *)src,maxcopy);}

//write functions return the number of bytes saved
//read functions return the number of items loaded
size_t Write(TStringList& list, const char * delimiter);
size_t Read(TStringList& list,const char* skip,char delim);

size_t Write(TNameValueList& list);
size_t Read(TNameValueList& list);

// Reads a string of the form name1=value1/name2=value2/...
// does write a final delimiter
// default delimiter is a forward slash
size_t WritePaired(TNameValueList& in, const char * delim=0, const char* eq=0);
size_t ReadPaired(TNameValueList& out, char delim='\n', int eq='=');

// Reads a string of the form name1: value1\nname2: value2\n\n
// does write a final delimiter
// if a line begins with whitespace, appends to the last value.
size_t WriteMIME(TNameValueList& in);
size_t ReadMIME(TNameValueList& out);

// saves these as int32, but on reading, converts to whatever an int is.
size_t writeints(int* ints,size_t count); //returns the number of bytes written
size_t readints(int* ints,size_t count);  //returns the number of items read

bool readint64(int64& x) {return (read(&x,8)==8);}
bool readint32(int32& x) {return (read(&x,4)==4);}
bool readuint32(uint32& x) {return (read(&x,4)==4);}
bool readint16(int16& x) {return (read(&x,2)==2);}
bool readuint16(uint16& x) {return (read(&x,2)==2);}
bool readint8(int8& x) {return (read(&x,1)==1);}
bool readuint8(uint8& x) {return (read(&x,1)==1);}
// string io
int writestr(const char * str)  {return str ? write(str,strlen(str)) : 0 ;}
int writestr(const uint16 * str)  {return str ? write(str,ucslen(str)) : 0 ;}

int writestr(const char * s1, const char* s2, const char* s3=0, const char* s4=0, const char* s5=0)
 {return writestr(s1)+writestr(s2)+writestr(s3)+writestr(s4)+writestr(s5);}

int writestr(const uint16 * s1, const uint16* s2, const uint16* s3=0, const uint16* s4=0, const uint16* s5=0)
 {return writestr(s1)+writestr(s2)+writestr(s3)+writestr(s4)+writestr(s5);}

size_t readstring(char * str, size_t maxcopy)
 {return readline(str,maxcopy,0);}

size_t readline(char * dest, size_t maxcopy,int delim = '\n');//max 64K
size_t readline(TStr & dest,int delim = '\n'); //max 8K
count_t readline(Stream& dest,int delim = '\n');//max 4G

// utility functions
long Append(Stream& copy, long maxcopy=LONG_MAX);
long AppendAsText(Stream& copy, long maxcopy=LONG_MAX);

// parsing functions
int ReadUntilChar(int check, Stream* out=0);
int ReadUntilChar(const char * check, Stream* out=0);
bool ReadUntilWord(const char * check, Stream* out=0);
bool ReadUntilBytes(const char * check, size_t length, Stream* out=0);
int EatChars(const char * skip);

// handy operators
friend Stream& operator << (Stream& os, uint16 * str );
friend Stream& operator << (Stream& os, const char * str );
friend Stream& operator << (Stream& os, int32 i ); //itoa(i,10)

#ifdef _HAS_INT64
inline void Write(int64 i ); //itoa(i,10)
#endif

friend Stream& operator >> (Stream& os, TStr &str); //readline
friend Stream& operator >> (Stream& os, char & c);  //readchar
};

//read a single character
inline Stream& operator >> (Stream& os, char & c)
{os.read(&c,1);
 return os;
}

//insert and remove strings.
//the >> operator reads until an EOL or EOL marker
inline Stream& operator >> (Stream& os, TStr &str)
{os.readline(str);
 return os;
}

inline void strcat(Stream&x, const char* y) {x.writestr(y);}

//formatted int

#ifdef _HAS_INT64
inline void Stream::Write(int64 i)
{char c[64]; _i64toa(i,c,10);  (*this) << c;}
#endif

inline Stream& operator << (Stream& os, int32 i )
{
 char c[64];ltoa(i,c,10); os.writestr(c); return os;
}

//writes until a null * is encountered
inline Stream& operator << (Stream& os, const char * str )
{os.writestr(str);
 return os;
}

inline Stream& operator << (Stream& os, uint16 * str )
{
 if (str) os.write(str,ucslen(str));
 return os;
}

//-------------------------------------------------------

class ConcatStream : public Stream
{
 public:
 TList<Stream> items;
 TStr name;

 ConcatStream() {}
 ~ConcatStream() {error=0;} //children delete their own error codes

 const char * filename() {return name;}

 void insert(Stream* s)
  {
      if (!items.Count()) error = s->error;
      items.Add(s);
  }

 bool eof()
 {
     switch (items.Count())
     { case 0: return true;
       case 1: return items[0]->eof();
       default : return false;
     }
 }

 bool canread()
 {
    return items.Count() != 0;
 }

 bool canwrite()
 {
     switch (items.Count())
     { case 0: return true;
       case 1: return items[0]->canwrite();
       default : return false;
     }
 }

 int read(char *b, int n)
   {
       int x = 0;
       while (x < n && items.Count())
       {
           int y = items[0]->read(b+x, n-x);
           if (y < n - x)
           {
             items.Destroy(0);
             if (items.Count()) error = items[0]->error;
           }
           x += y;
       }
       return x;
    }
};

#include "rs/io_pipe.h"

#ifndef NO_INET_STREAM
#include "rs/io_inet.h"
#endif

#ifndef NO_COMM_STREAM
#include "rs/io_comm.h"
#endif

#ifndef NO_MEMORY_STREAM
#include "rs/io_memory.h"

#ifndef NO_FILE_STREAM
#include "rs/io_file.h"
#endif

#endif

#endif

