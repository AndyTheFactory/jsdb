#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop
#include "js/src/jsdtoa.h"
#include <math.h>

class NumberArray
{
 public:
  double *data;
  unsigned length;
  unsigned maxLength;
  NumberArray(NumberArray&o) : length(o.length), maxLength(o.length)
   {
     data = new double[length];
     memcpy(data,o.data,length * sizeof(double));
   }
  NumberArray(unsigned l) : length(l), maxLength(l)
   {
     data = new double[length];
     memset(data,0,length * sizeof(double));
   }
  ~NumberArray()
   {
     delete [] data;
   }

  void resize(unsigned l) //can throw a memory exception
  {
    if (l < maxLength)
    {
     if (l > length)
         memset(data+length,0,(l - length)*sizeof(double));
    }
    else if (l > maxLength)
    {
     double* d = new double[l];
     memcpy(d, data, length * sizeof(double));
     memset(d+length, 0, (l-length) * sizeof(double));
     delete [] data;
     data = d;
     maxLength = l;
    }
    length = l;
  }
};


JSClass* Num_Class();

/**
n = new Numbers("1 3 4 5 6 7")  //read a string
m = new Numbers(n)              //copy constructor
n = new Numbers(445)            //initialize to zero
n = new Numbers(stream,"uint16",length) //needs work
*/
WRAP(Num,Numbers)
{
  if (!argc)
  {
   JS_ReportError(cx,"Need a parameter for new Numbers()");
   return JS_FALSE;
  }

  int32 x = 0;
  if (JSVAL_IS_STRING(argv[0])) //read a delimited list
  {
   int delta = 128;
   int err=0;
   double d;
   char*c = STR(0); //JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
   //no need to convert to UTF-8, since JS_strtod only works on ASCII anyway
   //first, try to count the number of elements
   //valid characters are 0-9+-[eEfFgG].
   {
   char*d = c;
   const char* valid = "0123456789.+-eEfFgG";
   x=1;
   while (*d)
   {
    if (*d == ' ' || *d == ',' || *d == '\t' || !strchr(valid,*d))
    {
     d++;
     continue;
    }
    while (*d)
    {
     if (!strchr(valid,*d)) break;
     d++;
    }
    x++;
    d++;
   }
   }
   NumberArray* n = new NumberArray(x);
   x=0;
   while (*c)
   {
    d = JS_strtod(c, &c, &err);
    if (*c) c++;
    if (err) continue; //bad character?

    if (x >= n->length) n->resize(n->length + delta);
    n->data[x++] = d;
   }
   n->resize(x);
   JS_SetPrivate(cx, obj, n);
  }
  else if (JSVAL_IS_OBJECT(argv[0]))
  {
   JSObject* jx = JSVAL_TO_OBJECT(argv[0]);
   if (!jx)
   {
      JS_ReportError(cx,"Null passed to Numbers()");
      return JS_FALSE;
   }

   if (JS_InstanceOf(cx,jx,Num_Class(),0))
   {
     NumberArray* d =(NumberArray*)JS_GetPrivate(cx, jx);
     if (!d)
     {
      JS_ReportError(cx,"Deleted object passed to Numbers()");
      return JS_FALSE;
     }
     NumberArray* n = new NumberArray(*d);
     JS_SetPrivate(cx, obj, n);
   }
   else if (JS_InstanceOf(cx,jx,Stream_Class(),0))
   {
      Stream* in;
      GETFILE(0,in);

   }
   else
   {
      JS_ReportError(cx,"Wrong object type for Numbers()");
      return JS_FALSE;
   }

  }
  else if (JS_ValueToInt32(cx,argv[0],&x) && x > 0)
  {
    NumberArray* n = new NumberArray(x);
    JS_SetPrivate(cx, obj, n);
  }
  else
  {
   JS_ReportError(cx,"Numbers() needs a length or a list of numbers (%d)",x);
   return JS_FALSE;
  }

  return JS_TRUE;
}

JSBool
Num_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  if (!JSVAL_IS_INT(id))
   return JS_PropertyStub(cx,obj,id,rval);

 NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
 if (!n) return JS_TRUE;

 int32 x = JSVAL_TO_INT(id);

 if (x == -1)
 {
  *rval = INT_TO_JSVAL(n->length);
  return JS_TRUE;
 }
 if (x < n->length)
 {
   return JS_NewDoubleValue(cx, n->data[x], rval);
 }
 return JS_TRUE;
}

