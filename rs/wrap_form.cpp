#pragma hdrfile "jsdb.csm"
#include "rslib.h"
#include "jsdb.h"
#include "rs/wrap_jsdb.h"
#pragma hdrstop

void Form_JSFinalize(JSContext *cx, JSObject *obj)
{
 DELPRIVATE(EZFForm);
}

JSBool
Form_Form(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
    jsval *rval)
{
 GETENV;
 if (!Env) return JS_FALSE;

// exit(EXIT_FAILURE);
 EZFForm * t = new EZFForm;

 try {

 if (argc == 1 || argc == 2&& ISSTR(0))
  {
   Stream* s0;
   GETFILE(0,s0);

   int32 index=0;
   if (s0)
   {
    XMLStream in(s0,false,"FORM");
    if (!OpenEZFFile(in, *t,NULL,NULL))
      {delete t; t = NULL;}
   }
   else if (ISSTR(0))
   {
    char* str = STR(0);
    if (!strncasecmp(str,"<?xml",5) || !strncasecmp(str,"<form",5))
    {
     ByteStream text(str);
     XMLStream in(&text,false,"FORM");
     if (!OpenEZFFile(in, *t,NULL,NULL))
       {delete t; t = NULL;}
    }
    else
    {
     if (argc == 2)
     {
      TOINT(1,index);
      JS_SetProperty(cx,obj,"file_id",argv + 1);
     }

     t->Filename = str;
     if (FileExists(t->Filename))
     {
      XMLStream in(t->Filename,"FORM",0,Stream::ReadOnly,false,index);
      if (!OpenEZFFile(in, *t,NULL,NULL))
       {delete t; t = NULL;}
     }
    }
   }
  }

 } catch (...) { if (t) delete t; t = NULL; }

 if (!t)
  {
  if (Env->errorOnFailure)
  {
   JS_ReportError(cx,"File read failure while creating a Form.");
   return JS_FALSE;
  }
  ERR_MSG(Form,"File read failure while creating a Form","Form");
 }

 if (t)
 {
 SETPRIVATE(obj,EZFForm,t,true,NULL);
// JS_SetPrivate(cx,obj,t);

 JSObject* options = Record_Object(cx,&t->Options,false,GETPOINTER);
 jsval opt = OBJECT_TO_JSVAL(options);
 JS_DefineProperty(cx, obj,"options",opt,NULL,NULL,JSPROP_READONLY|JSPROP_PERMANENT);
 }
 //the child will get deleted after the form does, but its pointer
 //will be invalid. shouldn't be a problem, though.
 return JS_TRUE;
}


WRAP(Form,Save)
{
    GETOBJ(Form,EZFForm,t);
    if (argc == 1)
    {
        if (!ISSTR(0))
        ERR_TYPE(Form,Save,1,string);
        t->Filename = STR(0);
    }
    int32 index=0;
    if (JS_GetProperty(cx,obj,"file_id",rval))
      index = JSVAL_TO_INT(*rval);

    if (argc == 2)
    {
        TOINT(1,index);
        JS_SetProperty(cx,obj,"file_id",argv+1);
    }

    if (!*t->Filename)
      RETBOOL(false);

    try {
        XMLStream out(STR(0),"FORM",0,Stream::ReadWrite,false,index);
        SaveEZFFile(*t,out,NULL,NULL);
        }
    catch(...)
        {t->Filename = 0; }

    RETBOOL(*t->Filename != 0);
}

JSBool
Form_JSGet(JSContext *cx, JSObject *obj, jsval id, jsval *rval)
{
 GETOBJ(Form,EZFForm,t);
 int x = JSVAL_TO_INT(id);

 if (JSVAL_IS_INT(id))
  switch (x)
  {
   case 0: RETSTRW(WStr(t->Filename));
   case 1: RETINT(t->Questions.Count());
   case 2: RETSTRW(L"Form");
   default: return JS_FALSE;
  }
 else return JS_FALSE;
}

static JSPropertySpec Form_properties[] = {
    {"name", 0,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Form_JSGet},
    {"count", 1,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Form_JSGet},
    {"className",2, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,Form_JSGet},
    {0}
};

