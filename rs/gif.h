//web-related tools

class GIFFont
{
 public:
 struct pixsymbol
 {
	int height, width, descent;
	const char *data;
 };
 const char * name;
 int size;
 const char * map;
 int *lookup;
 pixsymbol *symbols;
 int Length(const char * text);
  GIFFont(const char* name, int size, const char* map, int* lookup,pixsymbol*s);
};

extern GIFFont Font_Helvetica9;

class GIFImage
{
 uint8 gif_hdr[(128*3+32)];
 TChars image;
 TChars imageInterlaced;
 bool WriteInterlaced;
 public:
 int imsize;
 int ih,iw;
 int nextcolor;

 GIFImage(int width, int height, bool transp = true);
 //background color is white by default. transparent color is 127.
 ~GIFImage() {}

 void setBGColor(int R, int G, int B);
 int Color(int R, int G, int B);

 void Line(int x1,int y1,int w,int h,int c);
 void Arc(double r, int xc, int yc, double angle, double off, int c);
 void Fill(int x, int y, int c);
 void PieSlice(int xc, int yc, double a1, double a2, double r, int c);
 void Print(GIFFont&Font,int x,int y, int color, char *text, int bg=-1);

 uint8& Pixel(int x, int y);

 void Write(Stream& out);
 void WriteRLE(Stream& out);

 bool Interlace();
 bool IsLocked();

 int Size() {return sizeof(gif_hdr) + imsize + (imsize/126)*2+2+4; }

 protected:
  int draw_char(GIFFont&Font,int ll, int y, int c, int ch, int bg);
};
