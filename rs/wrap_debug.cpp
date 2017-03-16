#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

#include "js/src/jsdbgapi.h"
#include "js/src/jscntxt.h"

/** \file
Debugger commands
LINE filename#line
  returns script source code (one line only)
SOURCE filename#line
  returns script source code (entire compilation unit)
SOURCE filename
  returns all scripts for that filename
SCRIPTS
  lists scripts: filename#lines version
BREAK filename#line
  sets breakpoint
CLEAR filename
  removes a breakpoint
BREAK
  lists breakpoints
STACK
  requests the current stack
RUN
  runs until a breakpoint or error
STEP
  sets line-by-line mode
OVER
  Step over the next funcion call
OUT
  runs until the stack gets shorter
SKIP
  steps over the next function call
RETURN [rval]
  exit the current scope. Invalid after STOP FUNCTION|DONE
STOP
INSPECT object
  toSource. object=null for current stack frame
THROW error
EVALUATE length [code]

Debugger responses
MSG message\n
  status messages, no action required
OK length [data]
  last request succeeded. Stopped, action required
ERROR message \n
  last request failed. Stopped, action required
STOP reason file#line:col errormsg\n
  execution stopped, reason = DONE|BREAK|STEP|FUNCTION

Version 1.7.3

*/

class Debugger
{
 public:
  struct Script
  {
   JSScript* script;
   TStr name;
   TChars source;
   int start, end, version;
   bool isBase;
  };
  TList<Script> Scripts;

  struct Breakpoint
  {
   JSScript* script;
   TStr name;
   int line;
  };
  TList<Breakpoint> Breakpoints;

  JSScript* lastStop;
  int lastLine;
  bool Started;
  JSStackFrame* frameStop;

  JSRuntime* rt;
  TPointer<Stream> control;
  enum EMode {Run, Stepping, Stepover, Stepout, Break} mode;

  Debugger(JSRuntime* r) :Scripts(), Breakpoints()
  {
   rt = r;
   mode = Run;
   lastLine = 0;
   lastStop = 0;
   frameStop = 0;
   Started = false;
  }

  ~Debugger()
  {
  }

  JSTrapStatus Debug(JSContext* cx,jsval *rval=0,JSStackFrame *fp=0);

  void PrintObject(JSContext* cx,Stream& result, jsval *rval,bool summary=false);
  void SetBreakpoint(JSContext* cx,const char* name, uintN line);
  void RemoveBreakpoint(JSContext* cx,const char* name, uintN line);
  char* ScriptName(JSScript* s);
};

/** call ScriptSource() just before JS_ExecuteScript() to register the source
    code with the debugger
*/
void DebugScriptSource(void* opaque,const char* name, int start, const char* data, size_t length)
{
 Debugger* db = (Debugger*)opaque;
 if (!db) return;
 Debugger::Script* s = new Debugger::Script;
 s->name = name;
 s->start = start;
 s->end = 0;
 s->script = 0;
 s->isBase = true;
 if (length && length < (1024*1024))
  s->source.Set(data,length);
 db->Scripts.Add(s);
}

JSBool jsdb_DebugErrorHook(JSContext *cx, const char *message,
                           JSErrorReport *report, void *opaque)
{
  Debugger* db = (Debugger*)opaque;
  if (!db) return JS_TRUE;

  FOREACHBACK(Debugger::Breakpoint *b, db->Breakpoints)
   if (b->script) JS_ClearScriptTraps(cx,b->script);
   db->Breakpoints.Remove(i);
  DONEFOREACH

   if (JSREPORT_IS_WARNING(report->flags))
    {
     *db->control << "MSG Warning ";
     if (JSREPORT_IS_STRICT(report->flags))
      *db->control << " (strict) ";
    }
  else
   *db->control << "MSG Error  ";

  if (report->filename)
    *db->control << report->filename;
  if (report->lineno)
    *db->control << "#" << report->lineno;

  if (report->tokenptr && report->linebuf)
    *db->control << ":" << (int)(long)(report->tokenptr-report->lineno);

  if (message)
    *db->control << "\nMSG " << message;

  if (report->linebuf)
    *db->control << "\nMSG " << report->linebuf;

  if (report->tokenptr)
   {
    *db->control << "\nMSG " << report->linebuf;
    for (const char*c = report->linebuf; c < report->tokenptr; c++)
      db->control->write(".",1);
    db->control->write("^",1);
   }

  *db->control << "\n" << report->linebuf;
  return JS_FALSE; /* veto the standard error reporter */
}

