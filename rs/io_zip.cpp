#include "rslib.h"
#pragma hdrstop
#include "zlib/zlib.h"

// zip uses Intel byte order.
#define SIZEZIPLOCALHEADER (0x1e)
#define BUFREADCOMMENT 1064

#ifdef IS_BIG_ENDIAN
#define SWAP16(i) i = ( ((i >> 8) & 0x00ff) | ((i << 8) & 0xff00) )
#define SWAP32(i) i = ( ((i << 24)& 0xff000000) | ((i << 8) & 0x00ff0000) | ((i >> 8) & 0x0000ff00) | ((i >> 24) & 0x000000ff))
#else
#define SWAP16(i)
#define SWAP32(i)
#endif

static count_t FindDirectory(Stream &s)
{
    TChars buf(BUFREADCOMMENT+4);
uint32 uSizeFile = s.size();
uint32 uBackRead;
uint32 uMaxBack=0xffff; /* maximum size of global comment */
uint32 uPosFound=0;
s.seek(-1);
if (uMaxBack > uSizeFile)
uMaxBack = uSizeFile;
    uBackRead = 4;
while (uBackRead<uMaxBack)
    {
        uint32 uReadSize,uReadPos ;
        int i;
        if (uBackRead+BUFREADCOMMENT > uMaxBack)
            uBackRead = uMaxBack;
        else
            uBackRead+=BUFREADCOMMENT;
        uReadPos = uSizeFile-uBackRead ;

        uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uSizeFile-uReadPos);
        s.seek(uReadPos);
        if (s.read(buf,uReadSize) != uReadSize) break;
      for (i=(int)uReadSize-3; (i--)>0;)
            if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
            {
                uPosFound = uReadPos+i;
                break;
            }
        if (uPosFound!=0)
            break;
    }
    return uPosFound;
}

ZipArchive::ZipArchive(Stream* s, bool ad)
:data(s),AutoDelete(ad)
{
 if (!data) throw xdb("Input stream missing");
 count_t start = FindDirectory(*data);
 if (!start)
 {
  zipPos = -1;
  dirSize = dirCount = dirPos = zipPos = 0;
  return;
 }

 data->seek(start);

 data->goforward(8); // skip the signature, disk numbers
 int16 number_entry_CD;

 data->readint16(dirCount);
 data->readint16(number_entry_CD);

 if (number_entry_CD != dirCount)
  throw xdb("Incomplete ZIP file");

 data->readint32(dirSize);
 data->readint32(dirPos);

 uint16 dirComment=0;
 data->readuint16(dirComment);
 comment.Resize(dirComment);
 data->read(comment,dirComment);

 zipPos = start - (dirPos + dirSize);
if (zipPos < 0)
 throw xdb("Invalid directory position");

 ReadDirectory();
}

ZipArchive::~ZipArchive()
{
 if (AutoDelete && data) delete data;
}

void ZipArchive::ReadDirectory()
{
 Items.Clear();

 count_t pos = dirPos + zipPos; // first directory entry
 data->seek(pos);

 for(int16 i=0; i< dirCount; i++)
 {
  uint32 Magic = 0;
  data->readuint32(Magic);
  if (Magic != 0x02014b50) break;
  ZipFile* d = new ZipFile;
  data->readuint16(d->version);
  data->readuint16(d->versionNeeded);
  data->readuint16(d->flag);
  data->readuint16(d->compressionMethod);

  data->readuint32(d->dosDate);              /* last mod file date in Dos fmt   4 bytes */
  data->readuint32(d->crc);                  /* crc-32                          4 bytes */
  data->readuint32(d->compressedSize);      /* compressed size                 4 bytes */
  data->readuint32(d->uncompressedSize);    /* uncompressed size               4 bytes */
  data->readuint16(d->sizeFilename);        /* filename length                 2 bytes */
  data->readuint16(d->sizeFileExtra);      /* extra field length              2 bytes */
  data->readuint16(d->sizeFileComment);    /* file comment length             2 bytes */

  data->readuint16(d->disk_num_start);       /* disk number start               2 bytes */
  data->readuint16(d->internal_fa);          /* internal file attributes        2 bytes */
  data->readuint32(d->external_fa);          /* external file attributes        4 bytes */
  data->readuint32(d->offset);

  uint32 uDate;
  uDate = (unsigned)(d->dosDate>>16);
  d->date.wDay = (unsigned)(uDate&0x1f) ;
  d->date.wMonth =  (unsigned)((((uDate)&0x1E0)/0x20)) ;
  d->date.wYear = (unsigned)(((uDate&0x0FE00)/0x0200)+1980) ;
  d->date.wHour = (unsigned) ((d->dosDate &0xF800)/0x800);
  d->date.wMinute =  (unsigned) ((d->dosDate&0x7E0)/0x20) ;
  d->date.wSecond =  (unsigned) (2*(d->dosDate&0x1f)) ;

  if (d->sizeFilename)
  {
   d->name.Resize(d->sizeFilename);
   data->read(d->name,d->sizeFilename);
  }
  if (d->sizeFileExtra)
   data->goforward(d->sizeFileExtra);
  if (d->sizeFileComment)
  {
   d->comment.Resize(d->sizeFilename);
   data->read(d->comment,d->sizeFilename);
  }
  Items.Add(d);
 }

}

