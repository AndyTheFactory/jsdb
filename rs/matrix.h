#ifndef _RS_MATRIX_H
#define _RS_MATRIX_H


/******************** Template data storage classes ********************/

/*-----------------------------------------------------------------------
 TIMatrix<class T>
 Implements a fixed-size two-dimensional indirect matrix of type T.
-----------------------------------------------------------------------*/
template <class T> class TIMatrix
  {
  protected:
   T ***items;
   size_t _width,_height;
  public:
   TIMatrix(size_t w,size_t h);
   ~TIMatrix();
   T* Get(size_t i,size_t j);
   bool AddAt(size_t i,size_t j,T *t);
   T** Direct(size_t i,size_t j);
   void Flush();
   size_t Width() {return _width;};
   size_t Height() {return _height;};
   };


/*-----------------------------------------------------------------------
 TDMatrix<class T>
 Implements a fixed-size two-dimensional direct matrix of type T.
-----------------------------------------------------------------------*/
template <class T> class TDMatrix //meant for numeric types
  {
  protected:
   T **items;
   T invalid;
//   bool AutoDelete;
   size_t _width,_height;
  public:
   TDMatrix();
   TDMatrix(size_t w, size_t h);
   TDMatrix(T* _items,size_t height);
   ~TDMatrix();

   void Init(size_t w, size_t h,T t = 0);
   void Done();

   T Get(size_t i,size_t j);
   bool Put(size_t i,size_t j,T t);
   void Inc(size_t i,size_t j,T t = 1);   //increment an item

   const size_t Width() {return _width;}
   const size_t Height() {return _height;}

   void Reset(T t = 0);

   //Handy algorightms
   const size_t MaxInCol(size_t i,size_t j,size_t maxj);
   const size_t MaxInDiag(size_t i,size_t j,bool* skip);
   operator T** () {return items;}

   };

// square diagonal matrix

template <class T> class TDDiagMatrix
  {
  protected:
   T **items;
   T invalid;
//   bool AutoDelete;
   size_t _width;
  public:
   TDDiagMatrix();
   ~TDDiagMatrix();
   void Init(size_t w);
   void Done();
   T Get(size_t i,size_t j);
   bool AddAt(size_t i,size_t j,T t);
   T& Direct(size_t i,size_t j);
   void Flush(T t = 0);
   const size_t Width() {return _width;}
   const size_t Height() {return _height;}
   const size_t MaxInCol(size_t i,size_t j,size_t maxj);
   const size_t MaxInDiag(size_t i,size_t j,bool* skip);
   void Inc(size_t i,size_t j,T t = 1);
   operator T** () {return items;}
   };


/*---------------------------------------------------
  TD3DMatrix
  Implements a direct, 3-D matrix
-----------------------------------------------*/

template <class T> class TD3DMatrix //3 dimensions
  {
  protected:
   T ***items;
   T invalid;
   size_t _width,_height,_depth;
  public:
   TD3DMatrix();
   TD3DMatrix(size_t w, size_t h, size_t z);
   ~TD3DMatrix();
   void Copy(TD3DMatrix<T> & c);
   void Init(size_t w, size_t h, size_t z);
   void Done();
   T** Plane(size_t k);
   T* Column(size_t i, size_t k);
   T Get(size_t i,size_t j,size_t k);
   bool AddAt(size_t i,size_t j,size_t k,T t);
   T& Direct(size_t i,size_t j,size_t k);
   void Flush(T t = 0);
   const size_t Width() {return _width;}
   const size_t Height() {return _height;}
   const size_t Depth() {return _depth;}
   const size_t MaxInCol(size_t i,size_t j,size_t k,size_t maxj);
   const size_t MaxInDiag(size_t i,size_t j,size_t k,bool* skip);
   void Inc(size_t,size_t,size_t,T t = 1);
   operator T*** () {return items;}
   };


/*---------------------------------------------------
  TI3DMatrix
  Implements and indirect, 3-D matrix
-----------------------------------------------*/
template <class T> class TI3DMatrix
  {
  protected:
   T ****items;
   size_t _width,_height,_depth;
  public:
   TI3DMatrix(size_t w,size_t h,size_t d);
   ~TI3DMatrix();
   T* Get(size_t i,size_t j,size_t k);
   bool AddAt(size_t i,size_t j,size_t k,T *t);
   T** Direct(size_t i,size_t j,size_t k);
   void Flush();
   size_t Width() {return _width;};
   size_t Height() {return _height;};
   size_t Depth() {return _depth;};
   };


