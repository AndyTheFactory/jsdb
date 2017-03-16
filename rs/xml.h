#ifndef _RS_XML_H
#define _RS_XML_H

#ifndef _RS_STREAM_H
#include "rs/stream.h"
#endif

#ifndef _RS_SYSTEM_H
#include "rs/system.h"
#endif

#ifndef _RS_QUERY_H
#include "rs/query.h"
#endif

#ifdef RSLIB_EZSurvey
#define TXMLFile XMLStream

class XML: public TParameterList
{public:
 TStr Name;
 TList<XML> Children;
 MemoryStream Cdata;

 XML(const char * n): TParameterList(), Name(n) {}

 ~XML() {}

 size_t Add(XML* x) {return Children.Add(x);}
 XML* Remove(size_t n) {return Children.Remove(n);}
 XML* At(size_t n) {return Children.Get(n);}
 //! out will be made into a non-autodelete list
 void Find(TList<XML> &out,const char * name);

 XML* Find(const char* name,size_t *start = 0);

 XML* Find(const char* name,const char* field,const char* value,size_t *start = 0);

 void Find(TList<XML> &out,const char* name,QueryNode& Query);

 void Write(Stream& out);
};

/*! reads a single XML object from a stream, including all children
 The DTD describes a valid structure:
   name: child1,child2,child3
   child1: child4
 This allows the parser to bypass processing for tags with no valid children
*/

XML* ReadXML(Stream& in, const char* allowed,
             bool SkipHTML = true,TNameValueList* dtd=0);

class XMLStream : public Stream
//good for reading or writing files
{
 public:
 TStr system;

 protected:
 MemoryStream trailer;
 TParameterList Options;
 int32 startptr,endptr;
 MemoryStream * data;
 Stream* Parent;
 bool AutoDelete;
 void Init(const char * objname,bool OmitTitle, int32 index);

 public:
 XMLStream(Stream* s, bool AutoDelete,
           const char * objname, // "FORM"
           bool OmitTitle = false,
           int32 index=0);
 XMLStream(const char * filename, // "myfile.ezf"
           const char * objname, // "FORM"
           const char * DTD=0,  // http://www.raosoft.com/xml/form.dtd
           TType Type=Stream::ReadWrite,
           bool OmitTitle = false,
           int32 index=0);
             //you can look for the Nth instance of an object
             // ReadOnly or ReadWrite
 //opes a file in text mode, creates if it doesn't exist
 ~XMLStream();
 //you must always write <objname> data </objname>\n. the library only writes
 //section info for compressed data
 bool WriteCompressedSection(const char * objname,const char* title,Stream& in,
                             bool compress=false);
 int32 ReadSection(Stream& out);
  //reads out the entire section to a separate stream

 int read(char * dest,int maxcopy);

 int write(const char * src,int maxcopy);

 const char* filename() {return Parent->filename();}
 long size() {return data ? data->size() : Parent->size();}
 long pos(){return data ? data->pos() : Parent->pos();}
 bool eof(){return data ? data->eof() : Parent->eof();}
 long goforward(long x){return data ? data->goforward(x) : Parent->goforward(x);}
 long putback(long x){return data ? data->putback(x) : Parent->putback(x);}
 long seek(long x){return data ? data->seek(x) : Parent->seek(x);}
};

#endif
#endif