char* Debugger::ScriptName(JSScript* s)
{
 if (!control) return (char*)"";
  FOREACH(Debugger::Script* s1, Scripts)
   if (s1->script == s)
    return s1->name;
  DONEFOREACH
  return (char*)"";
}

void jsdb_NewScriptHook(JSContext  *cx,
                        const char *name,  /* URL of script */
                        uintN      line,     /* line script starts */
                        JSScript   *script,
                        JSFunction *fun,
                        void       *opaque)
{
 Debugger* db = (Debugger*)opaque;
 if (!db) return;
 if (!name || !*name) return;
 // don't save scripts created by EVALUATE or INSPECT
 Debugger::Script* s = 0;
 int extent = JS_GetScriptLineExtent(cx, script);
 FOREACH(Debugger::Script* s1, db->Scripts)
  if (s1->script == 0 && s1->isBase && s1->name == name && s1->start == line)
    {s = s1; break;}
 DONEFOREACH
 if (!s)
 {
  Debugger::Script* s2 = 0;
  FOREACH(Debugger::Script* s1, db->Scripts)
   if (s1->name == name && s1->isBase)
     {s2 = s1; break;}
  DONEFOREACH
  s = new Debugger::Script;
  s->isBase = false;
  db->Scripts.Add(s);
  if (s2)
  {
   char * start = s2->source.buf;
   size_t l = line, size=s2->source.size;
   size_t i=0;
   while (i < size && l)
    if (start[i++] == '\n') l--;
   l = extent;
   size_t j = i;
   while (j < size && l)
    if (start[j++] == '\n') l--;
   if (j != i)
    s->source.Set(start + i,j - i);
  }
 }
 s->name = name;
 s->start = line;
 s->end = line + extent -1;
 s->version = LOWORD(JS_GetScriptVersion(cx, script));
 s->script = script;
 *db->control << "MSG LOAD "<<name<<"#"<<line<<"-"<<s->end<<"\n";
// db->Debug(cx);
}

/* called just before script destruction */
void jsdb_DestroyScriptHook(JSContext *cx,
                            JSScript  *script,
                            void      *opaque)
{
 Debugger* db = (Debugger*)opaque;
 if (!db) return;
 FOREACHBACK(Debugger::Breakpoint *b, db->Breakpoints)
  if (b->script == script)
  {
     db->Breakpoints.Remove(i);
  }
 DONEFOREACH

 JS_ClearScriptTraps(cx,script);

 FOREACHBACK(Debugger::Script *s, db->Scripts)
   if (s->script == script)
   {
    *db->control << "MSG UNLOAD "<<s->name<<"#"<<s->start<<"-"<<s->end<<"\n";
     db->Scripts.Remove(i);
   }
 DONEFOREACH
}


//called at every execution point
JSTrapStatus jsdb_TrapHandler(JSContext *cx, JSScript *script, jsbytecode *pc,
                                  jsval *rval, void *opaque)
{
  Debugger* db = (Debugger*)opaque;
  if (pc && *pc != 0 && *pc != 125)
    db->Started = true;
  if (!db->Started) return JSTRAP_CONTINUE;

  if (db->frameStop != cx->fp &&
   (db->mode == Debugger::Stepout || db->mode == Debugger::Stepover))
      return JSTRAP_CONTINUE;

  if (db->mode == Debugger::Run) return JSTRAP_CONTINUE;

  int line = JS_PCToLineNumber(cx, script, pc);
  if (
      (
       db->mode == Debugger::Stepping
       || db->mode == Debugger::Stepover
       || db->mode == Debugger::Stepout
      )
      && script == db->lastStop
      && line == db->lastLine
     )
   return JSTRAP_CONTINUE;

  db->lastStop = script;
  db->lastLine = line;

  if (db->mode == Debugger::Break)
      *db->control << "STOP BREAK ";
  else
      *db->control << "STOP STEP ";

  FOREACH(Debugger::Script *s, db->Scripts)
   if (s->script == script)
   {
     *db->control << s->name;
     if (line)
          *db->control << "#" << line;
     break;
   }
  DONEFOREACH
  *db->control << "\n";
  return db->Debug(cx,rval);
}

