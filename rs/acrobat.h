class PDFWriter
{
 public:
 int32 CurrentObject, CurrentPage, FirstObject;
 int32 Catalog, Outlines,Pages,Procset;
 int32 PageWidth, PageHeight; // in points
 int32 Info;
// int32 start;

// TDRWPrinter Target; //for font widths

 // open the stream in binary mode, use linefeeds

 PDFWriter(Stream& outfile); //,TDRWPrinter &Printer=TDRWPrinter());
 ~PDFWriter(); //write the xref table

 int32 NextObjectNumber() {return CurrentObject++;}

 struct objid {
               int32 num, gen, offset;
               MemoryStream data;
               int Compare(objid&o) {return num-o.num;}
               objid() {num=gen=offset=0;}
              };

 struct font { int32 num; TStr face, name; };

 TRow<objid> ObjectIds;
 TRow<font> Fonts; //fonts used in the whole document current page

 TRow<objid> PageContents; //autodelete off
 TRow<objid> PageList;     //autodelete off

 char  * AddFont(const char * face);
 objid * AddObject(int32 num = 0);
 objid * GetObject(int32 id);

 int32 NewPage(int32 width, int32 height);
 void InsertGraphics(Stream& s,int32 len=LONG_MAX);
  //be sure to rewind the input stream
  
 void write(const char * text);
 void write(const char * text,size_t len);
 PDFWriter& operator << (const char * t) {write(t); return *this;}

protected:
  void DonePage();
  void Finish();
  Stream* outbuf;
  Stream* CurrentStream;
  int32 pos;
};
