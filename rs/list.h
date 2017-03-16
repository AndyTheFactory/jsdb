#ifndef _RS_LIST_H
#define _RS_LIST_H

#include <stddef.h>

#define TRow TList

class TDataSource
{public:
 TDataSource();
 virtual ~TDataSource();
 virtual size_t FindColumn(const char * title) = 0;
 virtual const char* GetDataC(count_t row, size_t column) = 0;
};

// acts like a table with a single row
class TNameValueList
{public:
 virtual ~TNameValueList() {}
 virtual const char* operator () (const char*c);
 virtual bool Has(const char * name);
 virtual size_t Count() = 0;
 virtual const char* Name (size_t i) = 0;
 virtual const char* Value (size_t i) = 0;
 virtual bool Set(const char * name, const char * value) {return false;}
 virtual bool Set(size_t index, const char * Value) {return false;}
 virtual void Set(const char * name, size_t nl, const char * value, size_t vl)
  {Set(TStr(name,nl),TStr(value,vl));}
 virtual void Append(TNameValueList&o); /* actually appends */

 // Moved from TParameterList (system.h, sy_param.cpp)
 void Write(TStr& out,const char * delim);
 /* Reads and parses Windows INI file section. Neat, eh? */
 size_t ReadINIFileSection(const char * file, const char * section);
 void WriteINIFileSection(const char * file, const char* section);
 int32 GetInt(const char * name,int def = 0);
 double GetDouble(const char * name,double def = 0.0);
 #ifdef _HAS_INT64
 int64 GetInt64(const char * name,int64 def = 0);
 int64 Get64(const char * name,int64 def = 0) {return GetInt64(name,def);}
 void Set64(const char * Name, int64 Value = 1);
 #endif
 /* Read values of the form { "name1=value1", "name2=value2", ... } */
 size_t Read(int start, int argc, char ** argv);
  /* Read values when running as a Netscape plug-in */
 size_t Read(int start, int argc, char ** argn, char ** argv);
  /* Reads a string of the form name1=value1/name2=value2/...*/
  // does not write a final delimiter
 size_t Read(const char *,char delim);

};

class TDataList : public TDataSource
{public: // rows and columns are indexed beginning with 1
 TDataList();
 virtual ~TDataList();

 virtual const char* filename() = 0;
 //GetDataC(row,0) returns status information for the row
 //defined values are "D" Deleted,"L" Locked,"S" special, and "" for a normal row
 bool GetRow(count_t row)
  { return GetDataC(row,0) != 0; }

 virtual const char* Name (size_t i) {return GetDataC(0,i);}

 virtual bool SetDataC(count_t row, size_t column, const char*c) {return false;}
 //SetDataC(row,0,"D") deletes a row.

 virtual count_t RowCount() {return 0;}
 virtual size_t ColumnCount() {return 0;}
 virtual const char* ColumnTitle(size_t index) {return NULL;}

 virtual size_t FindColumn(const char * title); //returns 0 if not found
 //this is the only function implemented here

 bool ReadRow(count_t i)  {return i ? GetDataC(i,0) != NULL : false;}
 int GetRowStatus(count_t i)
  {const char * x = GetDataC(i,0); return x ? x[0] : -1; }
 //advance warning that you're going to want to use that row of the table
 //returns false if the row doesn't exist.
 //usage: while (Table.ReadRow(++rownum)) { ... }
};


/*------------------------------------------------------------------------------
-  TableRow is a special utility class for accessing a line of data quickly.   -
-  Note that TableRow indexes values starting with zero, not with one, as do   -
-  data tables. Thus, a TableRow is treated like a C++ data container, not     -
-  as a database storage system.                                               -
------------------------------------------------------------------------------*/

class TableRow : public TNameValueList // completely inline!
 {public:
  TDataList* grid;
  count_t row;

  TableRow(TDataList*g=0,count_t r=0)
    {grid=g; row=r;}

  inline size_t Count()
    {return grid ? grid->ColumnCount() : 0;}

  inline const char * Name(size_t i)
    {return grid ? grid->ColumnTitle(i+1):0;}

  inline const char * GetTableValue(size_t i)
    {const char * x = grid ? grid->GetDataC(row,i) : 0;
     return x ? x : "";
    }

  inline const char * Value(size_t i)
    {return GetTableValue(i+1);
    }

 inline bool Has(const char * name)
    {return grid ? grid->FindColumn(name) != 0: 0;}

  inline bool Set(size_t j,const char * c) //doesn't check that the grid is valid
    {return grid->SetDataC(row,j+1,c);}

  inline bool Set(const char * f,const char * c)
    {count_t j = grid->FindColumn(f);
     return (j!=0) ? grid->SetDataC(row,j,c) : false;
    }

  inline const char * operator () (const char *f)
    {return GetTableValue(grid->FindColumn(f));
    }

  inline const char * operator [] (const char *f)
    {return GetTableValue(grid->FindColumn(f));
    }

  inline const char * operator [] (size_t i) //starts at zero
    {return grid ? grid->GetDataC(row,i+1) : 0;}

  inline bool IsActive()
    {const char * x = row ? grid->GetDataC(row,0) : NULL ;
     return x ? (x[0] != 'D' && x[0] != 'd') : false;
    }

  inline operator bool()
    {return row ? grid->GetDataC(row,0) != NULL : 0;}

  inline bool operator ++ ()
    {row++; return (*this);}
 };

