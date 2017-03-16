#include "rslib.h"
#pragma hdrstop

TDataList::TDataList() : TDataSource() {}
TDataList::~TDataList() {}

TDataSource::TDataSource() {}
TDataSource::~TDataSource() {}

size_t TDataList::FindColumn(const char * title)
{
 size_t j = ColumnCount();

 for (size_t x = 1; x <=j; x++)
  {
   const char * c = ColumnTitle(x);
   if (!c) continue;
   if (!strcasecmp(c,title)) return x;
  }
 //return strtod(title,0);
 return 0;
// return 0;
}

void TIndexList::Init(size_t * i, size_t _count)
{
 Init(_count);
 memcpy(items,i,count * sizeof(size_t));
}

void TIndexList::Init(size_t _count)
{
 if(items) delete[] items;
 count = _count;
 items=new size_t[count+1];
 items[count]=0;
}

void TIndexList::Resize(size_t _count,size_t set)
{
 size_t * olditems = items;
 size_t oldcount = count;
 items=0;
 Init(_count);
 Reset(set);
 memcpy(items,olditems,min(count,oldcount) * sizeof(size_t));
 if (olditems) delete[] olditems;
}

void TIndexList::Reset(size_t b)
 {for (size_t i = 0 ; i < count ; i++) items[i]=b; }


TIndexList& TIndexList::operator = (const TIndexList& o)
  {
   Init(o.count);
   for (size_t i = 0 ; i < count ; i++) items[i]=o.items[i];
   return *this;
  }

//-----------------------------------------------------------


TIntList& TIntList::operator = (const TIntList& o)
  {
   Init(o.count);
   for (size_t i = 0 ; i < count ; i++) items[i]=o.items[i];
   return *this;
  }

void TIntList::Init(int32 * i, size_t _count)
{
 Init(_count);
 memcpy(items,i,count * sizeof(int32));
}

void TIntList::Init(size_t _count, int32 value)
{
 if (items) delete[] items;
 count = _count;
 items=new int32[count+1];
 items[count]=0;
 Reset(value);
}

void TIntList::Resize(size_t _count,int32 set)
{
 int32 * olditems = items;
 size_t oldcount = count;
 items=0;
 Init(_count);
 Reset(set);
 memcpy(items,olditems,min(count,oldcount) * sizeof(int32));
 if (olditems) delete[] olditems;
}
void TIntList::Reset(int32 b)
 {
  for (size_t i = 0 ; i < count ; i++)
  items[i]=b;
 }

// -----------------------------------------------------------

