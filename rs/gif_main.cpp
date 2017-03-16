#include "rslib.h"
#pragma hdrstop
#include <math.h>

//#define PIXEL(a,b) (uint8)image.buf[(int)(labs(a)%iw)+(int)(labs(b)%ih)*iw]

#define IMAGEBITS 7
#define MAXCOLORS 128

#define INTERLACEBYTE 414

#define rint(a) (int)((a)+0.5)

/* MAXCOLORS is 2^(IMAGEBITS) */

#if ((IMAGEBITS!=7)&&(IMAGEBITS!=3))
#error Bit size is bad
#endif


uint8& GIFImage::Pixel(int x, int y)
{
 if (x < 0 || y < 0)
   return ((uint8*)image.buf)[0];

 if (x < iw && y < ih)
   return ((uint8*)image.buf)[(int)(x%iw)+((y%ih)*iw)];

 return ((uint8*)image.buf)[0];
}

void GIFImage::Line(int x1,int y1,int w,int h,int c)
{
int i;
double t,l;

WriteInterlaced = false;

if (w==0) {
    if (h<0) {
        h=h*-1;
        y1-=h;
    }
    for (i=0;i<h;i++)
        Pixel(x1,y1+i)= (uint8)c;
    return;
}

if (h==0) {
    if (w<0) {
        w=w*-1;
        x1-=w;
    }
    for (i=0;i<w;i++)
        Pixel(x1+i,y1)=(uint8)c;
    return;
}

l=1.0/sqrt((double)(h*h+w*w));

for (t=0;t<=1;t+=l) {
    Pixel(int(x1+t*w),int(y1+t*h))=(uint8)c;
}
return;
}

bool GIFImage::Interlace()
{
imageInterlaced.Resize(imsize);
int i,l;
l=0;
i=0;
while (i<ih) {
memcpy((char*)imageInterlaced+(l*iw),image+(i*iw),iw);
i+=8;
l++;
}

i=4;
while (i<ih) {
memcpy((char*)imageInterlaced+(l*iw),image+(i*iw),iw);
i+=8;
l++;
}

i=2;
while (i<ih) {
memcpy((char*)imageInterlaced+(l*iw),image+(i*iw),iw);
i+=4;
l++;
}

i=1;
while (i<ih) {
memcpy((char*)imageInterlaced+(l*iw),image+(i*iw),iw);
i+=2;
l++;
}
gif_hdr[INTERLACEBYTE]=64;
WriteInterlaced = true;
return true;
}

#define rint(a) (int)((a)+0.5)
#define sqr(a) ((a)*(a))

void GIFImage::Arc(double r, int xc, int yc, double angle, double off, int c) {
double a,l;
int x,y;
int i,j;

l=5/(r);
i=rint(r*cos(off));
j=rint(-r*sin(off));
for (a=l;a<angle;a+=l) {
    x=rint(r*cos(off+a));
    y=rint(-r*sin(off+a));
    Line(i+xc,j+yc,x-i,y-j,c);
    i=x;
    j=y;
}
x=rint(r*cos(off+angle));
y=rint(-r*sin(off+angle));
Line(i+xc,j+yc,x-i,y-j,c);
     WriteInterlaced = false;
return;
}

void GIFImage::Fill(int x, int y, int c) {
if ((x < 0) || (x >= iw) || (y < 0) || (y >= ih)) return;
if (Pixel(x,y) != 0) return;
Pixel(x,y)= (uint8)c;
Fill(x+1,y,c);
Fill(x,y+1,c);
Fill(x-1,y,c);
Fill(x,y-1,c);
WriteInterlaced = false;
}

void GIFImage::PieSlice(int xc, int yc, double a1, double a2, double r, int c) {
int x,y;
int a,b;
x=rint(r*cos(a1));
y=rint(r*sin(-a1));
Line(xc,yc,x,y,c);
a=rint(r*cos(a2));
b=rint(r*sin(-a2));
Line(xc,yc,a,b,c);


if (a2>a1) {
    Arc(r,xc,yc,(a2-a1),a1,c);
} else {
    Arc(r,xc,yc,(a1-a2),a2,c);
}
Fill(xc+(x+a)/2,yc+(y+b)/2,c);
WriteInterlaced = false;
return;
}

void GIFImage::WriteRLE(Stream& out)
{

 out.write(gif_hdr,sizeof(gif_hdr));

int i=0;
int stop;
unsigned char data[150],*buff;
int j,k,n=0;

char* CurrentImage = (WriteInterlaced) ? imageInterlaced : image;

stop=imsize;
buff=data+1;

#define START 129

#define SENDRLEDATA data[0]=n+1; data[n+1]=128; if (!out.write(data,n+2)) return; n=0;

while (1) {
    k=CurrentImage[i];
    for (j=i+1;j<stop;j++) {
        if (CurrentImage[j]!=k) break;
    }
    j--; /* image([i,j])=k */
    if (j-i>5) {
        if (n>0) { SENDRLEDATA }

        buff[0]=k;
        n=1;
        i++;

        while (j-i>n) {
            buff[n]=START+n;
            i+=n+1;
            n++;
            if (n==120) {
                /* this is untested */
                if (j-i<=120) {
                    buff[120]=START+(j-i);
                    n=121;
                    i=j+1;
                }
                break;
            }
        }

        SENDRLEDATA
    } else {
        /* replace with memset when bored */
        for (i=i;i<=j;i++) {
            buff[n]=k;
            n++;
            if (n==120) {
                SENDRLEDATA
            }
        }
    }
    if (i==stop) break;
}

if (n>0) {
    SENDRLEDATA
}

out.write("\x01\x81\x00\x3b",4); // 1, EO Data , EO Block , EO Gif
}

