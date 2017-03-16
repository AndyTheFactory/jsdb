#include "rslib.h"
#pragma hdrstop

#include <stdarg.h>

DataTable::DataTable(): TDataList()
  {
   TableType = ETableUnknown;
   Style = (ETableStyle) 0;
   IsModified = IsChanged = false;
  }

DataTable::~DataTable()
 {
 }

void DataTable::FreeMemory()
{}

const char * DataTable::GetLastError() {return "";}

count_t DataTable::FindRow(TNameValueList & pl,count_t start,int delta)
{
 if (delta == 0) return 0; //avoid infinite loops
 if (start == 0)
   {
    if (delta > 0) start = 1ul;
    else start = RowCount();
   }
 if (start == 0) return 0;

 TIntList columns(pl.Count());
 size_t i,imax=pl.Count();
 for (i=0;i<imax;i++)
 {
    columns[i]=FindColumn(pl.Name(i));
 }

 const char * c ;
 if (delta > 0)  for (count_t x=start; GetDataC(x,0); x++)
  {
   if (*GetDataC(x,0)) continue; //deleted record
   for (i=0;i<imax;i++)
   {
    if (!columns[i]) continue;
    c= GetDataC(x,columns[i]);
    if (!c || strcasecmp(c,pl.Value(i)) != 0) goto endloopf;
   }
   return x;
   endloopf: ;
  }
 else if (delta < 0) for (count_t x=start; GetDataC(x,0); x--)
  {
   if (*GetDataC(x,0)) continue; //deleted record
   for (i=0;i<imax;i++)
   {
    if (!columns[i]) continue;
    const char * c = GetDataC(x,columns[i]);
    if (!c || strcasecmp(c,pl.Value(i)) != 0) goto endloopb;
   }
   return x;
   endloopb: ;
  }

 return 0;
}

EDBFieldType DataTable::GetColStatus(size_t)
{ return db_ft_Char; }

bool DataTable::GetRow(count_t row,TNameValueList& pl)
{
 size_t jmax = ColumnCount();

 if (!GetDataC(row,0)) return false;

 for (count_t j = 1; j <= jmax;j++)
  {
   const char * c = GetDataC(row,j);
   const char * title =ColumnTitle(j);
   pl.Set(title,c ? c : "");
  }
 return true;
}

bool DataTable::SetRow(count_t row,TNameValueList& pl)
{
 size_t jmax = ColumnCount();

 if (row==0)
  {
   return AddRow(pl);
  }
 else
  {
   for (size_t j = 1; j <= jmax; j++)
    {
     const char * c = ColumnTitle(j);
     if (pl.Has(c)) SetDataC(row,j,pl(c));
    }
   return true;
  }
}

TableRow DataTable::operator [] (count_t row)
 {
  if (GetDataC(row+1,0) == NULL) return TableRow(this,0);
  return TableRow(this,row+1);
 }

double DataTable::GetDataD(count_t i, size_t j)
  {
   const char * c = GetDataC(i,j);
   return c ? strtod(c,0) : 0.0;
  }

bool DataTable::SetDataD(count_t i, size_t j, double d)
  {
   char s[128];
#ifdef __BORLANDC__
  gcvt(d,10,s);
#else
  sprintf(s,"%f",d);
#endif
   return SetDataC(i,j,s);
  }

int DataTable::GetColWidth(size_t) {return INT_MAX;}

const char * DataTable::ColumnTitle(size_t index)
{
 return GetDataC(0,index);
}


const char * DataTable::GetDataC(count_t ,size_t )
{
 return 0;
}

bool DataTable::SetTitle(size_t index,const char *c)
{
 return SetDataC(0,index,c);
}

count_t DataTable::AddRow(int count,...)
{
 va_list l;
 const char * c;
 va_start(l, count);
 TParameterList D;
 int i = 1;

 while (i <= count && (c = va_arg(l,char*)) != 0)
   {
    D.Set(ColumnTitle(i),c);
    i++;
   }
 va_end(l);
 return AddRow(D);
}

bool DataTable::Save() {return true;}
count_t DataTable::AddRow(TNameValueList&) {return 0;}

size_t DataTable::AddCol(const char *) {return 0;}
//bool DataTable::DelRow(count_t) {return true;}
bool DataTable::DelCol(size_t) {return true;}

