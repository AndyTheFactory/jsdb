#ifndef _RS_STREAM_H
#pragma warn "include rs/stream.h instead"
#endif



class MemoryStream: public Stream
{//a glorified TStr, only optimized for appending and binary data.
public:
TChars Mem;
long Position,Maxsize;

MemoryStream() ;
MemoryStream(long size);
~MemoryStream() ;
long seek(long offset);
virtual long size();
virtual long pos();
virtual bool eof();
long goforward(long delta);//seeks forward
long putback(long delta);
virtual int read(char * dest,int maxcopy);
virtual int write(const char * src,int maxcopy);
void Clear(long size=0);
void Resize(long size);
operator char * () {((char *)Mem)[Maxsize] = 0; return (char*)Mem;}
void operator = (const char * c) {Clear(); writestr(c);}
bool operator == (const char * c) {return !strcasecmp((char*)*this,c?c:"");}
bool operator != (const char * c) {return strcasecmp((char*)*this,c?c:"");}
bool canwrite() {return true;}

};

//ByteStream doesn't own its data. It's for turning a string that
//you own into a stream temporarily.
class ByteStream: public Stream
{
public:
char * Buf;
long Position,Maxsize;

ByteStream(char * Buf,long Size=0);
ByteStream(const char * Buf,long Size=0);
ByteStream(uint16 * Buf,long Size=0);
ByteStream(const uint16 * Buf,long Size=0);  //number of bytes, not characters
~ByteStream();
long seek(long offset);
long size();
long pos();
bool eof();
long goforward(long delta);//seeks forward
long putback(long delta);
int read(char * dest,int maxcopy);
int write(const char * src,int maxcopy);
bool canwrite() {return Position < Maxsize;}
};
