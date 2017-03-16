
//io_b64
int32 b64encode(Stream& in, Stream& out);

int32 qpdecode(Stream& In, Stream& h);

int32 b64decode(Stream& In, Stream& h);

//io_zlib
int32 ZCompress(Stream& in,Stream& out,int32* crc32=NULL);

int32 ZExpand(Stream& in,Stream& out,int32* crc32=NULL);

void HTMLEscape(Stream& out, const uint16* s);

void HTMLEscape(Stream& out, const char* s);

void HTMLUnEscape(char* s);

/// escapes \r, \n, \t, \\ by default. For other characters, list in 'extra'.
void CEscape(Stream& out, const char* s,const char* extra=0);

void CUnEscape(Stream& out, const char* s);

/// if (XMLonly) only encodes those characters that bother an XML parser.
void URLEncodeXML(TStr& out, const char* in,size_t len=0);
void URLEncodeURL(TStr& out, const char* in,size_t len=0);
void URLEncode1(WStr& out, const char* in,size_t len=0);

void URLDecode(char * str,bool XMLonly = false);

/* non-seekable read-only stream */
class StreamProcessor : public Stream
{public:
 Stream* data;
 TChars buffer;
 int length;

 StreamProcessor(Stream& d);
 ~StreamProcessor();
 int read(char * dest,int maxcopy);
 bool eof() {return length == 0 && (data?data->eof():true);}
 const char * filename() {return data->filename();}
 void rewind() {length=0; data->rewind();}

 /// translate input data, place result in out
 virtual bool translate(Stream& in, size_t& length, TChars& out) = 0;

};