//gets called when a script starts to be executed
void*
jsdb_InterpreterHook(JSContext*     cx,
                     JSStackFrame*   fp,
                     JSBool         before,
                     JSBool         *ok,
                     void*          opaque)
{
  Debugger* db = (Debugger*) opaque;
  if (db->mode == Debugger::Run) return 0;

  if (db->frameStop != fp &&
   (db->mode == Debugger::Stepout || db->mode == Debugger::Stepover))
      return 0;

  *db->control << "STOP SCRIPT ";

  JSScript* s = JS_GetFrameScript(cx, fp);
  jsbytecode * pc = JS_GetFramePC(cx, fp);
  int line = 0;
  if (pc)
  {
    JS_PCToLineNumber(cx,s,pc);
  }

  FOREACH(Debugger::Script *script, db->Scripts)
   if (script->script == s)
   {
     *db->control << script->name << "#" << (line ? line : script->start);
     break;
   }
  DONEFOREACH
  db->lastStop = s;
  db->lastLine = line;

  *db->control << "\n";

  //ask the user what to do
  if (db->Debug(cx, 0, fp) == JSTRAP_ERROR)
  {//exit if possible
     if (ok) *ok = JS_FALSE;
     return 0;
  }

  if (before)
  {
   if (db->mode == Debugger::Stepover || db->mode == Debugger::Stepout)
   {
    JS_SetExecuteHook(db->rt, 0, 0);
    JS_SetInterrupt(db->rt, 0, 0);
    return opaque; //call when done to restore debugger state.
   }
   //running or stepping? no need to call again for this script.
   return 0;
  }
  else
  {
   db->mode == Debugger::Stepping;
   JS_SetExecuteHook(db->rt, jsdb_InterpreterHook, opaque);
   JS_SetInterrupt(db->rt, jsdb_TrapHandler, opaque);
   return 0;
  }
}

JSTrapStatus jsdb_BreakHandler(JSContext *cx, JSScript *script, jsbytecode *pc,
                                  jsval *rval, void *opaque)
{
  Debugger* db = (Debugger*)opaque;
  int line = JS_PCToLineNumber(cx, script, pc);

  db->mode = Debugger::Break;
  // traphandler will be called at the beginning of the js_Execute loop
  JS_SetInterrupt(db->rt, jsdb_TrapHandler, opaque);
  JS_SetExecuteHook(db->rt, jsdb_InterpreterHook, opaque);

  return JSTRAP_CONTINUE;
}

void* JSDB_StartDebug(JSRuntime* rt, const char* address, int port)
{
  Stream* s = 0;
  Debugger * db= 0;
 try {
  if (!port) port = 6000;
  Stream* s = new InternetStream(address, port);
printf("debug %s:%d\n",address,port);
  Debugger * db = new Debugger(rt);
  db->control = s;
  JS_SetNewScriptHook(rt, jsdb_NewScriptHook, db);
  JS_SetDestroyScriptHook(rt, jsdb_DestroyScriptHook, db);
  JS_SetDebugErrorHook(rt, jsdb_DebugErrorHook,db);
  JS_SetExecuteHook(rt, jsdb_InterpreterHook,db);
  //trap at the next interpreter execution point
  db->mode = Debugger::Stepping;
  return db;
 }
 catch(...)
 {
  if (s) delete s;
  if (db) delete db;
  return 0;
 }
}

void JSDB_DisableDebug(JSRuntime* rt,void* opaque)
{
 Debugger* db = (Debugger*)opaque;
 if (!db->control) return;
 *db->control << "STOP DONE\n";
 JS_SetNewScriptHook(rt, 0, 0);
 JS_SetDestroyScriptHook(rt, 0, 0);
 JS_SetDebugErrorHook(rt,0,0);
 JS_SetInterrupt(rt, 0, 0);
 JS_SetExecuteHook(rt, 0,0);
 db->control =0;
 db->Scripts.Clear();
}

