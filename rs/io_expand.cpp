#include "rslib.h"
#pragma hdrstop
#include "zlib/zlib.h"
//#include "rs/stream.h"

//int32 CRC32(int32 seed,const char * buf, int size)
//{return adler32(seed,(const Bytef*)buf,size);}

int32 ZExpand(Stream&in,Stream& out,int32* crc32)
{
 TChars source(24000u), dest(32000u); //0.1% larger + 8 bytes
 z_stream stream;
 int err;
 stream.next_in = NULL;
 stream.avail_in = 0;
 stream.next_out = (Bytef*)(char*)dest;
 stream.avail_out = (uInt)dest.size;
 stream.zalloc = (alloc_func)NULL;
 stream.zfree = (free_func)NULL;
 err = inflateInit(&stream);
 stream.adler = 1;
 stream.data_type = Z_BINARY;
 if (err != Z_OK) return err;

 while ((stream.avail_in = in.read(source,source.size)) > 0)
 {
  stream.next_in = (Bytef*)(char*)source;
  err = Z_OK;

  while (stream.avail_in)
   {
    stream.next_out = (Bytef*)(char*)dest;
    stream.avail_out = dest.size;
    int32 start = stream.total_out;
    err = inflate(&stream, Z_FINISH);
    out.write(dest,stream.total_out - start);

   if (err == Z_NEED_DICT) return stream.total_out;

    if (err == Z_BUF_ERROR && stream.avail_out == 0) continue;
      //out of space for writing, so continue

    if (err == Z_BUF_ERROR && stream.avail_in == 0) break;
      //need more input, so break out of this loop.

    if (err != Z_OK && err != Z_STREAM_END)
       {
         inflateEnd(&stream);
         return 0;
       }
   }
  if (err == Z_STREAM_ERROR) break;
 }
 if (crc32) *crc32 = stream.adler;
 err = inflateEnd(&stream);
 return stream.total_out;
}


