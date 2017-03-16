#include "rslib.h"
#pragma hdrstop

/*------------------------------------------------------

Implementation of the quick and Radix sorts

------------------------------------------------------*/

void InvertList(TIndexList& il)
{
 size_t i,ic =il.Count();
 size_t imax=ic/2;
 for(i=0;i<imax;i++)
 {
  _SWAP(il[i],il[ic-i-1]);
 }
}

void TransformIndexes(TIndexList & i1, TIndexList & i2)
 {
  size_t i,imax =i1.Count();
  i2.Init(imax);
  for (i=0;i<imax;i++) i2[i1[i]]=i;
 }

/* ------------------------------------------------------------------ */


// QuickSort
TQuickSorter::TQuickSorter(size_t w, size_t c, TIndexList * il):
  SortOrder(il),
  Width(w),
  Count(c),
  Start(0),
  CaseSensitive(false)
 {
 }

TQuickSorter::~TQuickSorter() {}


int rsstrcmp(const char* a, const char* b)
{while (*b && *a && ((*b) == (*a))) {a++; b++;} return (*a)-(*b);}

int rsstricmp(const char* a, const char* b)
{while (*b && *a && (toupper(*b) == toupper(*a))) {a++; b++;} return toupper(*a)-toupper(*b);}


int TQuickSorter::Compare(size_t i,size_t j)
{
 const char* a = Get(((size_t*)(*SortOrder))[i]);
 const char* b = Get(((size_t*)(*SortOrder))[j]);
 int ret;

 if (Width) ret =
  CaseSensitive ? strncmp((a), (b),  Width):
                  strncasecmp((a), (b), Width);
 else ret =
  CaseSensitive ? rsstrcmp((a), (b)):
                  rsstricmp((a),(b));

 return ret;
};

void TQuickSorter::Exchange(size_t i, size_t j)
{
 size_t tmp = (*SortOrder)[i];
 (*SortOrder)[i]=(*SortOrder)[j];
 (*SortOrder)[j]=tmp;
};

void TQuickSorter::PreSort()
{

}

void TQuickSorter::Sort(bool asc)
{
  if (Count < 1) return;
  Ascending = asc;

  if (SortOrder)
   {
    SortOrder->Init(Count);
    size_t i;
    for (i=0;i<Count;i++) (*SortOrder)[i]=i;
   }

  if (Count <= 1) return;

  Start = 0;
  End = Count - 1;

  PreSort();

  if ( (End-Start) < 1 ) return;

  rqsort(Start, Count);
}

/** use the quicker sort algorithm */
void TQuickSorter::rqsort(size_t pivot,size_t count)
{
   if (pivot == NOT_FOUND) return;

   size_t    left, right, pivotEnd, pivotTemp, leftTemp;
   unsigned  lNum;
   int       retval;

tailRecursion:

   if (count <= 2u)
       {
        if (count == 2u)
         if (Direction(Compare(pivot,pivot+1)) > 0)
          Exchange(pivot,pivot+1);

        return; // 0 or 1 or 2
       }

  //now, start the qsort algorithm
  right = pivot + (count - 1u);
  left = pivot + (count / 2);

  /*  sort the pivot, left, and right elements for "median of 3" */

  if (Compare (left, right) > 0)
    Exchange (left, right);
  if (Compare (left, pivot) > 0)
    Exchange (left, pivot);
  else if (Compare (pivot, right) > 0)
    Exchange (pivot, right);

    if (Count == 3)
        {
         Exchange (pivot, left);
         return;
        }

  left = pivotEnd = pivot + 1;
  do
        {
        while (1)
            {
            retval = Direction(Compare(left, pivot));
            if (retval > 0) break;
            if (retval == 0)
                {
                 Exchange(left, pivotEnd);
                 pivotEnd += 1;
                }
            if (left < right)
                left += 1;
            else
                goto qBreak;
            }

        while (left<right)
            {
             retval = Direction(Compare(pivot, right));
             if (retval < 0)
                right -= 1u;
            else
                {
                Exchange (left, right);
                if (retval != 0)
                    {
                    left += 1;
                    right -= 1;
                    }
                break;
                }
            }
        }   while (left < right);

qBreak:

    if (Direction(Compare(left, pivot)) <= 0)
        left += 1u;

    leftTemp = left - 1u;

    pivotTemp = pivot;

    while ((pivotTemp < pivotEnd) && (leftTemp >= pivotEnd))
        {
        Exchange(pivotTemp, leftTemp);
        pivotTemp += 1;
        leftTemp -= 1;
        }

    lNum = (left - pivotEnd);
    count = ((pivot+count) - left);

    /* Sort smaller partition first to reduce stack usage */
    if (count < lNum)
        {
        rqsort(left, count);
        count = lNum;
        }
    else
        {
        rqsort (pivot, lNum);
        pivot = left;
        }

    goto tailRecursion;

}

