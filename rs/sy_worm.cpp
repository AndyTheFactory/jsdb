#include "rslib.h"
#pragma hdrstop
/*


class TWORMList //write-once, read-many version of string list
{public:
 struct TData
 {
   TChars data;
   int pos;
   TData* next;
   TData(TData*n,size_t l): data(l), pos(0), next(n) {}
   ~TData();
 };
 TData* tail;
 TList<char> pointers;

 size_t Add(const char* x);
 void Clear();
 size_t Count() {return pointers.Count();}
 char * Get(size_t i) {return pointers.Get(i);}
 char * operator [] (size_t i)  {return Get(i);}
 TWORMList(size_t minsize);
 ~TWORMList();
};

*/
size_t TWORMList::Add(const char* x)
{
 if (!x) x = "";
 size_t l = strlen(x) + 1;
 if (!tail)
   tail = new TData(0,max(l,4096u));

 if ((tail->data.size - tail->pos) < l)
 {
  if (tail->pos == 0)
   tail->data.Resize(l);
  else
   tail = new TData(tail,max(l,4096u));
 }

 char * start = tail->data.buf + tail->pos;
 memcpy(start,x,l);
 tail->pos += l;
 return pointers.Add(start);
}

void TWORMList::Clear()
{
 while (tail)
 {
  TData * t = tail;
  tail = tail->next;
  delete t;
 }
 tail = 0;
 pointers.Clear();
}

TWORMList::TData::~TData()
{
}

TWORMList::TWORMList(size_t minsize) :  pointers(minsize,0,1024,false)
{
 tail = 0;
}

TWORMList::~TWORMList()
{
 Clear();
 tail = 0;
}