//-----------------------------------------------------------------------
//TMatrix methods
// TImatrix

template <class T>
TIMatrix<T>::TIMatrix(size_t w,size_t h)
    {
      _width=w;
      _height=h;
      items=new T**[w];
      for (size_t i=0;i<w;i++)
      {
       items[i]=new T*[h];
       memset(items[i],0,sizeof(T*) * h);
      }
    }

/*--------------------------------------------------------------------------

                           Implementations

--------------------------------------------------------------------------*/

template <class T>
TIMatrix<T>::~TIMatrix()
     {
     Flush();
      for (size_t i=0;i<_width;i++)
	{
	 delete [] items[i];
	};
      delete [] items;
     }

template <class T>
T* TIMatrix<T>::Get(size_t i,size_t j)
     {
      if (i<_width && j<_height)
      return items[i][j];
      return 0;
     }


template <class T>
T** TIMatrix<T>::Direct(size_t i,size_t j)
	{
      if (i<_width && j<_height)
      return &(items[i][j]);
      return 0;
     }

template <class T>
bool TIMatrix<T>::AddAt(size_t i,size_t j,T *t)
     {
      if (i<_width && j<_height)
       {
	T* del=items[i][j];
	if (del!=0) delete del;
	items[i][j]=t;
	return true;
       }
      return false;
     }

template <class T>
void TIMatrix<T>::Flush()
  {
   for (size_t i = 0; i < _width ; i++)
     for (size_t j=0; j < _height; j++)
      {
      T** t=&items[i][j];
       if (*t)
	{
	 delete *t;
	 *t=0;
	}
      }
  }

//TDMatrix, for fast numerical processing
template <class T>
TDMatrix<T>::TDMatrix()
{_width=_height=0;
 items=0;
}

template <class T>
TDMatrix<T>::TDMatrix(size_t w, size_t h)
{_width=_height=0;
 items=0;
 Init(w,h);
}

template <class T>
void TDMatrix<T>::Init(size_t w,size_t h, T t)
    {
    Done();
      _width=w;
      _height=h;
      items=new T*[w+1];
      for (size_t i=0;i<w+1;i++)
	{
	 items[i]=new T[h+1];
//	 memset(items[i],0,sizeof(T) * (h+1));
	};
   Reset(t);
    }

template <class T>
const size_t TDMatrix<T>::MaxInCol(size_t i,size_t j,size_t maxj)
//i,start are coordinates of column, going down
 {
 int v=0;
 size_t c=NOT_FOUND;
 if (i >= _width) return c;
 T * t = items[i];
 maxj = min(_height,maxj);
 for ( ;j< maxj; j++)
  {
   if (t[j] > v) {c=j; v=t[j];}
  }
 return c;
 }

template <class T>
const size_t TDMatrix<T>::MaxInDiag(size_t i,size_t j,bool* skip)
//i,start are coordinates of column, going down
 {
  int v=0,c=NOT_FOUND;
 if ((i >= _width)||(j >= _height)) return c;

  for ( ; i < _width; j++)
   for ( ; j < _height; j++)
    {
      if (skip[j]) if (items[i][j] > v) {c=j; v=items[i][j];}
    }
   return c; // returns the row belonging to the greatest count.
 }


template <class T>
TDMatrix<T>::~TDMatrix()
    {
     Done();
    }

template <class T>
void TDMatrix<T>::Done()
     {
     if (!items) return;
//     Flush();
      for (size_t i=0;i<_width;i++)
	{
	 delete [] items[i];
	};
      delete [] items;
     }

template <class T>
void TDMatrix<T>::Inc(size_t i,size_t j,T t)
     {
      if (i<_width && j<_height) items[i][j] += t;
     }


template <class T>
T TDMatrix<T>::Get(size_t i,size_t j)
     {
      if (i<_width && j<_height)
      return items[i][j];
      return 0;
     }

template <class T>
bool TDMatrix<T>::Put(size_t i,size_t j,T t)
     {
      if (i<_width && j<_height)
       {
	//T* del=items[i][j];
	//if (del!=0) delete del;
	items[i][j]=t;
	return true;
       }
      return false;
     }

template <class T>
void TDMatrix<T>::Reset(T t)
  {
   for (size_t i = 0; i < _width ; i++)
     for (size_t j=0; j < _height; j++)
      {
      items[i][j]=t;
      }
  }

// TI3DMatrix, indirect 3-D matrix