#if 0
void TQuickSorter::rqsort(size_t low,size_t high )
{
   size_t pivot, base;

   int count = high - low + 1;

   if (count == 0 || low ==NOT_FOUND) return;
   if (count == 1) return;

   if (count == 2)
     {
      if (Direction(Compare(low,high)) > 0) Exchange(low,high);
      return;
     }

   if (count == 3)
     {
      int mid = low+1; bool e = false;
      if (Direction(Compare(low,mid)) > 0) {e = true; Exchange(low,mid);}
      if (Direction(Compare(mid,high)) > 0) {e = true; Exchange(mid,high);}
      if (e) if (Direction(Compare(low,mid)) > 0) Exchange(low,mid);
      return;
     }


   base = low ;            /* Remember base address of array. */
   pivot = high ;          /* Partition off the pivot.        */
   high--;

   do
   {
      while ( low < high  &&  (Direction(Compare(low,  pivot))) <= 0 )
        low  ++;

      while ( low < high  &&  (Direction(Compare(pivot, high))) <= 0 )
        high --;

      if ( low < high )      /* Exchange low & high */
      {
       Exchange(low,high);
      }

   } while ( low < high );

   if ( low < pivot  &&  (Direction(Compare(low,  pivot))) > 0 )
   { Exchange(low,pivot);}
   // "low" now points to an item that is greater than "pivot",
   // so it goes in the top part of the list, and "pivot" goes to
   // the slot that it will eventually occupy.

   low ++; //now, the two partitions are  [base, high] and [low,pivot]

      if ( low  < pivot )
        rqsort( low,  pivot );
      if ( base < high  )
        rqsort( base, high );
}
#endif

TStringSort::TStringSort(TStringList& List, TIndexList &il):
  TQuickSorter(UINT_MAX,List.Count(),&il),
  StringList(&List)
{
}

TStringSort::~TStringSort() {}

char* TStringSort::Get(size_t i)
{return (*StringList)[i];
}

TWORMSort::TWORMSort(TWORMList& List, TIndexList &il):
  TQuickSorter(UINT_MAX,List.Count(),&il),
  StringList(&List)
{
}

TWORMSort::~TWORMSort() {}

char* TWORMSort::Get(size_t i)
{return StringList->Get(i);
}

#ifdef YOUMORONDONTCOMPILETHIS
/*------------------------------------------------------------------*/

THeapSorter::THeapSorter(TStringList & sl, TIndexList & il):
  SortOrder(il),
  StringList(sl)
 {
  il.Init(sl.Count());
  size_t i,imax=StringList.Count();
  for (i=0;i<imax;i++) il[i]=i;
 }

THeapSorter::~THeapSorter() {}

TStr& THeapSorter::Get(size_t i)
 {
 TStr** s = (TStr**)StringList;
 return * (s[SortOrder[i]]);
 }

int THeapSorter::Compare(size_t i,size_t j)
{
return strcmp(StringList[SortOrder[i]],StringList[SortOrder[j]]);
};

void THeapSorter::Exchange(size_t i, size_t j)
{
 int32 tmp = SortOrder[i];
 SortOrder[i]=SortOrder[j];
 SortOrder[j]=tmp;
};

void THeapSorter::HeapSort(size_t low,size_t high)
{ //dumb sort
int i,j,exchcnt;
for (i=low; i <=high; i++)
{ exchcnt = 0;
 for (j=low; j<high; j++)
 {
  if ( Get(j+1) < Get(j) ) { Exchange(j,j+1); exchcnt++; }
 }
 if (exchcnt==0) break;
 }
}

void THeapSorter::Sort()
 {
  HeapSort(0,StringList.Count()-1);
 }

#endif
