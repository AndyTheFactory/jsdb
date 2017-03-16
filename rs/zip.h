
struct ZipFile
 {
    TStr name, comment;
    uint32 offset; //! location of data
    uint16 version;              /* version made by                 2 bytes */
    uint16 versionNeeded;       /* version needed to extract       2 bytes */
    uint16 flag;                 /* general purpose bit flag        2 bytes */
    uint16 compressionMethod;   /* compression method              2 bytes */
    uint32 dosDate;              /* last mod file date in Dos fmt   4 bytes */
    uint32 crc;                  /* crc-32                          4 bytes */
    uint32 compressedSize;      /* compressed size                 4 bytes */
    uint32 uncompressedSize;    /* uncompressed size               4 bytes */
    uint16 sizeFilename;        /* filename length                 2 bytes */
    uint16 sizeFileExtra;      /* extra field length              2 bytes */
    uint16 sizeFileComment;    /* file comment length             2 bytes */

    uint16 disk_num_start;       /* disk number start               2 bytes */
    uint16 internal_fa;          /* internal file attributes        2 bytes */
    uint32 external_fa;          /* external file attributes        4 bytes */

    SYSTEMTIME date;
 };

class ZipArchive
{public:
 Stream* data;
 bool AutoDelete;

 TStr comment;

 //central directory
 int16 dirCount;
 int32 dirSize;
 int32 dirPos;
 int32 zipPos; //byte before zipfile, >0 if attached to EXE

 TList<ZipFile> Items;

 ZipArchive(Stream* s, bool AutoDelete);
 ~ZipArchive();
 bool Extract(size_t i, Stream& out);

 inline const char* operator[] (size_t i)
  {return (i < Items.Count()) ? (const char*)Items[i]->name : (const char*)0;}

 inline ZipFile* operator() (size_t i)
  {return  (i < Items.Count()) ? Items[i] : 0;}
  
 size_t Count() {return Items.Count();}

 /* exact search, including directories */
 size_t Find(const char* filename);

 protected:
 
 void ReadDirectory();

};