JSBool
Num_JSSet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 if (GETCLASS != Num_Class() ) return JSBadClass(cx);
 if (!JSVAL_IS_INT(id))
  return JS_PropertyStub(cx,obj,id,rval);

 NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
 if (!n) return JS_TRUE;

 int32 x = JSVAL_TO_INT(id);

 if (x == -1)
 {
  *rval = INT_TO_JSVAL(n->length);
  return JS_TRUE;
 }
 if (x >= n->length)
 {
   try
   {
     n->resize(x+1);
   } catch(...)
   {
      JS_ReportError(cx,"Out of memory at array size %d",x+1);
    return JS_FALSE;
   }
 }
 if (x >= 0)
 {
   double d=0.0;
   if (!JS_ValueToNumber(cx,*rval,&d))
   {
      JS_ReportError(cx,"Value must be a number");
      return JS_FALSE;
   }
   n->data[x] = d;
   return JS_TRUE;
 }

 return JS_TRUE;
}

/** Options:
toString(delimiter, start, length)
*/
void Num_toString(JSContext *cx, double * data, jsval* rval, const char* delimiter, size_t start, size_t end)
{
    MemoryStream out;
    char s[32];
    bool sep = false;
    for (size_t i = start; i < end; i++)
    {
     if (sep) out.writestr(delimiter);
     else sep = true;
     out.writestr(JS_dtostr(s, sizeof(s), DTOSTR_STANDARD,0, data[i]));
    }
    *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, (char*)out, out.size()));
}

JSBool Num_JSConvert(JSContext *cx, JSObject *obj, JSType type, jsval *rval)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*) JS_GetPrivate(cx, obj);
  if (!n) return JS_TRUE;

  switch(type) {
  case JSTYPE_NUMBER:
    *rval = INT_TO_JSVAL(n->length);
    return JS_TRUE;
  case JSTYPE_BOOLEAN:
    *rval = BOOLEAN_TO_JSVAL((n->length > 0));
    return JS_TRUE;
  case JSTYPE_OBJECT:
    *rval = OBJECT_TO_JSVAL(obj);
    return JS_TRUE;
  case JSTYPE_STRING:
   {
    Num_toString(cx,n->data,rval,",",0,n->length);
    return JS_TRUE;
   }
  default:
    return JS_TRUE;
  }
}

/*
 * This function type is used for callbacks that enumerate the properties of
 * a JSObject.  The behavior depends on the value of enum_op:
 *
 *  JSENUMERATE_INIT
 *    A new, opaque iterator state should be allocated and stored in *statep.
 *    (You can use PRIVATE_TO_JSVAL() to tag the pointer to be stored).
 *
 *    The number of properties that will be enumerated should be returned as
 *    an integer jsval in *idp, if idp is non-null, and provided the number of
 *    enumerable properties is known.  If idp is non-null and the number of
 *    enumerable properties can't be computed in advance, *idp should be set
 *    to JSVAL_ZERO.
 *
 *  JSENUMERATE_NEXT
 *    A previously allocated opaque iterator state is passed in via statep.
 *    Return the next jsid in the iteration using *idp.  The opaque iterator
 *    state pointed at by statep is destroyed and *statep is set to JSVAL_NULL
 *    if there are no properties left to enumerate.
 *
 *  JSENUMERATE_DESTROY
 *    Destroy the opaque iterator state previously allocated in *statep by a
 *    call to this function when enum_op was JSENUMERATE_INIT.
 *
 * The return value is used to indicate success, with a value of JS_FALSE
 * indicating failure.
 *
 * You shouldn't ever return JS_FALSE.
 * idp is null on the first call when running a for .. in loop.
 * JSENUMERATE_DESTROY never happens when running a for .. in loop.
 */
JSBool Num_JSEnumerate(JSContext *cx, JSObject *obj,
                                     JSIterateOp enum_op,
                                     jsval *statep, jsid *idp)
{
 int32 * x, u;
 if (GETCLASS != Num_Class() ) return JSBadClass(cx);
 NumberArray * n = (NumberArray*)JS_GetPrivate(cx, obj);
 if (!n) return JS_TRUE;

 switch (enum_op)
 {
  case JSENUMERATE_INIT:
    x = new int32;
    //allocate a new integer, because PRIVATE_TO_JSVAL drops the last bit
    *x = 0;
    *statep = PRIVATE_TO_JSVAL(x);
    if (idp) *idp = INT_TO_JSVAL(n->length);
    break;
  case JSENUMERATE_NEXT:
    x = (int32*)JSVAL_TO_PRIVATE(*statep);
    u = *x;
    if (u < n->length)
    {
     if (idp) *idp = INT_TO_JSVAL(u);
     *x = u + 1;
     break;
    }
    //else done -- cleanup.
  case JSENUMERATE_DESTROY:
    x = (int32*)JSVAL_TO_PRIVATE(*statep);
    delete x;
    *statep = JSVAL_NULL;
 };
 return JS_TRUE;
}

