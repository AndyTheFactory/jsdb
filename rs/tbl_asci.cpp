#include "rslib.h"
#pragma hdrstop

#ifndef TBL_NO_ASCII

#define CACHESIZE 16384

#define SETFLAG(i,flag) (i) = (ETableStyle) ((int)i | (int)(flag))
#define CLEARFLAG(i,flag) (i) = (ETableStyle) ((int)i & ~(int)(flag))

SpreadsheetTable::TData::TData(count_t x, size_t y, const char * s):
 Loc(x,y),data(s && *s?AllocStr(s):0)
 {
 }

SpreadsheetTable::TData::~TData()
 {
  if (data) FreeStr(data);
 }
void SpreadsheetTable::TData::Set(const char* s)
{
  if (data) FreeStr(data);
  data=(s?AllocStr(s):0);

}
SpreadsheetTable::SpreadsheetTable()
 : DataTable(),Items(1000,1000,1000)
 {
  delimiter = 0;
  StreamData = NULL;
  TableType = ETableASCII;
  SETFLAG(Style,AllowModify|AllowAddRow|AllowAddCol|AllowDelRow|AllowDelCol);
  SETFLAG(Style,HasSave);
  Type = Ascii;
  Size.I = Size.J = 0;
  lastsort = 0;
  lastsearch=0;
 }


SpreadsheetTable::SpreadsheetTable(Stream* str, bool AD)
 : DataTable(),Items(100,100,100)
{
 if (!str) throw xdb("No data stream specified");
 delimiter = 0;
 StreamData=str;
 AutoDeleteStreamData=AD;

 TableType = ETableASCII;
 Filename = str->filename();
 CLEARFLAG(Style,AllowModify|AllowAddRow|AllowAddCol|AllowDelRow|AllowDelCol|HasSave);
 Type = Ascii;
 Size.I = Size.J = 0;
 lastsort = 0;
 lastsearch = 0;

 if (!LoadHeader(*StreamData,true)) throw xdb("Error in the data stream");
}

SpreadsheetTable::SpreadsheetTable(const char * filename,bool FirstRowAsTitles)
 : DataTable(),Items(1000,1000,1000)
{
  delimiter = 0;
  StreamData = NULL;
  TableType = ETableASCII;
  Filename = filename;
  SETFLAG(Style, AllowModify|AllowAddRow|AllowAddCol|AllowDelRow|AllowDelCol);
  IsModified = false;
  Type = Ascii;
  Size.I = Size.J = 0;
  lastsort = 0;
  lastsearch=0;

  if (*Filename && FileExists(Filename))
   {
   try{
    FileStream in(Filename);
    LoadFromStream(in,FirstRowAsTitles);
    SETFLAG(Style, HasSave);
    } catch (...){};
   }
  else
   {
    CLEARFLAG(Style, HasSave);
   }
}


SpreadsheetTable::~SpreadsheetTable()
 {
  if (StreamData) if (AutoDeleteStreamData) delete StreamData;
 }

void SpreadsheetTable::GetConnectString(TStr& x)
{
 if (*Filename) x = TStr("ascii://",Filename);
}

/** 7/1/03 added BSearch and lastsort */
size_t SpreadsheetTable::BSearch(TableLocation& Loc,size_t low, size_t high)
{
  if (low >= high) return high;

  if (low + 1 == high) return low;

  size_t middle = (low + high) / 2;
  TData* d = Items[middle];
  if (!d) return low;

  int i = Loc.Compare(d->Loc);
  if (i == 0) return middle;
  if (i < 0) return BSearch(Loc,low,middle-1);
  return BSearch(Loc,middle+1,high);
}

