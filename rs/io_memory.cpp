#include "rslib.h"
#pragma hdrstop

#ifndef NO_MEMORY_STREAM

MemoryStream::MemoryStream()
 : Stream(ReadWrite), Position(0), Mem(1024)
 {
  Maxsize = 0;
 }

MemoryStream::MemoryStream(long size)
: Stream(ReadWrite), Position(0), Mem(size)
 {
  Maxsize = 0;
 }

MemoryStream::~MemoryStream()
 {
 }

void MemoryStream::Resize(long size)
 {
  Mem.Resize(size);
  Maxsize=size;
  Position = min(Position,Maxsize);
 }

void MemoryStream::Clear(long size)
 {
  if (Mem.size > 16384) Mem.Resize(size);
  Position = 0;
  Maxsize = 0;
 }

long MemoryStream::seek(long offset)
 {
  Position = min((size_t)offset,Mem.size);
  return Position;
 }

long MemoryStream::goforward(long delta)
 {
  return seek(Position+delta);
 }

long MemoryStream::putback(long delta)
 {
  return seek(Position-delta);
 }

long MemoryStream::size()
 {
    return Maxsize;
 }

bool MemoryStream::eof()
 {
  return Maxsize <= Position;
 }

long MemoryStream::pos()
 {
  return Position;
 }

int MemoryStream::read(char * dest,int maxcopy)
  {
       if ((maxcopy<=0)||(!dest)) return 0;

       long s = Maxsize - Position ;

       if (s > maxcopy) s = maxcopy;
       // s is now the number of bytes to read

       if ( s <= 0 ) return 0;

       char * c =(char*) Mem;
       if (!c) return 0;

       c += Position;

       memcpy(dest,(char*)c,s);
       Position += s;
       return s;
      };

int MemoryStream::write(const char * src,int maxcopy)
      {
       if ((maxcopy<=0)||(!src)) return 0;

       long s = Mem.size - Position ;
       if (maxcopy > s) //expand the buffer
         {
          long newsize = maxcopy + Position + 1024;
          Mem.Resize(newsize); //always get 1k extra
          s = Mem.size - Position;
          if (maxcopy > s) return 0;
         }

       if ( s <= 0 ) return 0;//something is wrong

       char * c = (char*)Mem;
       if (!c) return 0;

       c = c + Position;

       memcpy((char*)(c),src,maxcopy);
       Position += maxcopy;

       Maxsize = max(Position,Maxsize); //buffer terminator
       return maxcopy;
      };

//------------------------------------------------------------

ByteStream::ByteStream(char * _Buf,long _Size)
:Stream(ReadWrite)
{
 Maxsize = _Size ? _Size : strlen(_Buf);
 Buf=_Buf;
 Position=0;
}

ByteStream::ByteStream(const char * _Buf,long _Size)
:Stream(ReadOnly)
{
 Maxsize = _Size ? _Size : strlen(_Buf);
 Buf=(char*)_Buf;
 Position=0;
}

ByteStream::ByteStream(const uint16 * _Buf,long _Size)
:Stream(ReadOnly)
{
 Maxsize = _Size ? _Size : sizeof(uint16)*ucslen(_Buf);
 Buf = (char*)_Buf;
 Position = 0;
}

ByteStream::ByteStream(uint16 * _Buf,long _Size)
:Stream(ReadWrite)
{
 Maxsize = _Size ? _Size : sizeof(uint16)*ucslen(_Buf);
 Buf = (char*)_Buf;
 Position = 0;
}

ByteStream::~ByteStream()
{
}

bool ByteStream::eof() {return Maxsize <= Position;}

long ByteStream::seek(long offset)
{
 if (offset < 0) offset = Maxsize;
 return Position = min(offset,Maxsize);
}

long ByteStream::size()
{return Maxsize;}

long ByteStream::pos()
{return Position;}

long ByteStream::goforward(long delta)
{ return seek(delta+Position); }

long ByteStream::putback(long delta)
{ return seek(Position-delta); }

int ByteStream::read(char * dest,int maxcopy)
{
       if ((maxcopy<=0)||(!dest)) return 0;

       long s = Maxsize - Position ;

       if (s > maxcopy) s = maxcopy;
       // s is now the number of bytes to read

       if ( s <= 0 ) return 0;

       char * c = Buf;
       if (!c) return 0;

       c += Position;

       memcpy(dest,(char*)c,s);
       Position += s;
       return s;
}

int ByteStream::write(const char * src,int maxcopy)
{
   if (!(Type & IOWrite)) return 0;

       if ((maxcopy<=0)||(!src)) return 0;

       long s = Maxsize - Position ;
       if (maxcopy > s) maxcopy = s;

       char * c = Buf;
       if (!c) return 0;

       c += Position;

       memcpy((char*)c,src,maxcopy);
       Position += maxcopy;
       return maxcopy;
}

#endif