static JSBool
EZFForm_Find(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 if (argc != 1) ERR_COUNT(EZFForm,Find);
 if (!ISSTR(0)) ERR_TYPE(EZFForm,Find,1,String);
 GETOBJ(Form,EZFForm,t);
 const char* comp = STR(0);
 FOREACH(EZFQuestion*q,t->Questions)
  if (q->FieldName == comp) RETINT(i);
 DONEFOREACH
 RETINT(-1);
}

EZFQuestion::EType GetType(const char* c)
{
      if (!strcasecmp(c,"Text")) return EZFQuestion::Text;
 else if (!strcasecmp(c,"Password")) return EZFQuestion::Password;
 else if (!strcasecmp(c,"Date")) return EZFQuestion::Date;
 else if (!strcasecmp(c,"Time")) return EZFQuestion::Time;
 else if (!strcasecmp(c,"Number")) return EZFQuestion::Number;
 else if (!strcasecmp(c,"Radio")) return EZFQuestion::Radio;
 else if (!strcasecmp(c,"Weighted")) return EZFQuestion::Weighted;
 else if (!strcasecmp(c,"Check")) return EZFQuestion::Checkbox;
 else if (!strcasecmp(c,"Single")) return EZFQuestion::ListSingle;
 else if (!strcasecmp(c,"Multiple")) return EZFQuestion::ListMulti;
 else if (!strcasecmp(c,"Combo")) return EZFQuestion::ListCombo;
 else if (!strcasecmp(c,"Rank")) return EZFQuestion::ListRank;
 else if (!strcasecmp(c,"Hidden")) return EZFQuestion::Hidden;
 else if (!strcasecmp(c,"Section")) return EZFQuestion::Section;
 else if (!strcasecmp(c,"Page")) return EZFQuestion::PageBreak;
 else if (!strcasecmp(c,"RichText")) return EZFQuestion::RichText;
 else if (!strcasecmp(c,"Image")) return EZFQuestion::Image;
 else if (!strcasecmp(c,"PlainText")) return EZFQuestion::PlainText;
 else if (!strcasecmp(c,"ToolButton")) return EZFQuestion::ToolButton;
 else return (EZFQuestion::EType)0;
}

static JSBool
EZFForm_addQ(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Form,EZFForm,t);
 if (argc < 2) ERR_COUNT(Form,AddQuestion);

 if (!ISSTR(0)) ERR_TYPE(Form,AddQuestion,1,String);
 if (!ISSTR(1)) ERR_TYPE(Form,AddQuestion,2,String);
 if (argc > 2 && !ISSTR(2)) ERR_TYPE(Form,AddQuestion,3,String);
 if (argc > 3 && !ISSTR(3)) ERR_TYPE(Form,AddQuestion,4,String);
 if (argc > 4 && !ISSTR(4)) ERR_TYPE(Form,AddQuestion,5,String);
 if (argc > 5 && !ISINT(5)) ERR_TYPE(Form,AddQuestion,6,Integer);
 if (argc > 6 && !ISSTR(6)) ERR_TYPE(Form,AddQuestion,7,String);
 if (argc > 7 && !ISINT(7)) ERR_TYPE(Form,AddQuestion,8,Integer);

 EZFQuestion::EType type = GetType(STR(0));
 if (!type) RETINT(-1);

 EZFQuestion* q = t->AddQuestion(type, STR(1),
                                       argc>2 ? STR(2) : 0,
                                       argc>3 ? STR(3) : 0,
                                       argc>4 ? STR(4) : 0,
                                       argc>5 ? INT(5) : -1,
                                       argc>6 ? STR(6) : 0,
                                       argc>7 ? INT(7) : -1);
 if (!q) RETINT(-1);
 RETINT(t->Questions.IndexOf(q));
}

static JSBool
Form_HasData(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Form,EZFForm,t);
 if (argc != 1) ERR_COUNT(EZFForm,HasData);
 if (!ISINT(0)) ERR_TYPE(EZFForm,HasData,1,Integer);
 EZFQuestion*q = t->Questions[INT(0)];
 if (q) RETBOOL(q->HasData());
 RETBOOL(false);
}