template <class T>
TI3DMatrix<T>::TI3DMatrix(size_t w,size_t h, size_t d)
    {
      _width=w;
      _height=h;
      _depth=d;
   items=new T***[w];
   for (size_t i=0;i<w;i++)
	{
	 items[i]=new T**[h];
    for (size_t j=0; j < h; j++)
    {
    items[i][j]=new T*[d];
	 memset(items[i][j],0,sizeof(T*) * d);
    }
	};
    }

template <class T>
TI3DMatrix<T>::~TI3DMatrix()
     {
     Flush();
      for (size_t i=0;i<_width;i++)
	{
	 for (size_t j=0; j<_height;j++)
     {
      delete [] items[i][j];
     }
     delete [] items[i];
	};
      delete [] items;
     }

template <class T>
T* TI3DMatrix<T>::Get(size_t i,size_t j,size_t k)
     {
      if ((i<_width) && (j<_height) && (k<_depth))
        return items[i][j][k];
      return 0;
     }

template <class T>
T** TI3DMatrix<T>::Direct(size_t i,size_t j,size_t k)
	{
      if ((i<_width) && (j<_height) && (k<_depth))
        return &items[i][j][k];
      return 0;
     }

template <class T>
bool TI3DMatrix<T>::AddAt(size_t i,size_t j,size_t k,T *t)
     {
      if ((i<_width) && (j<_height) && (k<_depth))
       {
	T* del=items[i][j][k];
	if (del!=0) delete del;
	items[i][j][k]=t;
	return true;
       }
      return false;
     }

template <class T>
void TI3DMatrix<T>::Flush()
  {
   for (size_t i = 0; i < _width ; i++)
     for (size_t j=0; j < _height; j++)
       for (size_t k=0; k < _depth; k++)
      {
      T** t=&items[i][j][k];
       if (*t)
	{
	 delete *t;
	 *t=0;
	}
      }
  }

// TD3DMatrix, 3-D analog of the previous
template <class T>
TD3DMatrix<T>::TD3DMatrix()
{_width=_height=_depth=0;
 items=0;
}

template <class T>
TD3DMatrix<T>::TD3DMatrix(size_t w, size_t h, size_t d)
{_width=_height=_depth=0;
 items=0;
 Init(w,h,d);
}

template <class T>
void TD3DMatrix<T>::Init(size_t w,size_t h,size_t d)
    {
    Done();
      _width=w;
      _height=h;
      _depth=d;
      items=new T**[d+1];
      for (size_t k=0;k<d+1;k++)
	{
	 items[k]=new T*[w+1];
    for (size_t i=0;i<w+1;i++)
    {
    items[k][i]=new T[h+1];
	 memset(items[k][i],0,sizeof(T) * (h+1));
    }
	}
}

template <class T>
const size_t TD3DMatrix<T>::MaxInCol(size_t i,size_t j,size_t k,size_t maxj)
//i,start are coordinates of column, going down
{int v=0;
 size_t c=NOT_FOUND;
 if (i >= _width) return c;
 T * t = items[k][i];
 maxj = min(_height,maxj);
 for ( ;j< maxj; j++)
  {
   if (t[j] > v) {c=j; v=t[j];}
  }
 return c;
 }

template <class T>
const size_t TD3DMatrix<T>::MaxInDiag(size_t i,size_t j,size_t k,bool* skip)
//i,start are coordinates of column, going down
 {
  int v=0,c=NOT_FOUND;
 if ((i >= _width)||(j >= _height)) return c;

  for ( ; i < _width; j++)
   for ( ; j < _height; j++)
    {
      if (skip[j]) if (items[k][i][j] > v) {c=j; v=items[k][i][j];}
    }
   return c; // returns the row belonging to the greatest count.
 }


template <class T>
TD3DMatrix<T>::~TD3DMatrix()
    {
     Done();
    }

template <class T>
void TD3DMatrix<T>::Done()
     {
     if (!items) return;
//     Flush();
   for (size_t k=0;k<_depth+1;k++)
   {
    for (size_t i=0;i<_width+1;i++)
      // delete [] items[k][i];
         delete items[k][i];

  //  delete [] items[k];
    delete items[k];
     }
   delete [] items;
   items=0;
   }

template <class T>
T TD3DMatrix<T>::Get(size_t i,size_t j,size_t k)
     {
      if (i<_width && j<_height && k < _depth)
      return items[k][i][j];
      return 0;
     }