void JSDB_EndDebug(JSRuntime* rt,void* opaque)
{
 Debugger* db = (Debugger*)opaque;
 *db->control << "STOP DONE\n";
 JS_SetNewScriptHook(rt, 0, 0);
 JS_SetDestroyScriptHook(rt, 0, 0);
 JS_SetDebugErrorHook(rt,0,0);
 JS_SetInterrupt(rt, 0, 0);
 JS_SetExecuteHook(rt, 0,0);
 delete db;
}

void Debugger::SetBreakpoint(JSContext* cx,const char* name, uintN line)
{
 if (!control) return;
  if (!line) line = 1;

  FOREACH(Script* s, Scripts)
   if (s->script
       && (*name && s->name == name)
       && s->start <= line
       && line <= s->end)
    {
     jsbytecode * pc = JS_LineNumberToPC(cx,s->script,line);
     if (pc)
      if (line == JS_PCToLineNumber(cx,s->script,pc))
       if (JS_SetTrap(cx,s->script,pc,jsdb_BreakHandler,this))
        {
         Breakpoint *bp = new Breakpoint;
         bp->name = name;
         bp->line = line;
         bp->script = s->script;
         Breakpoints.Add(bp);
        }
    }
  DONEFOREACH
}

void Debugger::RemoveBreakpoint(JSContext* cx,const char* name, uintN line)
{
 if (!control) return;
 FOREACHBACK(Breakpoint *b, Breakpoints)
  if (!*name || (b->name == name &&
      (!line || b->line == line)))
  {
     if (line)
     {
       jsbytecode * pc = JS_LineNumberToPC(0,b->script,line);
       if (!pc) return;
       JS_ClearTrap(cx,b->script,pc,0,0);
     }
     else
       JS_ClearScriptTraps(cx,b->script);

     Breakpoints.Remove(i);
  }
 DONEFOREACH
}