template <class T> class TList
  {
  public:
   T ** data;
  protected:
   size_t _count,_top,_delta;
  public:
   bool AutoDelete;
   typedef void (*IterFunc)(T &,void*);
   typedef bool (*CondFunc)(T &,void*);

  TList(bool AutoDelete=true);
  TList(int initial,int ignored, int delta, bool AutoDelete=true);
  //for compatibility with a TArray class
  ~TList();

   size_t Add(T *t)  {return t ? AddAt(t,_top) : NOT_FOUND;}
   T * Get(size_t loc) const {return loc < _top ? data[loc] : 0;}
   T * operator [] (size_t loc) const {return Get(loc);}

   T * First() {return Get(0);}
   T * Last() {return Get(_top-1);}

   size_t IndexOf(T * t) const ;
   bool HasMember(T * t) const  {return (IndexOf(t)!=NOT_FOUND);}
   bool Has(size_t loc) const  {return Get(loc)!=0;}

   void Push(T*t) {Add(t);}
   T * Pop() {return _top ? data[--_top] : 0; }

   void Destroy(size_t loc,bool concat = true);
   void Purge(size_t loc,size_t count);
   T *  Remove(size_t loc,bool concat = true);
    //removes, but doesn't delete

   size_t Count() const  {return _top;}

   size_t AddAt(T *t,size_t loc);
   size_t Replace(T *t, size_t loc);
   size_t Expand(size_t size);

   void Move(size_t from, size_t to);
   void Exchange(size_t i, size_t j);

   void Flush(size_t start=0);
   void Clear() {Flush(0);}
   size_t MaxSize() {return _count;}
  };

/*----------------------------------------------------------------------
 Some useful typed lists
 TIndexList, TBoolList in sy_list.cpp
 ----------------------------------------------------------------------*/
struct TInt32
 {
  public:
  int32 number;
  TInt32() {number=0;};
  TInt32(int32 l) {number=l;};
  TInt32(TInt32&t) {number=t.number;};
  ~TInt32() {};
  int32 operator +(TInt32&t) {return number + t.number;};
  int32 operator +=(TInt32&t) {return number += t.number;};
  bool operator > (TInt32&t) {return (number > t.number);};
  bool operator < (TInt32&t) {return (number < t.number);};
  bool operator == (TInt32&t) { return (number==t.number);};
  int32 operator ++ () {return number++;}      ;
  operator int32 () {return number;};
  TInt32 & operator = (const TInt32 &t) {number=t.number;return *this;};
  int Compare(TInt32& t) {return number - t.number;}
 };

struct TIntList
{int32 *items; size_t count;
 TIntList() {items=0; count=0; }
 TIntList(size_t _count) {items=0;count=0;Init(_count);}
 ~TIntList() {if (items) delete [] items;}

 void Init(int32 * i, size_t _count);
 void Init(size_t _count, int32 def = 0);

 int32 Get(size_t i) {return i < count ? items[i] : items[count];}
 void Put(size_t i, int32 x) {(*this)[i] = x;}

 size_t Count() {return count;}

 void Reset(int32 b);

 //
 void Resize(size_t _count,int32 set);
 void Add(int32 b) {Resize(count+1,b);}

 operator int32 * () {return items;}
#ifdef __BORLANDC__
 int32& operator [] (size_t i) {return i < count ? items[i] : items[count];}
#endif
 TIntList& operator = (const TIntList& o);
};

struct TIndexList
{size_t *items, count;
 TIndexList() {items=0; count=0; }
 ~TIndexList() {if (items) delete [] items;}
 void Init(size_t * i, size_t _count);
 void Init(size_t _count);

 TIndexList(size_t _count) {items=0;count=0;Init(_count);}
 size_t Count() {return count;}

 void Reset(size_t b);

 void Resize(size_t _count,size_t set=0);
 void Add(size_t b) {Resize(count+1,b);}

 operator size_t * () {return items;}
#ifdef __BORLANDC__
 size_t& operator [] (size_t i) {return i < count ? items[i] : items[count];}
#endif
 TIndexList& operator = (const TIndexList& o);
};