SpreadsheetTable::TData* SpreadsheetTable::Get(count_t x, size_t y,size_t & index)
{//optimize for retrieving values forwards.
 TData * d;
 size_t i, i2, imax = Items.Count();
 register size_t stop;
 TableLocation Loc(x,y);

 int oldsearch = lastsearch;

 if (lastsearch > imax) lastsearch = 0;
 if (lastsort > imax) lastsort = 0;

 // probably already there

 stop = min(lastsearch+Size.J+1,imax);
 for (i=lastsearch; i<stop; i++)
   {
    d = Items[i];
    if (d)
    if ( d->Loc.I == x && d->Loc.J == y )
       {
        lastsearch = index = i;
        return d;
       }
   }

 // nope. try a binary search.
 if (lastsort)
 {
 i2 = BSearch(Loc,0,lastsort);
 stop = min(i2 + Size.J +1, imax);
 for (i=i2; i< stop; i++)
   {
    d = Items[i];
    if ( d->Loc.I == x && d->Loc.J == y )
       {
        lastsearch = index = i;
        return d;
       }
   }
 }

 //didn't find it. look in the unsorted area
 //if (lastsort >= stop)
 for (i=max(lastsort,stop); i<imax; i++)
   {
    d = Items[i];
    if ( d->Loc.I == x && d->Loc.J == y )
       {
        lastsearch = index = i;
        return d;
       }
   }

 //exhaustive search of the sorted area. this should never run.
 /*
   for (i = 0; i < lastsort; i++)
    {
     if ( d->Loc.I == x && d->Loc.J == y )
        {
         lastsearch = index = i;
         return d;
        }
    }
  */
 lastsearch = oldsearch;
 return 0;
}

bool SpreadsheetTable::Add(count_t x, size_t y, const char * s)
 {
   if (Size.I < x) Size.I = x;
   if (Size.J < y) Size.J = y;

   if (x == 0 && y > 0)
    {
     Headers.Add(s);
     return true;
    }

   TData * d = new TData(x,y,s);

   bool sorted = (Items.Count() == lastsort+1);
   TData * l = Items.Last();

   IsModified = IsChanged = true;
   size_t index = Items.Add(d);

   if (l && sorted)
    if (l->Loc.Compare(d->Loc) < 0 )
      lastsort = index;

   return true;
 }

count_t SpreadsheetTable::AddRow(TNameValueList& pl)
 {
  size_t jmax = ColumnCount();
  size_t i = Size.I + 1;

  if (Style & AllowAddRow)
   {
     if (pl.Count())
     {
      for (size_t j=0; j< jmax; j++)
       Add(i,j+1,pl(Headers[j]));
//      SetRow(Size.I+1,pl);
     }
     else
      Add(i,0," ");
   }

  return Size.I;
 }

size_t SpreadsheetTable::AddCol(const char * x)
 {
  if (Style & AllowAddCol) Add(0,Size.J+1,x);
  IsModified = IsChanged = true;
  return Size.J;
 }

bool SpreadsheetTable::DelRow(count_t index)
 {
  if (!(Style&AllowDelRow)) return false;
  TData * d;
  FOREACHBACK(d,Items)
   if (d->Loc.I == index)
    {
     Items.Destroy(i,true);
     if (i <= lastsort) lastsort --;
    }
   else if (d->Loc.I > index) d->Loc.I --;
  DONEFOREACH
  IsModified = IsChanged = true;
  Size.I--;
  return true;
 }

bool SpreadsheetTable::DelCol(size_t index)
 {
  if (!(Style&AllowDelCol)) return false;
  TData* d;
  FOREACHBACK(d,Items)
   if (d->Loc.J == index)
    {
     Items.Destroy(i,true);
     if (i <= lastsort) lastsort --;
    }
   else if (d->Loc.J > index) d->Loc.J --;
  DONEFOREACH
  IsModified = IsChanged = true;
  Size.J--;
  return true;
 }

const char * SpreadsheetTable::GetDataC(count_t i, size_t j)
 {
  if ((j == 0 && i == 0)) return NULL;

  if (i == 0) return Headers[j-1];

  if (StreamData)
  {
   if (j == 0 && i <= Size.I) return "";

   if (j == 0 || j == 1 && i == (Size.I + 1)) //load the next row
   {
    count_t row = i;
    IsChanged = true;
    //clear the cache of data we don't need.
    if (Items.Count() > CACHESIZE)
    { //clear the first few expired records
     int size = min(Size.J * 40, CACHESIZE-(Size.J*10));
     if (size<0) size = Size.J;
     Items.Purge(0,size);
    }
    if (!LoadASCIILine(row,*StreamData)) return NULL;
    if (j == 0) return "";
   }
  }

  if (j > Size.J || i > Size.I) return NULL;

  if (j == 0) return ""; //normal, non-deleted row.

  size_t index=0;
  TData * d = Get(i,j,index);
  if (!d)
   {
    return ""; //we own the record, but it's blank. no biggie.
   }
  return d->Get();
 }