JSTrapStatus Debugger::Debug(JSContext* cx,jsval *rval,JSStackFrame *fp)
{
 if (!control) return JSTRAP_CONTINUE;
 TStr l;
 if (!cx) {*control<< "ERROR No context\n"; return JSTRAP_CONTINUE;}
 if (!fp)
   fp = cx->fp;
 if (!LOWORD((int)(long)fp) || JS_IsNativeFrame(cx,fp)) fp = 0;

 while(control->readline(l))
 {
 char* params = strchr(l,' ');
 if (params) *params++ = 0;
 else params = (char*)"";
 if (l == "BREAK" || l == "CLEAR")
 {
   uintN line = 0;
   bool a = *params;
   char* d = strrchr(params,'#');
   if (d)
    {
     line = atoi(d+1);
     *d=0;
    }
   if (!a)
   {
    MemoryStream result;
    Breakpoint* last = 0;
    FOREACH(Breakpoint*b,Breakpoints)
     if (last)
      if (last->name == b->name && last->line == b->line)
       continue;
     result << b->name << "#" << b->line << "\n";
     last = b;
    DONEFOREACH
    result.rewind();
    *control << "OK " << result.size() << "\n";
    control->Append(result);
   }
   else
   {
    JSScript* comp=0;
    if (line <= 0) line = 1;
    if (fp && !params) params = ScriptName(JS_GetFrameScript(cx, fp));

    if (l == "BREAK")
     SetBreakpoint(cx,params,line);
    else
     RemoveBreakpoint(cx,params,line);
    *control << "OK 0\n";
   }
 }
 else if (l == "LINE" || l == "SOURCE")
 {
   MemoryStream result;
   bool lineOnly = (l == "LINE");
   char* range = strchr(params,'#');
   if (range) *range++ = 0;
   else range = (char*)"";
   int line = atoi(range);

   JSScript* comp=0;
   if (!*params && fp)
    comp = JS_GetFrameScript(cx, fp);

   if (lineOnly && !line && comp)
   {
     jsbytecode * pc = JS_GetFramePC(cx, fp);
     if (pc) line = JS_PCToLineNumber(cx,comp,pc);
   }

   if (!*params && !comp)
   {
    *control << "ERROR Frame not loaded, try again later\n";
   }
   else
   {
    FOREACHBACK(Script*s, Scripts) //find the appropriate sub-script
     if ((comp && comp == s->script)
         || ((!strcasecmp(s->name,params)
            && ((line == 0 && s->isBase)
                || (s->start <= line && line <= s->end)))))
     {
      ByteStream b(s->source.buf,s->source.size);
      int current = s->start;
      while (b.readline(l) && current <= s->end)
      {
       if (!lineOnly || current == line)
         result << current << " " << l << "\n";
       current++;
      }
      if (line) break;
     }
    DONEFOREACH
    result.rewind();
    *control << "OK " << result.size() << "\n";
    control->Append(result);
   }
 }
 else if (l == "SCRIPTS")
  {
    MemoryStream result;
    FOREACH(Script*s, Scripts)
     result << s->name << "#" << s->start << "-" << s->end ;
     switch (s->version)
     {
      case JSVERSION_1_0: result << "(1.0)"; break;
      case JSVERSION_1_1: result << "(1.1)"; break;
      case JSVERSION_1_2: result << "(1.2)"; break;
      case JSVERSION_1_3: result << "(1.3)"; break;
      case JSVERSION_1_4: result << "(1.4)"; break;
      case JSVERSION_ECMA_3: result << "(ECMA)"; break;
      case JSVERSION_1_5: result << "(1.5)";
#ifdef JSVERSION_1_6
      case JSVERSION_1_6: result << "(1.6)";
#endif
#ifdef JSVERSION_1_7
      case JSVERSION_1_7: result << "(1.7)";
#endif
#ifdef JSVERSION_1_8
      case JSVERSION_1_8: result << "(1.8)";
#endif
     }
     result  << "\n";
    DONEFOREACH
    result.rewind();
    *control << "OK " << result.size() << "\n";
    control->Append(result);
   }
  else if (l == "STACK")
  {
   MemoryStream result;
   JSStackFrame* f = fp;
   while (f)
   {
     if (JS_IsNativeFrame(cx,f)) break;
     JSScript* s = f->script; //JS_GetFrameScript(cx, f);
     jsbytecode * pc = JS_GetFramePC(cx, f);
     if (!pc) break;
     int line = JS_PCToLineNumber(cx,s,pc);

     FOREACH(Script *script, Scripts)
     if (script->script == s)
     {
       result << script->name << "#" << line << "\n";
       break;
     }
     DONEFOREACH
     JSStackFrame* g = JS_GetScriptedCaller(cx,f);
     if (g == f) break;
     f = g;
   }
   result.rewind();
   *control << "OK " << result.size() << "\n";
   control->Append(result);
 }
 else if (l == "INSPECT")
 {
  JSObject* obj = 0;
  jsval rval = JSVAL_NULL;
  MemoryStream result;
  EMode m = mode;
  mode = Run;

  if (!*params)
   {
    if (fp)
    {
     obj = JS_GetFrameCallObject(cx,fp);
     if (!obj)
      obj = JS_GetFrameScopeChain(cx,fp);
    }
    if (!obj)
     obj = JS_GetGlobalObject(cx);
   }
  else
   {
     if (fp)
        JS_EvaluateInStackFrame(cx, fp,params,strlen(params),0,0,&rval);
     else
        JS_EvaluateScript(cx,JS_GetGlobalObject(cx),params,strlen(params),0,0,&rval);
   }

  if (rval != JSVAL_NULL) //eval a specific item
  {
    if (JSVAL_IS_OBJECT(rval))
      obj = JSVAL_TO_OBJECT(rval);
     else
      PrintObject(cx,result,&rval);

    if (obj) result << "\n"; //more properties to follow
  }
  if (obj)
  {
   JSPropertyDescArray pda;
   if (JS_GetPropertyDescArray(cx, obj, &pda))
   {
    //name type value\n
    for (size_t i=0; i< pda.length; i++)
    {
     if (pda.array[i].flags & (JSPD_ERROR | JSPD_EXCEPTION)) continue;
     {
      JSString* js = JS_ValueToString(cx,pda.array[i].id);
      if (js) result << TStr(JS_GetStringChars(js));
     }
     if (JSVAL_IS_PRIMITIVE(pda.array[i].value))
     {
      result << " ";
      PrintObject(cx,result,&pda.array[i].value,true);
     }
//     result << " ";
//     PrintObject(result,pda[i].value,true);
     result << "\n";
    }
    JS_PutPropertyDescArray(cx,&pda);
   }
  }
  if (result.size())
  {
   result.rewind();
   *control << "OK " << result.size() << "\n";
   control->Append(result);
  }
  else
   *control << "OK 0\n";

  mode = m;
 }
 else if (l == "EVALUATE")
 {
  EMode m = mode;
  mode = Run;
  jsval rval; //overload previous rval
  MemoryStream result;

  if (fp && *params)
   {
    JS_EvaluateInStackFrame(cx, fp,params,strlen(params),0,0,&rval);
    PrintObject(cx,result,&rval);
   }
  result.rewind();
  *control << "OK " << result.size() << "\n";
  control->Append(result);
  mode = m;
 }
 else if (l == "STOP")
  {
   return JSTRAP_ERROR;
  }
 else if (l == "DONE")
 {
  goto unload;
 }
 else if (l == "RUN")
 {
  JS_ClearInterrupt(rt,0,0);
  mode = Run;
  return JSTRAP_CONTINUE;
 }
 else if (l == "STEP")
 {
  JS_SetInterrupt(rt,jsdb_TrapHandler,this);
  mode = Stepping;

  return JSTRAP_CONTINUE;
 }
 else if (l == "SKIP" || l == "OVER")
 {
  frameStop = fp;
  mode = Stepover;
  return JSTRAP_CONTINUE;
 }
 else if (l == "OUT")
 {
  if (fp && !JS_IsNativeFrame(cx,fp))
   frameStop = JS_GetScriptedCaller(cx,fp);
  mode = Stepout;
  return JSTRAP_CONTINUE;
 }
 else if (l == "RETURN")
 {
  EMode m = mode;
  mode = Run;
  if (*params && fp && rval)
   JS_EvaluateInStackFrame(cx, fp,params,strlen(params),0,0,rval);

  mode = m;
  return JSTRAP_RETURN;
 }
 else if (l == "THROW")
 {
  if (*params && rval)
    JS_SetPendingException(cx,STRING_TO_JSVAL(JS_NewStringCopyZ(cx,params)));
  mode = Run;
  return JSTRAP_ERROR;
 }
 else
  *control << "ERROR Unknown command:" << l << "\n";


 } // while

unload:
 //couldn't read a line? debugger must have shut down. Un-debug and resume.
 //good thing this isn't a virtual method!
 FOREACHBACK(Breakpoint *b, Breakpoints)
  if (b->script) JS_ClearScriptTraps(cx,b->script);
  Breakpoints.Remove(i);
 DONEFOREACH

 JSDB_DisableDebug(rt,this);
 return JSTRAP_CONTINUE;
}

