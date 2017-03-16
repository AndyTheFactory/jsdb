#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

JSPointerBase::JSPointerBase(JSPointerBase* p)
 :Parent(p),Children(false)
 {
  if (Parent) Parent->AddChild(this);
 }

void JSPointerBase::AddChild(JSPointerBase *c)
{
 Children.Add(c);
}

void JSPointerBase::RemoveChild(JSPointerBase *c)
{
 if (!c) return;
 FOREACHBACK(JSPointerBase*d,Children)
  if (d && c) if (c == d)
   {
    c->Parent = NULL;
    Children.AddAt(NULL,i);
    return;
   }
 DONEFOREACH
}

JSPointerBase::~JSPointerBase()
{
}

void JSPointerBase::InvalidateChildren()
{
 FOREACH(JSPointerBase*c,Children)
  if (c)
  {
   c->Parent = NULL;
   c->Close();
  }
 DONEFOREACH
 Children.Flush(); //does not delete the pointers
}

JSBool JSBadClass(JSContext* cx)
 {JS_ReportError(cx,"Wrong object class"); return JS_FALSE;}

JSBool SuspendGC(JSContext*cx, JSGCStatus flags)
 {return JS_FALSE;}

void
rs_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
    GETENV;
    if (!Env->out) return;

    if (!report)
    {
    Env->out->writestr(message,"\n");
       return;
    }

    /* Conditionally ignore reported warnings. */
 //  if (JSREPORT_IS_WARNING(report->flags) && !Env->reportWarnings)
 //  return;

   TChars prefix;

   if (report->filename)
    prefix << report->filename << ":";

   if (report->lineno)
    prefix << report->lineno << "\t";

   // mark the error on screen
   if (report->tokenptr && report->linebuf)
   {
    *Env->out << ((char*)prefix) << report->linebuf;
    if (!strchr(report->linebuf,'\n') && *report->linebuf)
      *Env->out << "\n";

    *Env->out << ((char*)prefix);

    int j = 0;
    for (const char * c = report->linebuf; c != report->tokenptr ; c++)
    {
     if (*c == '\t')
     {
      for (int k = (j + 8) & ~7; j < k; j++)
       {
        *Env->out << ".";
       }
      continue;
     }
    *Env->out << ".";
    j++;
   }

   *Env->out << "^\n";
  }

  if (JSREPORT_IS_WARNING(report->flags))
    {
     *Env->out << "warning";
     if (JSREPORT_IS_STRICT(report->flags))
      *Env->out << " (strict)";
     *Env->out << ": ";
    }

   TStr mout(message);
   RemoveChar(mout,'\r',mout.length());
   RemoveChar(mout,'\n',mout.length());

   Env->out->writestr(prefix,mout,"\n");
}

extern jsval* nameList(JSContext *cx,Strings& names, int& count)
{
 count = names.Count();
 if (!count) return 0;
 jsval * arr = new jsval[count];
 FOREACH(const char* c, names)
  arr[i] = STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx,WStr(c)));
 DONEFOREACH
 return arr;
}


bool JSDBEnvironment::Startup(int memSize, int stackSize,void* global_private)
{
 if (!rt)
 {
   rt = JS_NewRuntime(memSize);
   if (!rt) return false;
   AutoDeleteRuntime = true;
 }
 if (!cx)
 {
   cx = JS_NewContext(rt, stackSize);
   if (!cx) return false;
   AutoDeleteContext = true;
 }

 InitGlobal(global_private);
 DefineProcessFunctions();
 DefineShellFunctions();
 JS_InitStandardClasses(cx, global);
 InitClasses();
 return true;
}

void JSDBEnvironment::InitClasses()
    {
     Stream_InitClass(cx, global);
     Record_InitClass(cx, global);
#ifndef JSDB_MINIMAL
     Table_InitClass(cx, global);
     Index_InitClass(cx,global);
     Server_InitClass(cx, global);
#ifdef RSLIB_EZSurvey
	 Form_InitClass(cx,global);
#endif
     Mail_InitClass(cx, global);
     Image_InitClass(cx, global);
     Archive_InitClass(cx, global);
     Num_InitClass(cx, global);
#ifndef TBL_NO_SQL
     ODBC_InitClass(cx, global);
#endif
#ifndef TBL_NO_SQLITE
     SQLite_InitClass(cx, global);
#endif
#ifdef XP_WIN
     ActiveX_InitClass(cx, global);
#endif
     Process_InitClass(cx, global);
#endif
    }

JSDBEnvironment::~JSDBEnvironment()
{
 if (AutoDeleteContext && cx) {JS_GC(cx); JS_DestroyContext(cx);}
 if (AutoDeleteRuntime && rt) JS_DestroyRuntime(rt);
 JS_ShutDown();
}

JSBool JSDBEnvironment::ExecScript(void*cx, void* obj, ::Stream& data, const char* filename, int line)
{
   MemoryStream text;
   text.Append(data);

   return ExecScript(cx, obj, text, text.size(),filename,line);
}

JSBool JSDBEnvironment::ExecScript(void*context, void* obj, const char* text, size_t length, const char* filename, int line)
{
bool ok = false;
   if (!obj) obj = global;
   JSContext* cnx = context ? (JSContext*)context : cx;
#ifndef JSDB_MINIMAL
   if (Debugger)
    DebugScriptSource(Debugger,filename,line,text,length);
#endif

//  if (JS_BufferIsCompilableUnit(cx, global, text, text.size()))
    {
     jsval rval;
     uint16* l = (uint16*)(void*)(char*)text;

     if (*l == 0xFEFF)
     {
        int size = length/2 -1;
        l++;
        if (l[0] == '#' && l[1] == '!')
        {
          line++;
          while (size && *l && *l != '\n') {l++; size--;}
          if (size) if (*l) {l++; size--;}
        }
        ok = JS_EvaluateUCScript((JSContext*)cnx, (JSObject*)obj, l , size,filename,line,&rval);
     }
     else
     {
        char* t = (char*)text;
        int size = length;
        if (t[0] == '#' && t[1] == '!')
        {
          line++;
          while (size && *t && *t != '\n') {t++; size--;}
          if (size) if (*t) {t++; size--;}
        }

        size_t len = UTF8ToUCS2Length(t,size);
        l = new uint16[len + 2];
        len = UTF8ToUCS2(t,l,size);

        ok = JS_EvaluateUCScript((JSContext*)cnx, (JSObject*)obj, l, len, filename,line,&rval);
        delete[] l;
     }
    }
//     JSScript* script = JS_CompileScript(cx,obj,text,text.size(),filename,line);
//     JSObject* o2 = JS_NewScriptObject(cx,script);
//     ok = JS_CallFunctionName(cx,o2,"exec",0,0,&rval);
//else if (!JS_IsExceptionPending(cx))
// {
//  JS_ReportError(cx,"File %s is not compilable.",filename);
// }

   if (!ok && !JS_IsExceptionPending(cnx))
   {
    if (errorOnFailure)
        return JS_FALSE;
    JS_ReportError(cnx,"Execution error in %s.",filename);
    return JS_FALSE;
   }

 return ok;
}