/*
 * Like JSResolveOp, but flags provide contextual information as follows:
 *
 *  JSRESOLVE_QUALIFIED   a qualified property id: obj.id or obj[id], not id
 *  JSRESOLVE_ASSIGNING   obj[id] is on the left-hand side of an assignment
 *
 * The *objp out parameter, on success, should be null to indicate that id
 * was not resolved; and non-null, referring to obj or one of its prototypes,
 * if id was resolved.
 *
 * note: objp is null when first called, but don't depend on that
 */
JSBool Num_JSResolve(JSContext *cx, JSObject *obj, jsval id,
              uintN flags, JSObject **objp)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray * n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n) return JS_TRUE;

  if (!JSVAL_IS_INT(id)) return JS_ResolveStub(cx,obj,id);

  int32 x = JSVAL_TO_INT(id);

  if (flags & JSRESOLVE_ASSIGNING) //Num_Set will expand if necessary
  {
    if (x < 0)
      return JS_TRUE;

    // Define the indexed property
    JS_DefineProperty(cx, obj, (char*)x, INT_TO_JSVAL(x), NULL, NULL,
              JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_INDEX);
    *objp = obj;
  }
  else if (flags & JSRESOLVE_DETECTING || flags & JSRESOLVE_QUALIFIED) //valid property exists with this id?
  {
    if (x >= n->length)
      return JS_TRUE;

    JS_DefineProperty(cx, obj, (char *) x, INT_TO_JSVAL(x), NULL, NULL,
              JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_INDEX);
    *objp = obj;
  }

  return JS_TRUE;
}

WRAP(Num,Get)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  int32 x;
  if (argc == 0 || !JSVAL_IS_INT(argv[0])) //!JS_ValueToInt32(cx,argv[0],&x) || x < 0)
  {
      JS_ReportError(cx,"Index must be a nonnegative integer");
      return JS_FALSE;
  }

  x = JSVAL_TO_INT(argv[0]);
  if (x < 0)
  {
      JS_ReportError(cx,"Index must be a nonnegative integer");
      return JS_FALSE;
  }

  if (x < n->length)
  {
   return JS_NewDoubleValue(cx, n->data[x], rval);
  }
  else
  {
      JS_ReportError(cx,"Index %d out of range (%d)",x,n->length);
      return JS_FALSE;
  }
}

bool NumOp(double* lhs, double* d1, double* d2, const char* op, size_t length)
{
 if (!strcmp(op,"+") && d1)
 {
  if (!d2) d2=lhs;
  for (size_t i=0; i<length; i++)
  {
   lhs[i] = d1[i] + d2[i];
  }
  return true;
 }
 if (!strcmp(op,"*") && d1)
 {
  if (!d2) d2=lhs;
  for (size_t i=0; i<length; i++)
  {
   lhs[i] = d1[i] * d2[i];
  }
  return true;
 }
 if (!strcmp(op,"-"))
 {
  if (!d1)
  {
   for (size_t i=0; i<length; i++)
   {
    lhs[i] = -lhs[i];
   }
   return true;
  }
  if (!d2) {d2=d1; d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    lhs[i] = d1[i] - d2[i];
  }
  return true;
 }

 if (!strcmp(op,"/"))
 {
  int err = 0;
  char*c = 0;
  const double infp = JS_strtod("Infinity",&c,&err);
  const double infm = JS_strtod("-Infinity",&c,&err);
  if (d1)
  {
  if (!d2) {d2=d1; d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    if (d2[i] == 0.0)
      lhs[i] = d1[i] > 0 ? infp : infm;
    else
      lhs[i] = d1[i] / d2[i];
  }
  return true;
 }
 else //invert
 {
  for (size_t i=0; i<length; i++)
  {
    if (lhs[i] == 0.0)
      lhs[i] = infp;
    else
      lhs[i] = 1.0/ lhs[i];
  }
  return true;
 }
}
 if (d1 && (!strcmp(op,"atan2")))
 {
  if (!d2) {d2=d1; d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    lhs[i] = atan2(d1[i], d2[i]);
  }
  return true;
 }
 if (d1 && (!strcmp(op,"pow")))
 {
  if (!d2) {d2=d1; d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    lhs[i] = pow(d1[i], d2[i]);
  }
  return true;
 }

 if (!strcmp(op,"atan"))
 {
  if (!d1) {d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    lhs[i] = atan(d1[i]);
  }
  return true;
 }
 if (!strcmp(op,"cos"))
 {
  if (!d1) {d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    lhs[i] = cos(d1[i]);
  }
  return true;
 }
 if (!strcmp(op,"sin"))
 {
  if (!d1) {d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    lhs[i] = sin(d1[i]);
  }
  return true;
 }
 if (!strcmp(op,"tan"))
 {
  if (!d1) {d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    lhs[i] = tan(d1[i]);
  }
  return true;
 }
 if (!strcmp(op,"exp"))
 {
  if (!d1) {d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    lhs[i] = exp(d1[i]);
  }
  return true;
 }
 if (!strcmp(op,"log"))
 {
  if (!d1) {d1 = lhs;}
  for (size_t i=0; i<length; i++)
  {
    lhs[i] = log(d1[i]);
  }
  return true;
 }

 return false;
}