void Debugger::PrintObject(JSContext* cx,Stream& result, jsval *rval,bool summary)
{
 if (!control) return;
    if (JSVAL_IS_BOOLEAN(*rval))
     result << "Boolean " << (JSVAL_TO_BOOLEAN(*rval)?"true":"false");
    else if (JSVAL_IS_INT(*rval))
     result << "Integer " << JSVAL_TO_INT(*rval);
    else if (JSVAL_IS_DOUBLE(*rval) || JSVAL_IS_NUMBER(*rval))
    {
     char c[128];
     sprintf(c,"%G",*JSVAL_TO_DOUBLE(*rval));
     result << "Double " << c;
    }
    else if (JSVAL_IS_NULL(*rval))
     result << "Null";
    else if (JSVAL_IS_VOID(*rval))
     result << "Void";
    else if (JSVAL_IS_STRING(*rval))
    {
     result << "String ";
     if (summary) result << JS_GetStringLength(JSVAL_TO_STRING(*rval));
     else
     {
      JSString* js = JSVAL_TO_STRING(*rval);
      if (js) result << TStr(JS_GetStringChars(js));
     }
    }
    else if (JSVAL_IS_OBJECT(*rval))
     result << "Object " << JS_GetObjectTotalSize(cx,JSVAL_TO_OBJECT(*rval));
    else
     result << "Unknown";
}
