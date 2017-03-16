function XML(name,params,cdata)
{
 if (name.constructor.name == "Stream")
 {
   this.children = new Array;
   this.cdata = '';
   readXML(name,null,this)
 }
 else
 {
   this.name=name;

   if (typeof(params) == 'undefined' || params == null)
    this.params = new Record();
   else if (typeof(params) == 'string')
    this.params = new Record(params);
   else
    this.params = params;

   this.cdata = (cdata == null) ? '' : cdata;
   this.children = new Array;
 }
}

XML.prototype.toString = function ()
{
 var out = new Stream;
 this.toStream(out);
 return out.toString();
}

XML.prototype.toStream=function (out)
{
 out.write('<', this.name);

 if (this.params != null)
 for(var x=0; x<this.params.count; x++)
   out.write(' ',this.params.name(x),'="',encodeHTML(this.params.value(x)),'"');

 if (this.children.length > 0 || this.cdata.length > 0)
  {
   out.writeln('>');

   for(var x in this.children)
    this.children[x].toStream(out);

   if (this.cdata.length)
    {
     if (this.cdata.indexOf('</') != -1)
     {/*looks like included markup data*/
      out.writeln('<![CDATA[',this.cdata,']]>');
     }
     else
     {
      out.writeln(this.cdata);
     }
    }
   out.writeln('</',this.name,'>');
 }
 else
  out.writeln(' />');
}

XML.prototype.select=function(type, test, z)
{
 var ret = [];
 for(var x =0; x<this.children.length; x++)
  {
   if (type != null) if (this.children[x].name != type) continue;
   if (test == null)
     ret.push(this.children[x]);
   else if (test(this.children[x],z))
      ret.push(this.children[x]);
  }
 return ret;
}
/*
XML.prototype.find=function(type,field,value,field2,value2)
{
 var ret = [];
 for(var x =0; x<this.children.length; x++)
  {
   if (type != null) if (this.children[x].name != type) continue;
   if (field == null)
     ret.push(this.children[x]);
   else if (this.children[x].get(field) == value)
   {
     if (field2 == null || this.children[x].get(field2) == value2)
      ret.push(this.children[x]);
   }
  }
 return ret;
}
*/
XML.prototype.find=function(type,field,value,field2,value2)
{
 var x
 var i
 var m = arguments.length - 1;
 var ret = [];
 iterator: for(x =0; x<this.children.length; x++)
  {
   if (type != null) if (this.children[x].name != type) continue;
   for (i=1; i < m; i += 2)
    if (this.children[x].get(arguments[i]) != arguments[i+1]) continue iterator;
   ret.push(this.children[x]);
  }
 return ret;
}

XML.prototype.getElementsByTagName = function (n) 
{
    var a = [];
    if (this.name == n) 
        a.push(this);
    for (var i = 0; i < this.children.length; i++) 
        a = a.concat(this.children[i].getElementsByTagName(n));
    return a;
}

XML.prototype.has=function (str)
{
 return this.params.has(str);
}

XML.prototype.get= function(str)
{
 if (str == 'cdata') return this.cdata;
 if (this.params.has(str))
  return this.params.get(str);
 var ch = this.find(str);
 if (ch == null) return "";
 if (ch.length == 1) return ch[0].cdata;
 return "";
}

XML.prototype.insert=function (node,params,cdata)
{
 if (node.constructor.name != 'XML' && typeof node == 'string')
 {
  node = new XML(node,params,cdata); 
 } 
 if (this.db != null)
  {
   return this.db.insert(this.id,node);
  }
 this.children.push(node);
 return true;
}

XML.prototype.set=function (name,value)
{
 if (this.db != null)
  {
   return this.db.set(this.id,name,value);
  }
 if (name == 'cdata') {this.cdata=value; return -1;}
 return this.params.set(name,value);
}