/** Vector operations in RPN
  n.exec(a, b, '+')
  n.exec(a, b, '+'
  n.exec(a, 'cos')
  n.exec(n, 'cos')

  operators:  + - / * cos sin tan acos asin atan atan2 ^(power) log exp
*/

WRAP(Num,Exec)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  NumberArray * d1 = 0, *d2 = 0;
  const char* op;
//  size_t dCount = 0;
  JSObject* o;

  for(size_t x=0; (x < argc); x++)
  {
   if (JSVAL_IS_STRING(argv[x]))
   {
    op = STR(x); //JS_GetStringBytes(JSVAL_TO_STRING(argv[x]));
    if (!NumOp(n->data, d1 ? d1->data : 0, d2 ? d2->data : 0, op, n->length))
    {
      JS_ReportError(cx,"Equation error at step %d",x);
      return JS_FALSE;
    }
//    dCount = 0;
    d1 = d2 = 0;
    continue;
   }

   if (!JSVAL_IS_OBJECT(argv[x]))
   {
      JS_ReportError(cx,"Wrong object type for parameter %d in Numbers.exec()",x);
      return JS_FALSE;
   }
   JSObject* jx = JSVAL_TO_OBJECT(argv[x]);
   if (!jx)
   {
      JS_ReportError(cx,"Null passed for parameter %d to Numbers.exec()",x);
      return JS_FALSE;
   }

   if (!JS_InstanceOf(cx,jx,Num_Class(),0))
   {
      JS_ReportError(cx,"Wrong object type for parameter %d in Numbers.exec()",x);
      return JS_FALSE;
   }

   NumberArray* d =(NumberArray*)JS_GetPrivate(cx, jx);
   if (!d)
   {
      JS_ReportError(cx,"Deleted object for parameter %d in Numbers.exec()",x);
      return JS_FALSE;
   }

   if (d->length != n->length)
   {
      JS_ReportError(cx,"Length mismatch at parmeter %d in Numbers.exec() (%d != %d)",x,n->length,d->length);
      return JS_FALSE;
   }

   if (d1) d2 = d; else d1 = d;
  }

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}


//add
WRAP(Num,Add)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  *rval = JSVAL_NULL;
  if (argc == 0)
    return JS_TRUE;

  jsdouble d;
  if (!JS_ValueToNumber(cx, argv[0],&d))
  {
   JS_ReportError(cx,"Numbers.add() requires a number");
   return JS_FALSE;
  }

  for (size_t i=0; i<n->length; i++)
   n->data[i] += d;

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

//scalar multiply
WRAP(Num,Scale)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  *rval = JSVAL_NULL;
  if (argc == 0)
    return JS_TRUE;

  jsdouble d;
  if (!JS_ValueToNumber(cx, argv[0],&d))
  {
   JS_ReportError(cx,"Numbers.scale() requires a number");
   return JS_FALSE;
  }

  for (size_t i=0; i<n->length; i++)
   n->data[i] *= d;

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