static JSBool
Form_SetModified(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Form,EZFForm,t);
 t->IsModified = true;
 RETBOOL(true);
}

static JSBool
Form_ToString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 GETOBJ(Form,EZFForm,t);

 RETSTRW(WStr(t->Filename));
}

WRAP(Form,getType)
{
 if (argc != 1) ERR_COUNT(Form,getType)
 if (!ISINT(0)) ERR_TYPE(Form,getType,1,integer)
 GETOBJ(Form,EZFForm,t);
 EZFQuestion * q = t->Questions[INT(0)];
 if (!q)
 {
  ERR_MSG(Form,"Index out of bounds",itos(INT(0)));
 }

 switch (q->Type)
 {
  case EZFQuestion::Text:       RETSTRW(L"Text");
  case EZFQuestion::Password:   RETSTRW(L"Password");
  case EZFQuestion::Date:       RETSTRW(L"Date");
  case EZFQuestion::Time:       RETSTRW(L"Time");
  case EZFQuestion::Number:     RETSTRW(L"Number");
  case EZFQuestion::Radio:      RETSTRW(L"Radio");
  case EZFQuestion::Weighted:   RETSTRW(L"Weighted");
  case EZFQuestion::Checkbox:   RETSTRW(L"Check");
  case EZFQuestion::ListSingle: RETSTRW(L"Single");
  case EZFQuestion::ListMulti:  RETSTRW(L"Multiple");
  case EZFQuestion::ListCombo:  RETSTRW(L"Combo");
  case EZFQuestion::ListRank:   RETSTRW(L"Rank");
  case EZFQuestion::Hidden:     RETSTRW(L"Hidden");
  case EZFQuestion::Section:    RETSTRW(L"Section");
  case EZFQuestion::PageBreak:  RETSTRW(L"Page");
  case EZFQuestion::RichText:   RETSTRW(L"RichText");
  case EZFQuestion::Image:      RETSTRW(L"Image");
  case EZFQuestion::PlainText:  RETSTRW(L"PlainText");
  case EZFQuestion::ToolButton: RETSTRW(L"ToolButton");
  default: RETINT(q->Type);
 }
}

WRAP(Form,setType)
{
 if (argc != 2) ERR_COUNT(Form,setType)
 if (!ISINT(0)) ERR_TYPE(Form,setType,1,integer)
 GETOBJ(Form,EZFForm,t);
 EZFQuestion * q = t->Questions[INT(0)];
 if (!q)
  {
  ERR_MSG(Form,"Index out of bounds",itos(INT(0)));
 }

 if (ISINT(1))
   {
    q->Type = (EZFQuestion::EType)INT(1);
    return JS_TRUE;
   }
 if (!ISSTR(1)) ERR_TYPE(Form,setType,2,string_integer)
 const char * c = STR(1);

 q->Type = GetType(c);
 if (!q->Type) return 0;

 RETBOOL(true);
}

B_WRAP_I(Form,EZFForm,IsPageStart);
B_WRAP_SET_S(Form,EZFForm,Questions,QuesText);
B_WRAP_SET_S(Form,EZFForm,Questions,FieldName);
B_WRAP_SET_S(Form,EZFForm,Questions,Description);
B_WRAP_SET_S(Form,EZFForm,Questions,HelpText);
B_WRAP_SET_S(Form,EZFForm,Questions,UnusedTags);
B_WRAP_SET_I(Form,EZFForm,Questions,Length);
B_WRAP_SET_R(Form,EZFForm,Questions,Options);
B_WRAP_SET_R(Form,EZFForm,Questions,Responses);
B_WRAP_SET_R(Form,EZFForm,Questions,Skips);
S_WRAP_GET(Form,EZFForm,Questions,QuesText);
S_WRAP_GET(Form,EZFForm,Questions,FieldName);
S_WRAP_GET(Form,EZFForm,Questions,Description);
S_WRAP_GET(Form,EZFForm,Questions,HelpText);
S_WRAP_GET(Form,EZFForm,Questions,UnusedTags);
I_WRAP_GET(Form,EZFForm,Questions,Length);
R_WRAP_GET(Form,EZFForm,Questions,Options);
R_WRAP_GET(Form,EZFForm,Questions,Responses);
R_WRAP_GET(Form,EZFForm,Questions,Skips);
V_WRAP_II(Form,EZFForm,Move);
V_WRAP_I(Form,EZFForm,Remove);