XML.prototype.sort=function (field,reverse)
{
 if (reverse)
  this.children.sort(function comp(x,y){a = x.get(field); b = y.get(field); return a > b ? -1 : a == b ? 0 : 1;})
 else
  this.children.sort(function comp(x,y){a = x.get(field); b = y.get(field); return a < b ? -1 : a == b ? 0 : 1;})
}

function appendXML(text,allowed)
{
 var result = new Array;
 var infile;
 if (text.className = 'stream')
  infile = text;
 else
 {
  infile = new Stream();
  infile.write(text);
  infile.rewind();
 }

 while (true)
 {
  var tag = readXML(infile,allowed);
  if (tag == null) break;
  result.push(tag);
 }
 return result;
}

XML.read = function(stream, allowed, ignore, start)
{
/* Reads a single XML object and all its children: name, properties, children, cdata */
 if (typeof stream == "string")
 {
  var s = new Stream
  s.write(stream);
  s.rewind()
  stream = s;
 }
 var parent = null; /*top object on the stack */
 var end = '';
 var stack = new Array;
 if (ignore == null) ignore=',BR,P,B,I,C,TT,U,IMG,A,';
 else ignore = ',' + ignore + ',';
 var startpos = 0;
 var startname = '';
 
 while (!stream.eof)
 {
  var params = new Record;
  /* readTag automatically decodes the &amp; &quot; &lt; &gt; sequences */
  var name = stream.readTag(params,allowed); 

  /* may be appending cdata to a parent */
  if (parent != null)
  {
   if (name.length == 0)
   {
    parent.cdata += '<>';
    continue;
   }

   if (name == '![CDATA[')
   {
    parent.cdata = stream.tagText;
    delete stream.tagText;
   }

   /* HTML-like markup tags don't count */
   if (name.length <= 3 && name.search('[^/A-Za-z_]') == -1)
   {    
    var n = (name[0] == '/') ? name.substr(1) : n;
    var find = new RegExp(',' + n + ',','i') ;
    if (ignore.length && ignore.search(find) != -1)
     {
      parent.cdata += decodeHTML(stream.tagText);
      parent.cdata += '<' + name;
      if (params.length > 0) parent.cdata += ' ' + params.write(' ');
      parent.cdata += '>';
      continue;
     }
   }
  }
  else
  {/* if we just started, wait until we see the start of an object */
   if (name.charAt(0) == '/' || name.length == 0)
   continue;
  }

  /* XML control tags don't count */
  if (name.charAt(0) == '?' || name.charAt(0) == '!')
  continue;

  if (name == end) /* finish an object */
  {
    parent.cdata += decodeHTML(stream.tagText);
    delete stream.tagText;

    if (stack.length == 0) return parent; /* just finished the first object */

    end = stack.pop();
    parent = stack.pop();
   }
  else if (name.charAt(0) == '/')
   { //probably a parse error, so ignore it
    throw('XML parse error at '+stream.pos+' in object '+startname+' started at '+startpos+' after '+parent.name+'.'+parent.get('name')+ ' before "'+stream.read(32)+ '"');
   }
  else /* start a new object */
   {
    var obj;

    if (parent == null && start != null) 
    {
       obj = start;
       obj.name = name;
       obj.params = params;
    }
    else
       obj = new XML(name,params,'');

    startpos = stream.pos;
    startname = name;

    if (parent != null)
     {
      parent.children.push(obj);
      parent.cdata += decodeHTML(stream.tagText);
     }

    if (stream.hasChildren)
     {
      if (parent != null)
      {
      stack.push(parent);
      stack.push(new String(end));
      }
      end = "/" + name;
      parent = obj;
     }
    else if (parent == null) return obj;
   }
  delete stream.tagText;

 }/*! stream.eof() */

 /* the XML file was not terminated, so return the first object */
 /* and assume terminators for the rest */
 return parent;
}

XML.test = function()
{
 var x = XML.read('<list><square id=44 /><sphere id=55 /></list>')
 y = x.select(null,function (a,b) {return a.get('id') == b},55)
 writeln(y)
}

function readXML(stream, allowed)
{
 return XML.read(stream,allowed, null, null)
}
