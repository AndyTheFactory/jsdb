#include "rslib.h"
#pragma hdrstop

#include <time.h>

#ifdef XP_UNIX

#include <sys/times.h>
extern "C" long GetTickCount()
{
    struct tms tm;
    return times(&tm);
}
#endif

extern const char * months[] =
{"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

void ParseDateTime(char* in, SYSTEMTIME& tm)
{
   static int days[] =
    { 31,29,31,30,31,30,31,31,30,31,30,31 };

   int i;
   int ss;
   int nn;
   int hh;
   int dd;
   int mm;
   int yy;
   int dhh;
   int dnn;
   char *p;
   memset(&tm,0,sizeof(tm));

   p = strchr(in, ',');       // skip day if any

   if (p != NULL)
   {
      p++;    // parse after the comma
   }
   else
   {
      p = in;  // parse from the beginning
   }
   p = strtok(p, " ");  // get date
   if (!p) return;

   dd = atoi(p);

   p = strtok(NULL, " ");  // get month
   if (!p) return;

   for (i = 0; i < 12; i++)
   {
      if (!strcasecmp(p, months[i]))
      {
         mm = i+1;   // mm is in [1, 12]

         break;
      }
   }
   p = strtok(NULL, " ");  // get year
   if (!p) return;

   yy = atoi(p);

   if (yy < 100)   // eg, yy = 82
   {
      yy += 1900;  // 82 ==> 1982
   }
   p = strtok(NULL, " ");  // get time
   if (!p) return;

   if (sscanf(p, "%d:%d:%d", &hh, &nn, &ss) != 3)
   {
      sscanf(p, "%d:%d", &hh, &nn);

      ss = 0;
   }
   p = strtok(NULL, " ");  // get time zone
   if (!p) return;

   dhh = 0;  // by default, no adjustment
   dnn = 0;

   if (!strcasecmp(p, "UT") || !strcasecmp(p, "GMT"))
   {
      ;
   }
   else if (!strcasecmp(p, "EST"))
   {
      dhh = -5;
   }
   else if (!strcasecmp(p, "EDT"))
   {
      dhh = -4;
   }
   else if (!strcasecmp(p, "CST"))
   {
      dhh = -6;
   }
   else if (!strcasecmp(p, "CDT"))
   {
      dhh = -5;
   }
   else if (!strcasecmp(p, "MST"))
   {
      dhh = -6;
   }
   else if (!strcasecmp(p, "MDT"))
   {
      dhh = -7;
   }
   else if (!strcasecmp(p, "PST"))
   {
      dhh = -8;
   }
   else if (!strcasecmp(p, "PDT"))
   {
      dhh = -7;
   }
   else if (!strcasecmp(p, "Z"))
   {
      dhh = 0;
   }
   else if (!strcasecmp(p, "A"))
   {
      dhh = -1;
   }
   else if (!strcasecmp(p, "M"))
   {
      dhh = -12;
   }
   else if (!strcasecmp(p, "N"))
   {
      dhh = 1;
   }
   else if (!strcasecmp(p, "Y"))
   {
      dhh = 12;
   }
   else
   {
      int t;
      t = atoi(p);
//      sscanf(p, "%d", &t);   // eg, +0800 or -0400

      if (t >= 0)
      {
         dhh = t/100;
         dnn = t%100;
      }
      else
      {
         dhh = -(-t/100);
         dnn = -(-t%100);
      }

   }

   tm.wDayOfWeek =0;
   tm.wMilliseconds =0;
   tm.wSecond = (uint16) ss;
   tm.wMinute = (uint16)(i = nn + dnn) % 60;
   tm.wHour = (uint16)(i = hh + dhh + i / 60) % 24;
   tm.wDay = (uint16)(i = dd + i / 24);
   tm.wMonth = (uint16)(i = mm + i / days[mm-1]);
   tm.wYear = (uint16)(yy + i / 12);

   if (tm.wMonth == 2 && tm.wDay == 29)
    {
     //is it not a leap year?
     if ( ((tm.wYear % 4) || (tm.wYear % 100 ==0))
           && (tm.wYear % 400))
       {tm.wMonth = 3; tm.wDay = 1;}
    }
}

bool WriteDate(TChars &out,unsigned y, unsigned m, unsigned d)
{// 06-JUN-19999
 if (out.size < 12) out.Resize(12);
 if (m > 12 || m < 1) return false;
 if (d > 31) return false;
 sprintf(out,"%2.2d-%3s-%4.4d",d,months[m-1],y);
 return true;
}

void GetDateTime(char* Date,char * Time,int type,bool GMT)
 {
#ifdef XP_WIN
  SYSTEMTIME tb;
  if (GMT) GetSystemTime(&tb);
  else  GetLocalTime(&tb);
#else
  time_t timer;
   time(&timer);
   struct tm *tb;
  if (GMT)  tb = gmtime(&timer);
  else tb = localtime(&timer);
#endif

  switch (type)
  {
case 5:
   sprintf(Date,"%04d%02d%02d%02d%02d%02d",
#ifdef XP_WIN
           tb.wYear,tb.wMonth,tb.wDay,
           tb.wHour,tb.wMinute,tb.wSecond);
#else
           tb->tm_year+1900,tb->tm_mon+1,tb->tm_mday,
           tb->tm_hour,tb->tm_min,tb->tm_sec);

#endif
           break;
case 4:
#ifdef XP_WIN
 {
  int32 a  = (tb.wMonth - 1); a = a << 4;
        a |= (tb.wDay - 1);   a = a << 5;
        a |= (tb.wHour);      a = a << 5;
        a |= (tb.wMinute);    a = a << 6;
        a |= (tb.wSecond);

   char c[8],d[8];

   sprintf(Date,"%d:%s%s",tb.wYear,IntToStr32(a,c),IntToStr32(GetTickCount(),d));
   break;
 }
#else
 {
  int32 a  = (tb->tm_mon); a = a << 4;
        a |= (tb->tm_mday - 1); a = a << 5;
        a |= (tb->tm_hour);   a = a << 5;
        a |= (tb->tm_min);    a = a << 6;
        a |= (tb->tm_sec);
   char c[8],d[8];

   sprintf(Date,"%d:%s%s",tb->tm_year+1900,IntToStr32(a,c),IntToStr32(clock(),d));
   break;
 }
#endif

case 3:
   sprintf(Date,"%02d %s %d %02d:%02d:%02d",
#ifdef XP_WIN
           tb.wDay,months[tb.wMonth-1],tb.wYear,
           tb.wHour,tb.wMinute,tb.wSecond);
#else
           tb->tm_mday,months[tb->tm_mon],tb->tm_year+1900,
           tb->tm_hour,tb->tm_min,tb->tm_sec);
#endif
           break;

case 2:
   if (Date) sprintf(Date,"%2.2d/%2.2d/%4.4d",  //world dmy
#ifdef XP_WIN
           tb.wDay,tb.wMonth,tb.wYear);
#else
           tb->tm_mday,tb->tm_mon+1,tb->tm_year+1900);
