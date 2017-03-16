#ifndef _RS_QUERY_H
#define _RS_QUERY_H

#ifndef _RS_LIST_H
#include "rs/list.h"
#endif

#ifndef _RS_SYSTEM_H
#include "rs/system.h"
#endif

class QueryNode
 {public:
  enum EMode {Uninitialized,Comparison, Logic} Mode;

  ///comparisons understand regexp wildcards: . any character, * anything
  enum EComparison {Qnone,
                    Qequals, // could be text, number, or choice
                    Qequalsnum,Qgt,Qlt,Qlte,Qgte,
                    Qbetween, // including end points
                    Qequalstext,Qcontains,
                    Qcontainsany,Qcontainsall,
                    Qstartswith,Qendswith,
                    Qhas, // multiple choice questions
                    Qthismonth,Qthisyear,Qlastyear} comparison;
  // '=','>','>=','<=','!=',...
  int Negative;
  TDataSource * IsSetup;

  TStr cLeft, cRight;
  TChars cTemp;
  size_t fLeft, fRight;

  enum ELogic {QIF, QFALSE, QAND, QOR, QXOR, QTRUE} logic;
                   // & | &! |!
  TPointer<QueryNode> nLeft, nRight;

  QueryNode(const char* text=0);
  ~QueryNode();

  void Clear();
  inline bool Parse(const char * text);

  bool Evaluate(TNameValueList& d);
  bool Evaluate(TDataSource& d, count_t row);
  bool Setup(TDataSource& d);
 };

/*! Usage:

 QueryNode Query;
 xdb ParseError;
 if (!ParseQuery(Query,"Q1.Contains hello & Q2.Between(40,50)",ParseError))
   puts(ParseError.why());

 The general syntax for query text is

 (Fieldname.Operator text_value) & !(Fieldname > number_value)

 Operators include
  .ISNUMBER     number equality
  .ISTEXT       text equality (case sensitive)
  .ISCODE       multiple-response code (case sensitive, comma-delimited)
  .CONTAINSALL  contains all comma-delimited values (not cs)
  .CONTAINSANY  contains any comma-delimited value (not cs)
  .CONTAINS     contains literal text (not cs)
  .STARTSWITH   starts with text
  .ENDSWITH     ends with test
  .BETWEEN      between two number values (inclusive). good for date ranges

 Defined, but unused, are
  .THISMONTH
  .THISYEAR

 Other syntax operators are
  ==            numeric equality
  =             numeric equality, then text equality (not CS), then a code comparison
  >, >=, <, <=  numeric comparisons
  !=            negative of =
  !==           negative of ==
  !             unary negation of the next expression
  ()            recursive branching

*/

bool ParseQuery(QueryNode& q,const char * text,xdb& Error);
bool PrintQuery(QueryNode& q,Stream& text);

inline bool QueryNode::Parse(const char * text)
{
 xdb err;
 return ParseQuery(*this,text,err);
}
#endif
