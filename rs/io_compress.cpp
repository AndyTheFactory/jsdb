#include "rslib.h"
#pragma hdrstop
#include "zlib/zlib.h"

int32 ZCompress(Stream& in, Stream& out,int32* crc32)
{
 TChars source(24000u), dest(32000u); //0.1% larger + 8 bytes

// int32 size = in.size();

 z_stream stream;
 int err;
 stream.next_in = (Bytef*)(char*)source;
 stream.avail_in = 0;
 stream.next_out = (Bytef*)(char*)dest;
 stream.avail_out = (uInt)dest.size;
 stream.total_out = 0;
 stream.msg = NULL;
 stream.state=NULL;
 stream.zalloc =  NULL;//(alloc_func)(*zalloc);
 stream.zfree = NULL; //(free_func)(*zfree);
 stream.opaque = (voidpf)0;
 stream.adler = 1;
 err = deflateInit(&stream,Z_BEST_COMPRESSION);
 if (err != Z_OK) return err;

 stream.data_type = Z_BINARY;

 if (crc32) *crc32=1;
 int32 start;

 while (( stream.avail_in = in.read(source,source.size)) > 0)
 {
  stream.next_in = (Bytef*)(char*)source;

  do {
  stream.avail_out = dest.size;
  stream.next_out = (Bytef*)(char*)dest;

  start = stream.total_out;

  err = deflate(&stream,Z_SYNC_FLUSH);

  if (err != Z_OK && err != Z_STREAM_END) return 0;
  out.write(dest,stream.total_out - start);

  } while (stream.avail_out == 0);
 }

 start = stream.total_out;
 deflate(&stream,Z_FINISH);
 out.write(dest,stream.total_out - start);

 deflateEnd(&stream);

 if (crc32) *crc32 = stream.adler;
 return stream.total_out;
}
