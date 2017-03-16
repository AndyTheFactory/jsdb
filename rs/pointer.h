#ifndef _RS_POINTER_H
#define _RS_POINTER_H

#define DOWNCAST(p,c) TYPESAFE_DOWNCAST(p.TypecastPointer(),c)

//#ifdef CLASSLIB_POINTER_H //from Borland's pointer file
namespace rslib {
#define POINTER_NAMESPACE
//#else
//#define CLASSLIB_POINTER_H //replace the classlib
//#endif


template<class T> class TPointer
{public:
    T* P;
    TPointer() ;
    TPointer(T* pointer);
   ~TPointer();
    TPointer<T>& operator =(T* src) {delete P; P = src; return *this;}
    T& operator* () {return *P;}
    T* operator->() {return P;}  // Could throw exception if P==0
#if 0
    operator bool() {return P != 0;}
#endif
    operator T*() {return P;}
    T* TypecastPointer() { return P; }
};

template<class T> TPointer<T>::TPointer(): P(0) {}
template<class T> TPointer<T>::TPointer(T* pointer) : P(pointer) {}
template<class T> TPointer<T>::~TPointer() {delete P;}

template<class T> class TAPointer
{public:
	T *P;
    TAPointer() : P(0) {}
    TAPointer(T array[]) : P(array) {}
   ~TAPointer() {delete[] P;}
    TAPointer<T>& operator =(T src[]) {delete[] P; P = src; return *this;}
//    T& operator[](int i) {return P[i];}  // Could throw exception if P==0
  //  operator bool() {return P != 0;}
    operator T*() {return P;}
};

class TNotifyRelease
{public:
 virtual void NotifyRelease(void*) {};
};

template<class T> class TEnvelope
{public:

    TEnvelope(T* object,TNotifyRelease*r = 0) :
      Letter(new TSmartPointer(object,r)) {}

    TEnvelope(const TEnvelope& src) :
      Letter(src.Letter)
         {Letter->Lock();}

   ~TEnvelope()
         {Letter->Release();}

    TEnvelope& operator =(const TEnvelope& src);

    TEnvelope& operator =(T* object);

    T* operator->() { return Letter->Object; }
    T& operator *() { return *Letter->Object; }
    operator bool() {return Letter->Object != 0;}
    T* TypecastPointer() { return Letter->Object; }

    bool IsMine() {return Letter->RefCount == 1;}

  private:
    struct TSmartPointer
    {
     T*  Object;
     int RefCount;
     TNotifyRelease * Owner;

     public:
     TSmartPointer(T* object,TNotifyRelease*r) :
         Object(object), Owner(r), RefCount(1)
         {}

     ~TSmartPointer() { if (Object) delete Object; }

     void Lock() { RefCount++; }
     void Release()
      { if (--RefCount == 0) delete this;
        else if (Owner) if (RefCount == 1) Owner->NotifyRelease(Object);
      }
    };

    TSmartPointer* Letter;
};

// --------
template<class T>
TEnvelope<T>& TEnvelope<T>::operator =(const TEnvelope<T>& src)
{
  Letter->Release();
  Letter = src.Letter;
  Letter->Lock();
  return *this;
}

template<class T>
TEnvelope<T>& TEnvelope<T>::operator =(T* object)
{
  Letter->Release();
  Letter = new TSmartPointer(object,0);  // Assumes non-null! Use with new
  return *this;
}

#ifdef POINTER_NAMESPACE //from Borland's pointer file
}
#define TPointer rslib::TPointer
#define TAPointer rslib::TAPointer
#define TEnvelope rslib::TEnvelope
#define TNotifyRelease rslib::TNotifyRelease

#endif
#endif