void GIFImage::Write(Stream& out)
{
char* CurrentImage = (WriteInterlaced) ? imageInterlaced : image;

 out.write(gif_hdr,sizeof(gif_hdr));
 int i=0;

 while (imsize>i) {
  out.put((imsize-i>126) ? 127 : (imsize-i+1));
  out.write(CurrentImage+i, (imsize-i>126) ? 126 : (imsize-i));
  out.put(128); /* clear dict */
  i += 126; /* I *think* this could be 127 without any problems from the dict */
 }

 out.write("\x01\x81\x00\x3b",4); // 1, EO Data , EO Block , EO Gif

}

void GIFImage::setBGColor(int R, int G, int B)
{

 gif_hdr[13] = (uint8)R; //color zero: white
 gif_hdr[14] = (uint8)G;
 gif_hdr[15] = (uint8)B;

}

int GIFImage::Color(int R, int G, int B)
{
 for (int i=1; i < nextcolor; i++)
 {
  if (gif_hdr[13+i*3] == R &&
      gif_hdr[14+i*3] == G &&
      gif_hdr[15+i*3] == B)
   return i;
 }

 if (nextcolor > 127) return 0;

 gif_hdr[13+3*nextcolor] = R;
 gif_hdr[14+3*nextcolor] = G;
 gif_hdr[15+3*nextcolor] = B;
 int x = nextcolor;
 nextcolor++;
 return x;
}

void GIFImage::Print(GIFFont& Font, int x,int y, int c, char *text, int bg)
{
int i;
int ll,n;
ll=x;

for (i=0;text[i];i++) {
    n=draw_char(Font,ll,y,c,text[i],bg);
    /*
     * just skip chars that can't be drawn;
    if (n==-1) break;
    */
    if (n!=-1) ll+=n;
}
WriteInterlaced = false;
}

#define DEFAULTCHAR ((int)'.')

int GIFImage::draw_char(GIFFont&Font,int ll, int y, int c, int ch, int bg) {
int height,desc,wid;
int n;
int i,j;
if ((n=Font.lookup[ch])==-1) {
    if ((n=Font.lookup[DEFAULTCHAR])==-1) return -1;
}
height=Font.symbols[n].height;
wid=Font.symbols[n].width;
desc=Font.symbols[n].descent;

y+=desc;

if ((y>ih)||(y-height<0)||(ll<0)||(ll+wid>iw)) return -1; /* later clip */

for (j=0;j<height;j++)
 for (i=0;i<wid;i++)
 {
            if (Font.symbols[n].data[j*wid+i]=='X')
                Pixel(ll+i,y-height+j)=c;
            else if (bg > -1) Pixel(ll+i,y-height+j)=bg;
 }

return wid;
}

GIFImage::GIFImage(int width, int height, bool transp):
 image(width*height), iw(width), ih(height)
{
 WriteInterlaced = false;
 memset(image.buf,0,image.size);
 memset(gif_hdr,0,sizeof(gif_hdr));

 imsize = width*height;
 nextcolor = 1;

 gif_hdr[0]='G';
 gif_hdr[1]='I';
 gif_hdr[2]='F';
 gif_hdr[3]='8';
 gif_hdr[4]='9';
 gif_hdr[5]='a';

 gif_hdr[6]=iw%256;
 gif_hdr[7]=iw/256;
 gif_hdr[8]=ih%256;
 gif_hdr[9]=ih/256;
 gif_hdr[10]= 230; //packed bits. see GIF89a for definition
 /*gif_hdr[10]= 6 | (6<<4) | 128;*/
 gif_hdr[11]=0; //background color is the first one
 gif_hdr[12]=0; //aspect ratio ignored


 gif_hdr[13]=255; //color zero: white
 gif_hdr[14]=255;
 gif_hdr[15]=255;

 // 13 .. 396 are the mostly empty color table

 //graphic control extension
 gif_hdr[397]=0x21;
 gif_hdr[398]=0xF9; /* transparency extension */
 gif_hdr[399]=4;

 if (transp) {
  gif_hdr[400]=1; /* set to 0 to turn off transp. */
  gif_hdr[403]=127;
 } else {
  gif_hdr[400]=0;
  gif_hdr[403]=0;
 }

 gif_hdr[401]=0;
 gif_hdr[402]=0;
 gif_hdr[404]=0;

 //image start
 gif_hdr[405]=0x2C; /* start of img */

 gif_hdr[406]=0;
 gif_hdr[407]=0;
 gif_hdr[408]=0;
 gif_hdr[409]=0;
 gif_hdr[410]=iw%256;
 gif_hdr[411]=iw/256;
 gif_hdr[412]=ih%256;
 gif_hdr[413]=ih/256;
 gif_hdr[414]=0; /* interlace here - bit 6 */
 gif_hdr[415]=7;

}