#endif

   if (Time) sprintf(Time,"%2.2d:%2.2d:%2.2d",
#ifdef XP_WIN
           tb.wHour,tb.wMinute,tb.wSecond);
#else
           tb->tm_hour,tb->tm_min,tb->tm_sec);
#endif
           break;
case 1:
   if (Date) sprintf(Date,"%2.2d/%2.2d/%4.4d", //US mdy
#ifdef XP_WIN
           tb.wMonth,tb.wDay,tb.wYear);
#else
           tb->tm_mon+1,tb->tm_mday,tb->tm_year+1900);
#endif

   if (Time) sprintf(Time,"%2.2d:%2.2d:%2.2d",
#ifdef XP_WIN
           tb.wHour,tb.wMinute,tb.wSecond);
#else
           tb->tm_hour,tb->tm_min,tb->tm_sec);
#endif
           break;

default:
   if (Date) sprintf(Date,"%4.4d%2.2d%2.2d",
#ifdef XP_WIN
           tb.wYear,tb.wMonth,tb.wDay);
#else
           tb->tm_year+1900,tb->tm_mon+1,tb->tm_mday);
#endif
   if (Time) sprintf(Time,"%2.2d%2.2d%2.2d",
#ifdef XP_WIN
           tb.wHour,tb.wMinute,tb.wSecond);
#else
           tb->tm_hour,tb->tm_min,tb->tm_sec);
#endif
  }
 }

