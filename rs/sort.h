#ifndef _RS_SORT_H
#define _RS_SORT_H

#ifndef _RS_LIST_H
#include "rs/list.h"
#endif
// sorting lists

class TQuickSorter
{public:
 size_t Width; //if strings have a fixed width, set it. Else, set Width = 0
 size_t Start,End,Count; //number of items in the source list
 bool CaseSensitive; //default false
 bool Ascending;
 TIndexList *SortOrder;
 TQuickSorter( size_t Width, size_t Count, TIndexList*SortOrder);
        //SortOrder will be initialized to the correct size
        //you can pass zero for all of these, then set them later
 ~TQuickSorter();

 void Sort(bool Ascending=true);
 virtual void PreSort();

 virtual char* Get(size_t i) {return 0;};
 //override this to do your own sorting if you use strings

protected:
 virtual int Compare(size_t i,size_t j);
   //has string comparison by default. For other types, you can
   //just override this function
   // -  a < b
   // 0  a = b
   // +  a > b

 virtual void Exchange(size_t i, size_t j);

private:
 int Direction(int i) {return Ascending? i: -i;}
 void rqsort(size_t low,size_t high);
};

class TStringSort : public TQuickSorter
{public:
 TStringList *StringList;
 TStringSort(TStringList &List, TIndexList &SortOrder);
 //the IntList will be initialized to the correct size
 ~TStringSort();
 char* Get(size_t i);
};

class TWORMSort : public TQuickSorter
{public:
 TWORMList *StringList;
 TWORMSort(TWORMList &List, TIndexList &SortOrder);
 //the IntList will be initialized to the correct size
 ~TWORMSort();
 char* Get(size_t i);
};

#ifdef DONT_COMPILE_THIS
//only available if you've already included the database headers
class TDatabaseSort : public TQuickSorter
{public:
 DBFFile * Database;
 char * RecordData;
 char * FieldData;
 TDatabaseSort(DBFFile* Database,char * fieldname, TIndexList&SortOrder);
 ~TDatabaseSort();
 char * Get(size_t i); //only goes up to the 65535th record.
};
#endif

#define SORTLIST(T,list) TListSorter<T>(list).Sort()
#define SORTLISTBACK(T,list) TListSorter<T>(list).Sort(false)
#define SORTLISTALT(T,list) TListSorterAlt<T>(list).Sort()
#define SORTLISTBACKALT(T,list) TListSorterAlt<T>(list).Sort(false)

template <class T> class TListSorter : public TQuickSorter
{public:
 TList<T> * Row;
 TListSorter(TList<T> & _row);
 ~TListSorter();
 virtual int Compare(size_t i, size_t j);
   // T must have a subtraction operator
 void Exchange(size_t i, size_t j);
};


template <class T>
TListSorter<T>::TListSorter(TList<T> & _row)
   : TQuickSorter(0,_row.Count(),0), Row(&_row) {}


template <class T>
TListSorter<T>::~TListSorter(){}

template <class T>
void TListSorter<T>::Exchange(size_t i, size_t j)
{ Row->Exchange(i,j); }

template <class T>
int TListSorter<T>::Compare(size_t i, size_t j)
{
 T* a, *b;
 a=(*Row)[i];
 b=(*Row)[j];
 return a && b ? a->Compare(*b) : a - b;
}

template <class T> class TListSorterAlt : public TListSorter<T>
{public:
 TList<T> * Row;
 TListSorterAlt(TList<T> & _row);
 ~TListSorterAlt();
 int Compare(size_t i, size_t j);
 void Exchange(size_t i, size_t j);
};

template <class T>
int TListSorterAlt<T>::Compare(size_t i, size_t j)
{
 T* a, *b;
 a=(*Row)[i];
 b=(*Row)[j];
 return a && b ? a->CompareAlt(*b) : a - b;
}
template <class T>
TListSorterAlt<T>::TListSorterAlt(TList<T> & _row)
   : TQuickSorter(0,_row.Count(),0), Row(&_row) {}


template <class T>
TListSorterAlt<T>::~TListSorterAlt(){}

template <class T>
void TListSorterAlt<T>::Exchange(size_t i, size_t j)
{ Row->Exchange(i,j); }

#endif
