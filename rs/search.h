#ifndef _RS_SEARCH_H
#define _RS_SEARCH_H
/*!
    file:   boyer.h
    desc:   Boyer-Moore text search algorithm (Windows version) header
    by:     Patrick Ko
    date:   6 Mar 91 - born
    revi:   4 Apr 94 - port Windows 3.1
    note:   use huge pointers to cater for big contiguous memory
*/
#include <stdlib.h>
#include <string.h>

class TStringSearch
{public:
 virtual size_t Find( const char *stext) = 0;
};

class TStringListSearch
{public:
 TStringList * list;
 TIndexList order;

 TStringListSearch(TStringList*l);

 ~TStringListSearch();

 void Sort();

 size_t FindStart(const char * text);
};

///uses the fastest-known algorightm for substring searches
class TSubstringSearch: public TStringSearch
{
private:
  int  skip[256];
  ///even byte boundaries are faster, one dummy byte extra
  unsigned char p[258];
  size_t plen;
  bool cs;

public:
  ///returns NOT_FOUND if not found;
  TSubstringSearch(const char* Pattern , bool CaseSensitive = true);

  size_t FindFirst(const char * s, size_t length);
  size_t Length() {return plen;}

  size_t Find( const char *stext)
   { return FindFirst(stext,strlen(stext)); }

protected:
  size_t FindCI(const char* s, size_t length);
  size_t FindCS(const char* s, size_t length);
};

#define NSUBEXP  10

class TRegExpSearch: public TStringSearch
{
struct regexp {
	const char *startp[NSUBEXP];
	const char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Internal use only. */
};

 const char *regparse;	/* Input-scan pointer. */
 int regnpar;		        /* () count. */
 char regdummy;
 char *regcode;		    /* Code-emit pointer; &regdummy = don't. */
 long regsize;		    /* Code size. */

 const char *reginput;		    /* String-input pointer. */
 const char *regbol;		/* Beginning of input, for ^ check. */
 const char **regstartp;	    /* Pointer to startp array. */
 const char **regendp;		    /* Ditto for endp. */

 int regerror;
 regexp *regcomp(const char *exp);
 int regexec(register regexp* prog, register const char *string);

 char *reg(int paren, int *flagp);
 char *regbranch(int*flagp);
 char *regpiece(int *flagp);
 char *regatom(int *flagp);
 char *regnode(int op);
 char *regnext(register char * p);
 void regc(int);
 void reginsert(char op, char *);
 void regtail(char*,char*);
 void regoptail(char*,char*);
 int regtry(regexp *prog,const char *string);
 int regmatch(char*);
 int regrepeat(char*);

protected:
 regexp* Reg;

public:
 TRegExpSearch( const char* exp);
~TRegExpSearch();

 size_t Find( const char *stext);
 bool Find( const char *stext,size_t &start, size_t& finish);
};
#endif