//exponentiation
WRAP(Num,Pow)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  *rval = JSVAL_NULL;
  if (argc == 0)
    return JS_TRUE;

  jsdouble d;
  if (!JS_ValueToNumber(cx, argv[0],&d))
  {
   JS_ReportError(cx,"Numbers.pow() requires a number");
   return JS_FALSE;
  }

  for (size_t i=0; i<n->length; i++)
  {
      n->data[i] = pow(n->data[i],d);

  }

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

//log
WRAP(Num,Log)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  for (size_t i=0; i<n->length; i++)
   n->data[i] = log(n->data[i]);

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

//sum (start, range)
WRAP(Num,Sum)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  size_t start = 0;
  size_t end = n->length;

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    start = JSVAL_TO_INT(argv[0]);
    if (start >= n->length)
    {
      JS_ReportError(cx,"Start address out of bounds %d",start);
      return JS_FALSE;
    }
    argv++;
    argc--;
  }

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    end = start + JSVAL_TO_INT(argv[0]);
    if (end > n->length)
    {
      JS_ReportError(cx,"Length out of bounds %d + %d",start, JSVAL_TO_INT(argv[0]));
      return JS_FALSE;
    }
  }

  double r = 0.0;
  for (size_t i=start; i<end; i++)
  {
   r += n->data[i];
  }

   return JS_NewDoubleValue(cx, r, rval);
}

/* Proper way to calculate std dev, from Knuth
>   var S = 0;
>   var M = 0, M_prev;
>   var i = 0;
>   while (i < n.length)
>   {
>       M_prev = M;
>       var n_i = n[i];
>       ++i;
>       M += (n_i - M)/i;
>       S += (n_i - M)*(n_i - M_prev);
>   }
>   return {mean: M, stdev: Math.sqrt(S/n.length)};
*/

//sum (start, range)
WRAP(Num,Sum2)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  size_t start = 0;
  size_t end = n->length;

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    start = JSVAL_TO_INT(argv[0]);
    if (start >= n->length)
    {
      JS_ReportError(cx,"Start address out of bounds %d",start);
      return JS_FALSE;
    }
    argv++;
    argc--;
  }

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    end = start + JSVAL_TO_INT(argv[0]);
    if (end > n->length)
    {
      JS_ReportError(cx,"Length out of bounds %d + %d",start, JSVAL_TO_INT(argv[0]));
      return JS_FALSE;
    }
  }

  double r = 0.0;
  double d;
  for (size_t i=start; i<end; i++)
  {
   d = n->data[i];
   r += d*d;
  }

  return JS_NewDoubleValue(cx, r, rval);
}
//max (start, range) returns the index, not the value
WRAP(Num,Max)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  size_t start = 0;
  size_t end = n->length;

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    start = JSVAL_TO_INT(argv[0]);
    if (start >= n->length)
    {
      JS_ReportError(cx,"Start address out of bounds %d",start);
      return JS_FALSE;
    }
    argv++;
    argc--;
  }

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    end = start + JSVAL_TO_INT(argv[0]);
    if (end > n->length)
    {
      JS_ReportError(cx,"Length out of bounds %d + %d",start, JSVAL_TO_INT(argv[0]));
      return JS_FALSE;
    }
  }

  double r = n->data[start];
  double d;
  size_t pos = start;
  start++;
  for (size_t i=start; i<end; i++)
  {
   d = n->data[i];
   if (d <= r) continue;
   r = d;
   pos = i;
  }

  *rval = INT_TO_JSVAL(pos);
  return JS_TRUE;
}

//min (start, range) returns the index, not the value
WRAP(Num,Min)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  size_t start = 0;
  size_t end = n->length;

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    start = JSVAL_TO_INT(argv[0]);
    if (start >= n->length)
    {
      JS_ReportError(cx,"Start address out of bounds %d",start);
      return JS_FALSE;
    }
    argv++;
    argc--;
  }

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    end = start + JSVAL_TO_INT(argv[0]);
    if (end > n->length)
    {
      JS_ReportError(cx,"Length out of bounds %d + %d",start, JSVAL_TO_INT(argv[0]));
      return JS_FALSE;
    }
  }

  double r = n->data[start];
  double d;
  size_t pos = start;
  start++;
  for (size_t i=start; i<end; i++)
  {
   d = n->data[i];
   if (d >= r) continue;
   r = d;
   pos = i;
  }

  *rval = INT_TO_JSVAL(pos);
  return JS_TRUE;
}

/** Usage: n.toString(' ',start,length)
    Throws an error if an index is out of bounds
 */