/*static JSBool EZFForm_getFieldName(JSContext *cx,
 JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{int32 index;
 if (argc != 1) ERR_COUNT(EZFForm,name);
 if (!TOINT(0,index)) ERR_TYPE(EZFForm,name,1,string);
 GETOBJ(Form,EZFForm,t);

 printf("\nt=%lx, count=%d, index=%d, q=%x\n",
  (int)(void*)t,t->Questions.Count(),
  index,(int)(void*)t->Questions[index]);

 if (t->Questions[index] == NULL) RETBOOL(false);
 RETSTR(t->Questions[index]->FieldName);
} */

static JSFunctionSpec Form_functions[] = {
    {"find",     EZFForm_Find, 1},
    {"findQuestion",     EZFForm_Find, 1},
    {"isPageStart",     EZFForm_IsPageStart, 1},
    {"save",     Form_Save, 0},
    {"toString", Form_ToString,0},
    {"getName",    EZFForm_getFieldName, 1},
    {"setName",    EZFForm_setFieldName, 2},
    {"getText",    EZFForm_getQuesText, 1},
    {"setText",    EZFForm_setQuesText, 2},
    {"getDescription",    EZFForm_getDescription, 1},
    {"setDescription",    EZFForm_setDescription, 2},
    {"getLength",    EZFForm_getLength, 1},
    {"setLength",    EZFForm_setLength, 2},
    {"getHelp",    EZFForm_getHelpText, 1},
    {"setHelp",    EZFForm_setHelpText, 2},
    {"hasData",    Form_HasData, 1},
    {"getExtra",    EZFForm_getUnusedTags, 1},
    {"setExtra",    EZFForm_setUnusedTags, 2},
    {"getType",    Form_getType, 1},
    {"setType",    Form_setType, 2},
    {"addQuestion",    EZFForm_addQ, 4},
    {"move",    EZFForm_Move, 2},
    {"remove",    EZFForm_Remove, 1},
    {"getResponses", EZFForm_getResponses, 1},
    {"setResponses", EZFForm_setResponses, 2},
    {"getOptions", EZFForm_getOptions, 1},
    {"setOptions", EZFForm_setOptions, 2},
    {"getSkips", EZFForm_getSkips, 1},
    {"setSkips", EZFForm_setSkips, 2},
    {"setModified",Form_SetModified,0},
    {0}
};

WRAP(Form,HELP)
{
 MemoryStream msg;
 for (int i = 0;Form_functions[i].name; i++)
 {
  msg << Form_functions[i].name << "(";
  if (Form_functions[i].nargs) msg  << Form_functions[i].nargs;
  msg << ")\n";
 }
 RETSTRW(WStr((char *)msg));
}

static JSFunctionSpec Form_fnstatic[] = {
    {"help",  Form_HELP,    0},
    {0}
};

static JSClass Form_class = {
    "Form", JSCLASS_HAS_PRIVATE,         //Form_JSGet
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub,   JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,Form_JSFinalize
};



JSObject*
Form_Object(JSContext *cx, EZFForm* t,bool autodelete,JSPointerBase* Parent)
{
 GETENV;
 JSObject* obj;
 MAKENEW(Form);;

 SETPRIVATE(obj,EZFForm,t,autodelete,Parent);

 if (!obj) return NULL;

 JSObject* options = Record_Object(cx,&t->Options,false,GETPOINTER);
 jsval opt = OBJECT_TO_JSVAL(options);
 JS_DefineProperty(cx, obj,"options",opt,NULL,NULL,JSPROP_READONLY|JSPROP_PERMANENT);
 return obj;
}

void Form_InitClass(JSContext *cx, JSObject *obj)
{
 GETENV;
 INITCLASS(Form);
}

JSClass* Form_Class() {return &Form_class;}