template <class T>
T& TD3DMatrix<T>::Direct(size_t i,size_t j,size_t k)
	{
      if (i<_width && j<_height && k < _depth)
      return (items[k][i][j]);
      return invalid;
     }

template <class T>
void TD3DMatrix<T>::Inc(size_t i,size_t j,size_t k,T t)
{
            if (i<_width && j<_height && k < _depth) items[k][i][j] += t;
     }


template <class T>
bool TD3DMatrix<T>::AddAt(size_t i,size_t j,size_t k,T t)
     {
      if (i<_width && j<_height && k < _depth)
       {
	items[k][i][j]=t;
	return true;
       }
      return false;
     }

template <class T>
void TD3DMatrix<T>::Flush(T t)
  {
  for (size_t k=0; k< _depth; k++)
   for (size_t i = 0; i < _width ; i++)
     for (size_t j=0; j < _height; j++)
      {
      items[k][i][j]=t;
      }
  }

template <class T>
void TD3DMatrix<T>::Copy(TD3DMatrix<T> & c)
  {
  Init(c.Width(),c.Height(),c.Depth());
  for (size_t k=0; k< _depth; k++)
   for (size_t i = 0; i < _width ; i++)
     for (size_t j=0; j < _height; j++)
      {
      items[k][i][j]=c.Get(i,j,k);
      }
  }

// TDDiagMatrix, a diagonal version of TDMatrix, more efficient

#define DIAGDIMENSION(i,j) if (j > i) _SWAP (i,j)
template <class T>
TDDiagMatrix<T>::TDDiagMatrix()
{_width=_height=0;
 items=0;
}

template <class T>
void TDDiagMatrix<T>::Init(size_t w)
    {
    Done();
      _width=w;
      items=new T*[w+1];
      for (size_t i=0 ; i <= w ; i++)
	{
	 items[i]=new T[i+1];
	 memset(items[i],0,sizeof(T) * (i+1));
	};
    }

template <class T>
const size_t TDDiagMatrix<T>::MaxInCol(size_t i,size_t j,size_t maxj)
//i,start are coordinates of column, going down
 {
 DIAGDIMENSION(i,j);
 int v=0;
 size_t c=NOT_FOUND;
 if (i >= _width) return c;
 T * t = items[i];
 maxj = min(_height,maxj);
 for ( ;j< maxj; j++)
  {
   if (t[j] > v) {c=j; v=t[j];}
  }
 return c;
 }

template <class T>
const size_t TDDiagMatrix<T>::MaxInDiag(size_t i,size_t j,bool* skip)
//i,start are coordinates of column, going down
 {
 DIAGDIMENSION(i,j);
 int v=0,c=NOT_FOUND;
 if (i >= _width) return c;

  for ( ; i < _width; j++)
   for ( ; j < _height; j++)
    {
      if (skip[j]) if (items[i][j] > v) {c=j; v=items[i][j];}
    }
   return c; // returns the row belonging to the greatest count.
 }


template <class T>
TDDiagMatrix<T>::~TDDiagMatrix()
    {
     Done();
    }

template <class T>
void TDDiagMatrix<T>::Done()
     {
     if (!items) return;
//     Flush();
      for (size_t i=0;i<_width;i++)
	{
	 delete [] items[i];
	};
      delete [] items;
     }

template <class T>
void TDDiagMatrix<T>::Inc(size_t i,size_t j,T t)
     {
      DIAGDIMENSION(i,j);
      if (i<_width) items[i][j] += t;
     }


template <class T>
T TDDiagMatrix<T>::Get(size_t i,size_t j)
     {
 DIAGDIMENSION(i,j);
      if (i<_width)
      return items[i][j];
      return 0;
     }


template <class T>
T& TDDiagMatrix<T>::Direct(size_t i,size_t j)
	{
 DIAGDIMENSION(i,j);
      if (i<_width)
      return (items[i][j]);
      return invalid;
     }

template <class T>
bool TDDiagMatrix<T>::AddAt(size_t i,size_t j,T t)
     {
 DIAGDIMENSION(i,j);
      if (i<_width)
       {
	//T* del=items[i][j];
	//if (del!=0) delete del;
	items[i][j]=t;
	return true;
       }
      return false;
     }

template <class T>
void TDDiagMatrix<T>::Flush(T t)
  {
   for (size_t i = 0; i < _width ; i++)
     for (size_t j=0; j <= i; j++)
      {
      items[i][j]=t;
      }
  }


#endif