bool SpreadsheetTable::SetDataC(count_t i, size_t j, const char * c)
 {
  if (j == 0)
  {
   if (i == 0 || i > Size.I) return false;
   if (c && *c == 'D') return DelRow(i);
   return false;
  }

  size_t index;
  TData * d = NULL;
  if (i <= Size.I && j <= Size.J)
   d = Get(i,j,index);

  if (c) // (&& *c) tried to leave out nulls, but that causes more trouble.
  //instead, modify TData to not make zero-length allocs.
  {
   if (d)
    {
     d->Set(c);
     IsModified = IsChanged = true;
     return true;
    }
   else
    {
     Add(i,j,c);
     return true;
    }
  }
  else
  {
   if (d)
    {
     Items.Destroy(index,true);
     IsModified = IsChanged = true;
     return true;
    }
   else
   {
    return false;
   }
  }
 }

void SpreadsheetTable::FreeMemory()
{
 Items.Clear();
 lastsort=0;
}

bool SpreadsheetTable::Optimize()
 {
  //Size.I = Size.J = 0;
  TData * d;
  if (!Items.Count())
   return true;

  bool change = false;
  FOREACHBACK(d,Items)

   if (d)
    {
      if (Size.I < d->Loc.I) {change = true; Size.I = d->Loc.I;}
      if (Size.J < d->Loc.J) {change = true; Size.J = d->Loc.J;}
    }

  DONEFOREACH

  if (!change && lastsort+1 == Items.Count())
  {
   return true;
  }

  // sort here, but modify rowsorter code to handle null items;
  SORTLIST(TData,Items);

  // the whole list is sorted now!
  if (Items.Count())
    lastsort = Items.Count() - 1;
  else
    lastsort = 0;
  return true;
 }

bool SpreadsheetTable::Save()
 {
  Optimize();
  const char * ext = GetExtension(Filename);
  if (!strcasecmp(ext,"HTM") || !strcasecmp(ext,"HTML")) Type = Html;
  else if (!strcasecmp(ext,"IDS")) Type = Binary; //indexed data sheet
  else Type = Ascii;

  const char * delim="\t";

  if (Type == Ascii && (*ext == 'c' || *ext == 'C')) delim=",";
   //CSV gets commas, all else tabs

  Stream::TOpenMode Mode =  Type ? Stream::OMText : Stream::OMBinary;

try{
  FileStream out(Filename, Mode, Stream::WriteOnly);
  SaveToStream(out,delim);
} catch (...) {return false;}

  IsModified = false;
  return true;
 }

void WriteASCIIData(const char * c,const char * delim,Stream& out)
 {
  if (!strstr(c,delim) && !strchr(c,'\"')) {out<<c; return;}

  out << "\"";
  while (*c)
  {
   if (*c == '\"') out << "\"";
   out.write(c,1);
   c++;
  }
  out << "\"";
 }

bool SpreadsheetTable::SaveToStream(Stream & out,const char *delim,bool WriteTitles)
{
 TData * d;
 size_t i;
 bool titles = (ColumnTitle(1) != 0);

 if (delim == 0 || delim[0] == 0) delim = "\t";

 if (Type == Ascii)
  {
   // out.write("#",1);
    if (titles && WriteTitles)
    {
     for (count_t y=1; y <= Size.J; y++)
      {
       const char * c = ColumnTitle(y);
       if (c) WriteASCIIData(c,delim,out);
       if (y == Size.J)
        out << "\n" ;
      else
        out << delim;
      }
    }
    for (count_t x = 1; x <= Size.I; x++)
    for (count_t y = 1; y <= Size.J; y++)
    {
      d = Get(x,y,i);
      if (d) WriteASCIIData(d->Get(),delim,out); //out << d->Data;

      if (y == Size.J)
        out << "\n" ;
      else
        out << delim;
    }
  }
  else if (Type == Html)
  {
   if (WriteTitles) out << "<HTML>\n<TABLE>\n<TR>";
   if (titles && WriteTitles)
    {
     for (count_t y = 1; y <= Size.J; y++)
     {
      out << "<TH>" << ColumnTitle(y);
     }
    }
    for (count_t x = 1; x <= Size.I; x++)
    {
    out << "\n<TR>";
    for (count_t y = 1; y <= Size.J; y++)
    {
      out << "<TD>";
      d = Get(x,y,i);
      if (d) out << d->Get();
      else out << "<br>";
    }
    }
   if (WriteTitles) out << "\n</TABLE>\n</HTML>\n";
  }
  else //type is binary
  {
   char c = 26;
   out.write(&c,1);
   FOREACHPTR(d,Items)
      out.write(&(d->Loc),sizeof(TableLocation));
      out.writestr(d->Get());
      out.write("",1);
   DONEFOREACH
  }
  return true;
}

