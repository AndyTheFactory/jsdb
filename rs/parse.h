#ifndef _RS_PARSE_H
#define _RS_PARSE_H

#ifndef _RS_STREAM_H
#include "rs/stream.h"
#endif

#ifndef _RS_SYSTEM_H
#include "rs/system.h"
#endif

//HTML parsing functions
//sy_html.cpp

#define EOTAG ('>')
#define ENDLINE "\n"

/*
In XML, a tag must always have a terminator. For example, text is:

   This is <b> bold text </b>

The </b> tag terminates the <b> tag. But if you have a tag that stands by
itself, like a page break in EZF files, it needs to be self-terminating:

   <PAGE/>  or  <PAGE />

In case your parser cares about this, we have the EndTag parameter which tells
you if the terminator was included in a tag or not. EndTag will be true for
these types of tags:

  </INPUT>           (StartTag)
  <PAGE/>            (StartTag)
  <PAGE />           (FinishTag)
  <SKIP ... />       (FinishTag)

*/

//writing HTML/XML
void WriteTag(Stream& out, const char * type,
              TParameterList & Options,bool EndTag = false);

//start read HTML/XML
int ReadUntilTag(Stream& in,const char * tag,Stream * SkipText=0,bool *EndTag = 0);

int StartTag(Stream&in,TStr& type,Stream * SkipText=0,bool *EndTag = 0,const char* AllowedTags=0);

void PutBackTag(Stream& in,const char * tag,bool *EndTag = 0);

void SkipTag(Stream& in,int StartResult, Stream* TagText=0);

//finish read HTML/XML
bool FinishTag(Stream&in, int StartResult, TParameterList* params=0,bool *EndTag = 0);

#endif