//convert index to sort location
void TransformIndexes(TIndexList & i1, TIndexList & i2);
//sort descending
void InvertList(TIndexList& il);

#define FOR(i,list) for(size_t i=0;i<list.Count();i++)

#define FOREACHBACK(x,TList) \
{ \
size_t i,_ndx,imax= (TList).Count(); \
for (_ndx=1;_ndx<=imax;_ndx++) \
 { i = imax - _ndx; x = (TList)[i]; {

#define FOREACHPTR(x,TList) \
{ \
size_t i,imax= (TList).Count(); \
for (i=0;i<imax;i++) \
 { if ((TList)[i]==NULL) continue; x=(TList)[i]; {

#define FOREACHITER(TList) \
{ \
size_t i,imax= (TList).Count(); \
for (i=0;i<imax;i++) \
 {{

#define FOREACH(x,TList) \
{ \
size_t i,imax= (TList).Count(); \
for (i=0;i<imax;i++) \
 { x = (TList)[i]; {

#define DONEFOREACH }}}

/*-------------------------------------------------------
                  Template implementations
---------------------------------------------------------*/

template <class T>
size_t TList<T>::Replace(T *t, size_t loc) {return AddAt(t,loc);}

template <class T>
void TList<T>::Exchange(size_t i, size_t j)
{
 if ((i>=_top) || (j >=_top)) return;
 T*t = data[i];
 data[i]=data[j];
 data[j]=t;
}

template <class T>
TList<T>::TList(int initial,int, int delta, bool AD)
  {
  _count=initial;_delta=delta;_top=0;
    data=new T*[_count+1];
  memset(data,0,sizeof(T*) * (_count+1));
  AutoDelete = AD;
  }

template <class T>
TList<T>::TList(bool AD)
   {
     _count=4;
     _delta=8;
     _top=0;
      data=new T*[_count+1];
     memset(data,0,sizeof(T*) * (_count+1));
     AutoDelete = AD;
    }

template <class T>
TList<T>::~TList()
   {
    if (AutoDelete) Flush();
    delete [] data;
   }

template <class T>
size_t TList<T>::IndexOf(T * t) const {
       for (size_t i=0;i<_top;i++)
      if (data[i]==t) return i;
       return NOT_FOUND;
      }

 template <class T>
  size_t TList<T>::Expand(size_t size) {
       _count=size;
       size++;
         T ** ni=new T*[size];
         memset(ni,0,size*sizeof(T *)) ;
         memmove(ni,data,(_top)*sizeof(T *));
       delete [] data;
       data=ni;
       return _count;
     }

template <class T>
size_t TList<T>::AddAt(T *t,size_t loc)  {

      if (loc>=_count) //need to allocate space
        {
       if (_delta < 128 && _delta < _count/4)
         _delta += 8;

       Expand(_count+_delta);
        }

      if (loc < _top)
        {
        if (AutoDelete && (data[loc]!=0)) delete data[loc];
        data[loc]=t;
        }
        else
        {
         data[_top]=t;
         loc = _top;
         _top++;
        } //allocate a little more space
     return loc;
     }

 template <class T>
  void TList<T>::Move(size_t from, size_t to)
  {
   if (from >= _top || to >= _top) return;

   size_t i;
   if (from < to) for (i = from ; i < to ; i++) Exchange(i,i+1);
   else
   if (to < from) for (i = from ; i > to ; i--) Exchange(i,i-1);
  }

 template <class T>
  void TList<T>::Destroy(size_t loc,bool concat)
  {
       T *t=Remove(loc,concat);
//       if (!concat) data[loc]=0;
       if (AutoDelete && t) delete t;
  }

 template <class T>
  void TList<T>::Purge(size_t loc,size_t count)
    {
      if (loc >= _top) return;

      size_t last = min(loc+count,_top);
      count = last - loc;

      T *t;
      for (size_t i=loc;i<last; i++)
      {
       t=data[i];
       if (AutoDelete && t) delete t;
      }

     if (last < _top)
      {
       memmove(data+loc,data+last,(_top-last)*sizeof(T *));
       _top -= count;
      }
     else
      {
       _top = last;
      }

  }

 template <class T>
  T* TList<T>::Remove(size_t loc,bool concat)
  {
       if (loc >= _top) return 0;

       T *t=data[loc];
       data[loc]=0;

       if (concat && _top)
       {
        size_t nt = _top - 1;
        if (loc != nt) Move(loc,nt);
        _top = nt;
       }
      return t;
  }

 template <class T>
  void TList<T>::Flush(size_t start)
 {
  if (AutoDelete)
  {
       for (size_t i=start;i<_top;i++)
      if (data[i]) {delete data[i];data[i]=0;};
  }
  else
  {
     memset(data + start,0,(_count-start) * sizeof(T*));
  }
  _top=start;
 }

#endif