WRAP(Num,ToString)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  const char* delimiter = ",";
  size_t start = 0;
  size_t end = n->length;

  if (argc && JSVAL_IS_STRING(argv[0]))
  {
    delimiter = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
    argv++;
    argc--;
  }

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    start = JSVAL_TO_INT(argv[0]);
    if (start >= n->length)
    {
      JS_ReportError(cx,"Start address out of bounds %d",start);
      return JS_FALSE;
    }
    argv++;
    argc--;
  }

  if (argc && JSVAL_IS_INT(argv[0]))
  {
    end = start + JSVAL_TO_INT(argv[0]);
    if (end > n->length)
    {
      JS_ReportError(cx,"Length out of bounds %d + %d",start, JSVAL_TO_INT(argv[0]));
      return JS_FALSE;
    }
  }

  Num_toString(cx,n->data,rval,delimiter, start, end);
  return JS_TRUE;
}

WRAP(Num,Set)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  int32 x;
  if (argc == 0 || !JSVAL_IS_INT(argv[0])) //!JS_ValueToInt32(cx,argv[0],&x) || x < 0)
  {
      JS_ReportError(cx,"Index must be a nonnegative integer");
      return JS_FALSE;
  }

  x = JSVAL_TO_INT(argv[0]);
  if (x < 0)
  {
      JS_ReportError(cx,"Index must be a nonnegative integer");
      return JS_FALSE;
  }

  if (x < n->length)
  {
   double d = 0.0;
   if (argc > 1)
   {
    if (!JS_ValueToNumber(cx,argv[1],&d))
    {
      JS_ReportError(cx,"Value must be a number");
      return JS_FALSE;
    }
   }
   n->data[x] = d;
   return JS_TRUE;
  }
  else
  {
      JS_ReportError(cx,"Index %d out of range (%d)",x,n->length);
      return JS_FALSE;
  }
}

WRAP(Num,Resize)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n)
  {
      JS_ReportError(cx,"Object already closed");
      return JS_FALSE;
  }

  int32 x;
  if (argc == 0 || !JS_ValueToInt32(cx,argv[0],&x) || x <= 0)
  {
      JS_ReportError(cx,"Length must be a positive integer");
      return JS_FALSE;
  }

  n->resize(x);
  *rval = INT_TO_JSVAL(n->length);
  return JS_TRUE;
}

WRAP(Num,Close)
{
  if (GETCLASS != Num_Class() ) return JSBadClass(cx);
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (!n) return JS_TRUE;

  JS_SetPrivate(cx,obj,0);
  delete n;
  *rval = OBJECT_TO_JSVAL(0);
  return JS_TRUE;
}

JS_PUBLIC_API(void) Num_JSFinalize(JSContext *cx, JSObject *obj)
{
  NumberArray* n = (NumberArray*)JS_GetPrivate(cx, obj);
  if (n) delete n;
}

struct JSPropertySpec Num_properties[] = {
  { "length", -1, JSPROP_READONLY|JSPROP_PERMANENT, 0, 0 },
  {0}
};

static JSFunctionSpec Num_functions[] = {
    {"close", Num_Close,0},
    {"at", Num_Get,1},
    {"put", Num_Set,2},
    {"get", Num_Get,1},
    {"set", Num_Set,2},
    {"toString", Num_ToString,0},
    {"resize", Num_Resize,1},
    {"scale", Num_Scale,1},
    {"add", Num_Add,1},
    {"max", Num_Max,2},
    {"min", Num_Min,2},
    {"sum", Num_Sum,2},
    {"log", Num_Log,0},
    {"pow", Num_Pow,1},
    {"sum2", Num_Sum2,2},
    {"exec",Num_Exec,10},
    {0}
};

static JSClass Num_class = {
    "Numbers", JSCLASS_HAS_PRIVATE | JSCLASS_NEW_ENUMERATE | JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub, Num_JSGet, Num_JSSet,
    (JSEnumerateOp)Num_JSEnumerate, (JSResolveOp)Num_JSResolve,
    Num_JSConvert, Num_JSFinalize
};

void Num_InitClass(JSContext *cx, JSObject *obj)
{
 JS_InitClass(cx, obj, NULL, &Num_class,
 Num_Numbers, 1,
 Num_properties, Num_functions, //object properties and functions
 NULL,NULL);            //static properties and functions
}

JSClass* Num_Class() {return &Num_class;}