size_t ZipArchive::Find(const char* filename)
{
 FOREACH(ZipFile* f, Items)
  if (f->name == filename) return i;
 DONEFOREACH
 return NOT_FOUND;
}

bool ZipArchive::Extract(size_t i, Stream& out)
{// first seek to the local header
 ZipFile* f = Items[i];
 if (!f) return false;

 data->seek(zipPos + f->offset);
 uint32 lData=0;
 uint16 sData=0;
 uint16 flags = 0;

 data->readuint32(lData);
 if (lData != 0x04034b50) return false;

 //version needed to extract       2 bytes
 data->readuint16(sData);
 //general purpose bit flag        2 bytes
 data->readuint16(flags);
 //compression method              2 bytes
 data->readuint16(sData);
 if (sData != f->compressionMethod) return false;
 //last mod file time              2 bytes
 //last mod file date              2 bytes
 data->readuint32(lData);
 //crc-32                          4 bytes
 data->readuint32(lData);
 if (lData != f->crc && (flags & 8)==0) return false;
 //compressed size                 4 bytes
 data->readuint32(lData);
 if (lData != f->compressedSize && (flags & 8)==0) return false;
 //uncompressed size               4 bytes
 data->readuint32(lData);
 if (lData != f->uncompressedSize && (flags & 8)==0) return false;
 //filename length                 2 bytes
 data->readuint16(sData);
 if (sData != f->sizeFilename) return false;
 //extra field length              2 bytes
 data->readuint16(sData);

 //skip the filename and extra data
 data->goforward(f->sizeFilename + sData);

 //we can only handle the "deflate" method
 if (f->compressionMethod && f->compressionMethod != 8) return false;

 if (!f->compressionMethod) //extract a STORE'd file, skip the CRC check
 {
  out.Append(*data,f->compressedSize);
  return true;
 }

 TChars cin(32768), cout(32768);
 z_stream zlib;
 zlib.total_out = 0;
 zlib.zalloc    = 0;
 zlib.zfree     = 0;
 zlib.opaque   = 0;
 if (inflateInit2(&zlib,-MAX_WBITS) != Z_OK) return false;
 zlib.avail_in  = 0;

 zlib.next_in = 0;
 zlib.avail_in = 0;
 int crc = 0;

 count_t bytesin = 0;

 while (zlib.total_out < f->uncompressedSize)
 {
  if (zlib.avail_in == 0)
  {
   if (bytesin < f->compressedSize)
   {
    uint32 x = data->read(cin,min(f->compressedSize-bytesin,(uint32)cin.size));
    if (x == 0) break;
    bytesin += x;
    zlib.next_in = (Bytef*)cin.buf;
    zlib.avail_in = x;
   }
  }

  zlib.next_out = (Bytef*)cout.buf;
  zlib.avail_out = cout.size;
  int flush=Z_SYNC_FLUSH;
  int err=inflate(&zlib,flush);
  int count = cout.size - zlib.avail_out;

  crc = crc32(crc,cout.buf,count);
  out.write(cout.buf,count);

  if (err==Z_STREAM_END)
   return (zlib.total_out == f->uncompressedSize);
  if (err!=Z_OK)
   return false;
 }
 return true;
}
