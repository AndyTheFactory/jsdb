#include "rslib.h"
#include "io_fifo.h"
#pragma hdrstop

/*
   by Jason M Sachs  7/11/2008
 */

#ifndef NO_MEMORY_STREAM

FIFOStream::FIFOStream()
 : Stream(ReadWrite), _inpos(0), _outpos(0), maxsize(1024)
 {
 }

FIFOStream::FIFOStream(long size)
: Stream(ReadWrite), _inpos(0), _outpos(0), maxsize(size)
 {
	 if (maxsize < 0)
		 maxsize = 0;
 }

FIFOStream::~FIFOStream()
 {
 }

void FIFOStream::Resize(long size)
 {
	maxsize=size;
 }


long FIFOStream::seek(long offset)
 {
  // undefined operation
  return _outpos;
 }

long FIFOStream::goforward(long delta)
 {
	 pull(NULL,delta);
	 return _outpos;
 }

long FIFOStream::putback(long delta)
 {
	 // not possible
	return _outpos;
 }

long FIFOStream::size()
 {
    return maxsize;
 }

bool FIFOStream::eof()
 {
	 // hmm. EOF has different meanings for reading out & writing in.
	return false;
 }

long FIFOStream::pos()
 {
  return _outpos; // does this make sense?
 }


#endif
