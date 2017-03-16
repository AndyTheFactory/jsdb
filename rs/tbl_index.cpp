#include "rslib.h"
#pragma hdrstop

#include "rs/sort.h"


DataIndex::DataIndex(TDataList& Source,size_t column)
 : Index(Source.RowCount()),Overflow(0)
{
 base=1;
 CaseSensitive = true;
 Data = &Source;
 Columns.Init(1);
 MainColumn = Source.Name(column);
 Columns[0] = column;
}


DataIndex::DataIndex(TDataList& Source,TStringList& ColumnNames)
 : Index(Source.RowCount()),Overflow(0)
{
 base=1;
 CaseSensitive = true;
 Data = &Source;
 Columns.Init(ColumnNames.Count());
 MainColumn = ColumnNames[0];
  FOREACH(const char*c, ColumnNames)
   Columns[i] = Source.FindColumn(c);
  DONEFOREACH
}

DataIndex::DataIndex(TNameValueList& Source)
 : Index(0),Overflow(0),Columns(0)
{
 Data=0;
 base=0;
 Order.Init(Source.Count());
 Rows.Init(Source.Count());
 CaseSensitive = true;
 size_t imax = Source.Count();
 for (size_t i=0; i< imax; i++)
 {
   Index.Add(Source.Name(i));
   Order[i]=0;
   Rows[i]=i;
 }
 BuildIndex(0,0);
}

DataIndex::DataIndex(size_t b)
 : Index(0),Overflow(0),Columns(0),base(b)
{
 Data=0;
 Order.Init(0);
 Rows.Init(0);
 CaseSensitive = true;
}

bool DataIndex::BuildIndex(RSCallback Callback,void*v)
{
  if (!Data)
  {
   size_t oldsize = Order.Count();
   size_t newc = Overflow.Count();
   size_t newsize = oldsize + newc;
   size_t i;
   Order.Resize(newsize);
   Rows.Resize(newsize);
   for (i=0; i<newc; i++)
   {
     Rows[oldsize+i]=base+oldsize+i;
     Index.Add(Overflow[i]);
   }

   Overflow.Clear();
   TWORMSort(Index,Order).Sort();
   return true;
  }

  Index.Clear();
  Overflow.Clear();

  count_t row;
  size_t col0 = 0;
  if (Columns.Count() == 1)
   col0 = Columns[0];

  count_t max = Data->RowCount();
  Rows.Init(max);

  for (row = base; Data->ReadRow(row) && row <= max; row++)
  {
   if (!(row%4)) if (Callback) if (!Callback(v,row,max,0)) return false;
   const char * m = Data->GetDataC(row,0);

   if (!m) break;
   if (m[0] == 'D') continue; //skip deleted rows

   if (col0)
   {
    Rows[Index.Add(Data->GetDataC(row,col0))] = row;
   }
   else
   {
    temp.Clear();
    FOR(i,Columns)
    {
     if (i) temp << "\n";
     temp << Data->GetDataC(row,Columns[i]);
    }
    Rows[Index.Add(temp)] = row;
   }
  }

  TWORMSort(Index,Order).Sort();

  return true;
}

void DataIndex::BuildKey(Stream& key, TNameValueList& Q)
{
 FOR(i,Columns)
   {
    if (i) key << "\n";
    key << Q(Data->ColumnTitle(Columns[i]));
   }
}

size_t DataIndex::FindRecord(TNameValueList& Q)
{
 size_t loc = 0;
 if (Columns.Count()==1)
 {
  if (FindPosition(Q(MainColumn),loc)) return loc;
 }
 else
 {
  temp.Clear();
  BuildKey(temp,Q);
  if (FindPosition(temp,loc)) return loc;
 }
 return 0;
}

void DataIndex::AddKey(const char * key)
{
 Overflow.Add(key);
}

void DataIndex::AddRecord(TNameValueList& Rec)
{
 if (Columns.Count()==1)
 {
  AddKey(Rec(MainColumn));
 }
 else
 {
  temp.Clear();
  BuildKey(temp,Rec);
  AddKey(temp);
 }
}

bool DataIndex::FindPosition(const char * key, size_t &x)
{
  size_t nelem = Order.Count();
  size_t low;
  size_t i,probe;
  int j;

  low = 0;
  if (nelem)
  {
   while (nelem > 0)
   {
    i = nelem >> 1; // *1/2
    probe = low + i; //midpoint

    const char * c = Index[Order[probe]];
    if (!c) c = "";
    j = CaseSensitive ? rsstrcmp(key,c): rsstricmp(key,c);

    if (j == 0)
      {x = Rows[Order[probe]]; return true;}
    else if (j < 0) //look lower
      nelem = i;
    else //look higher
    {
      low = probe + 1;
      nelem = nelem - i - 1;
    }
   }
   const char * c = Index[Order[probe]];
   if (!c) c = "";
   j = CaseSensitive ? rsstrcmp(key,c): rsstricmp(key,c);
   if (j == 0)
   {
      x = Rows[Order[probe]];
      return true;
   }
  }

  size_t ocount = Order.Count();
  bool ret = false;
  FOREACH(const char * c,Overflow)
   j = CaseSensitive ? rsstrcmp(key,c) : rsstricmp(key,c);
   if (j == 0)
    {
     x = ocount + i + base;
     ret = true;
     break;
    }
  DONEFOREACH

  if (!Data && Overflow.Count() > 128)
    BuildIndex(0,0);

  return ret;
}
