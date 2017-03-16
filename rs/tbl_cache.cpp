#include "rslib.h"
#pragma hdrstop

TableDataCache::TableDataCache(size_t Columns)
{
 columnCount = Columns;
 pageSize = 16384;
 maxPages = 64;
 lastSearch = 0;
}

TableDataCache::~TableDataCache()
{
}

void TableDataCache::Flush()
{
 Items.Flush();
 lastSearch=0;
}

//2004 SpreadsheetTable was modified not to save empty strings. Oops.
void TableDataCache::Set(count_t x,size_t y, const char * data)
{
 size_t imax = Items.Count();
 size_t start = min(lastSearch,imax);
 SpreadsheetTable::TData * d;
 size_t index=0;

 for (size_t i=0; i<imax; i++)
 {
  size_t j = (i+start)%imax;
  d = Items[j]->Get(x,y,index);
  if (d)
   {
    lastSearch = j;
    d->Set(data);
    return;
   }
 }
 //not found, add to end

 SpreadsheetTable* current = Items.Last();
 if (!current)
 {
   current = new SpreadsheetTable();
   Items.Add(current);
 }
 else
 if (current->Items.Count() >= pageSize) //full page
   {
    if (Items.Count() >= maxPages) //drop the oldest page
     Items.Destroy(0);

    current->Optimize(); //sort the top page
    current = new SpreadsheetTable();
    Items.Add(current);
   }

 current->SetDataC(x,y,data);
}

const char * TableDataCache::Get(count_t x,size_t y)
{
 size_t imax = Items.Count();
 size_t start = min(lastSearch,imax);
 SpreadsheetTable::TData * d;
 size_t index=0;

 for (size_t i=0; i<imax; i++)
 {
  size_t j = (i+start)%imax;
  d = Items[j]->Get(x,y,index);
  if (d)
   {
    lastSearch = j;
    return d->Get();
   }
 }
 return 0;
}

#if 0
 TableDataCache::TableDataCache(size_t Size)
  : Items(256,0,256)
 {
  sequence = 1;
  lastsearch = lastsort = 0;
  Oldest = 0;
  size=Size;
  if (size <= 0) size = 65535;
 }
 TableDataCache::~TableDataCache() {}

 void TableDataCache::Unset(count_t x)
  {
   Oldest = UINT_MAX;
   FOREACH(TData* d,Items)
    if (d->x == x)
     {
      d->age = 0; //make it look really old?
      if (Oldest == UINT_MAX) Oldest = i;
     }
   DONEFOREACH
  }

 const char * TableDataCache::Set(count_t x,size_t y, const char * data)
   {
    TData* d = Find(x,y);
     if (d)
      {
       d->Data = data;
      }
    else if (Items.Count() >= size) //recycle the cache
      {
//       Items.Flush(size);       //clear out the extra elements
//       FOREACH(TData* d,Items)
//        d->age = i+1;
//       DONEFOREACH
//       sequence = size+1;
       d = Items[Oldest++];
       if (!d) {Oldest = 0; d=Items.First();}
       d->x = x;
       d->y = y;
       d->age = sequence++;
       d->Data = data;
      }
    else
     {
      d = new TData(x,y,sequence++,data);
      Items.Add(d);
     }
    return d->Data;
   }

const char * TableDataCache::Get(count_t x,size_t y)
  {
   if (lastsort + 1024 < imax) //resort
   {

  // sort here, but modify rowsorter code to handle null items;
  SORTLIST(TData,Items);

  // the whole list is sorted now!
  if (Items.Count())
    lastsort = Items.Count() - 1;
  else
    lastsort = 0;
   }

   TData*d = Find(x,y);
   if (d)
    {
     d->age = sequence++;
     return d->Data;
    }
   else return 0;
  }

TableDataCache::TData* TableDataCache::Find(count_t x,size_t y)
 {
  size_t imax = Items.Count();
  TData *d;

  for (size_t i = LastSearch; i < imax; i++)
   {
    d = Items[i];
    if (!d) break;
    if (d->age && d->x == x && d->y == y)
      {LastSearch = i; return d;}
   }
  imax = min(LastSearch,imax);
  LastSearch = 0;
  for (size_t i = 0; i < imax; i++)
   {
    d = Items[i];
    if (!d) break;
    if (d->age && d->x == x && d->y == y)
      {LastSearch = i; return d;}
   }
  return 0;
 }
#endif