bool SpreadsheetTable::LoadFromStream(Stream & in,bool titles)
{

 if (!LoadHeader(in,titles)) return false;

 if (Type == Ascii)
  {
   count_t x = Size.I+1;
   while (1)
    {
     if (!LoadASCIILine(x,in)) break;
     x++;
    }
  }

  IsModified = false;
  IsChanged = true;
//  Optimize(); //sort the responses
  return true;

}

bool SpreadsheetTable::LoadHeader(Stream & in,bool titles)
{
 char firstchar;
 in.read(&firstchar,1);
 switch (firstchar)
  {
   case 26: Type = Binary; break;
   case '<': Type = Html; break;
   case '#': titles = true; Type=Ascii; break;
   default:  Type=Ascii; in.putback(1);
  }

 if (Type == Html)
   return false;


 if (Type == Ascii)
  {
   if (titles) return LoadASCIILine(0,in);
   return true;
  }

 if (Type == Binary)
  {
   MemoryStream Str;
   TableLocation Pos;
   while (in.read(&Pos,sizeof(TableLocation))==sizeof(TableLocation))
   {
    in.ReadUntilChar((char)0,&Str);
    SetDataC(Pos.I,Pos.J,Str);
   }
  return true;
  }
 return false; // unknown type
}

// \r ignored
int ReadQuoted(Stream&in, Stream&out, int quote, int delim)
{
 char c=0;
 while(1)
  {
   if (in.read(&c,1) == 0) return EOF;
   if (c == '\r') continue;
   //if (c == '\n') return c;
   if (c == quote)
    {
     in.read(&c,1);
     if (c == '\r') in.read(&c,1);
     if (!delim && (c == ',' || c == '\t' || c == '|'))
       { return c; }
     if (c == '\n' || c == delim) return c;
     else out.write(&c,1);
    }
   else out.write(&c,1);
  }
}

bool SpreadsheetTable::LoadASCIILine(count_t x, Stream & in)
{
   count_t y = 1;
   bool any = false;
   bool clipquote = false;
   //char delim = 0 ; //allow updates later
   //read a delimited line

   MemoryStream current;
   char c;
   while (1)
   {
    if (!in.read(&c,1))
     {
      if (!any) return false;
      else c = '\n'; //simulate the end of the row.
     }

    if (c == '\r') continue;

    if (!any)
    {
       if (c == '\n') //ignore blank lines
        continue;

      if (c == 0)
       if (!current.pos())
        {
         in.ReadUntilChar('\n');
         continue;
        }
    }

    /*if (c == '#' && y == 1 && !current.pos())
     {
      continue; //Skip the first character and read the line normally. later: set row flag as special.
     }*/

    any = true;

    if (c == '\"' || c == '\'')
    {
     if (current.pos())
      {
       current.write(&c,1);
       continue;
      }
     else
      {  //  ""Quoted string""   "The nail is 3"" long"   abc""def
       c = ReadQuoted(in, current, c, delimiter) ;
//       c = in.ReadUntilChar('\"',&current);
      }
    }

    if (!delimiter && (c == ',' || c == '\t' || c == '|'))
     { delimiter = c; }

    if (c == delimiter || c == '\n')
    {
     if (current.pos()) Add(x,y,current);
     current.Clear();
     y ++;
     if (c == '\n')
       break;

     continue;
    }

   current.write(&c,1);
  } //end of while

 if (current.pos())  Add(x,y,current);
  //save the last column in the row
 return true;
}
/*
   int c;
   while ((c = in.ReadUntilChar("\t\n",&Str))!=EOF)
   {
    char * s = Str;
    if (x == 0 && y == 1) if (*s == '#') s++;
      //comments begin with a # sign

    char * d = s;
    while (*d) if (*d == '\r') *d=0 ; else d++;
      //remove carriage returns

    if (*s) SetDataC(x,y,s);
    Str.Clear();
    switch (c)
    {
     case '\t': y ++; break;
     case '\n': x ++; y=1; break;
     case -1  : goto End;
    }
   }  */
#endif
