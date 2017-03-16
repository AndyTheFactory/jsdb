function Debugger(port)
{
  this.http = new Server(8086);
  this.debug = null;
  try {
   this.debug = new Server(port || 0);
   } catch(x) {this.debug = null;}
  if (this.debug == null ) this.debug = new Server();
  
  writeln("HTTP server: ",this.http);
  writeln("Debug server: ",this.debug);
  this.remote = null;
  this.reset();
  this.commandLine = null;
}

Debugger.prototype.reset = function(r)
{
  this.remote = r;
  this.ready = false;
  this.quit = false;

  this.messages = null;
  this.history = new Array
  this.breaks = null;
  this.breakText = null;
  this.watches = null;
  this.scripts = null;
  this.stack = null;
  this.locals = null;
  this.lines = null;

  this.currentFile = '';
  this.currentLine = '';

  this.status = ''; //STOP STEP, STOP FUNCTION, STOP BREAK, STOP DONE, ERROR
}

Debugger.prototype.printTextHeader = function(client)
{
  client.writeln("HTTP/1.1 200 OK");
  client.writeln("client: close");
  client.writeln("Date: 0");
  client.writeln("Expires: 0");
  client.writeln("Server: JSDB/0.1");
  client.writeln("Content-type: text/html\n");
}

Debugger.prototype.printHeader = function(client,title)
{
  client.writeln("HTTP/1.1 200 OK");
  client.writeln("client: close");
  client.writeln("Date: 0");
  client.writeln("Expires: 0");
  client.writeln("Server: JSDB/0.1");
  client.writeln("Content-type: text/html\n");
  client.writeln("<head><title>",title,"</title></head><body link=blue vlink=blue leftmargin=0 topmargin=0 rightmargin=0>");
  client.writeln("<style>");
  client.writeln("BODY {font: 11pt Arial,Helvetica,sans-serif; text-align:justify; border:0px;}");
  //client.writeln("P {font: 11pt Arial,Helvetica,sans-serif; text-align:justify;}");
  //client.writeln("H1 {font: 16pt Arial,Helvetica,sans-serif ; font-weight: bold; margin-bottom: 3pt;}");
  //client.writeln("H2 {font: 14pt Arial,Helvetica,sans-serif ; font-weight: bold; margin-bottom: 3pt;}");
  client.writeln("TABLE {border-collapse: collapse;}");
  client.writeln("TD {font-family: Arial,Helvetica,sans-serif; font-size: 10pt; border: solid 1pt #808080; margin:0px; padding:0px;}");
  client.writeln(".title {font-family: Arial,Helvetica,sans-serif; font-size: 10pt; background: #c0c0c0; border: solid 1pt #808080; border-bottom: thin solid #e0e0e0}");
  client.writeln(".frame {padding:0px; margin:0px; border: solid 1pt #808080;}");
  client.writeln("TT {font-size: 11pt;}");
  client.writeln("A {color: #0000c0; cursor:pointer; text-decoration: none;}");//cursor:pointer; 
  client.writeln("A.button {color: #c00000; cursor:pointer; text-decoration: none; background:#c0c0c0; border-left:#ffffff; border-top:#ffffff; border-right:#808080; border-bottom:#808080;}");
  client.writeln("A:link {color: #0000c0;}");
  client.writeln("A:visited {color: #801A8B;}");
  client.writeln("A:active, A:hover {color: #FFFFFF; background: #0000c0}");
  client.writeln("</style>");
}
//LINE SOURCE STACK SCRIPTS EVALUATE INSPECT
//BREAK CLEAR RUN STEP SKIP RETURN STOP THROW
Debugger.prototype.send = function(msg,text)
{
  if (!this.remote) return;
  this.ready = false;
  var m = msg.toUpperCase();
  if (text) m += ' ' + text;

  if (msg == 'STEP' || msg== 'SKIP'
      || msg == 'RUN' || msg == 'RETURN'
      || msg == 'STOP' || msg == 'THROW'
      || (text && (msg == 'BREAK' || msg == 'CLEAR' || msg == 'EVALUATE' )))
   this.history.push(m);

  writeln(m)
  if (this.remote.canWrite) this.remote.writeln(m);
}

Debugger.prototype.readReplyLines = function()
{
  var d = this.readReply();
  writeln(d)
  if (d)
  {
    d = d.split('\n');
    if (d.length && !d[d.length-1]) d.pop();
    return d;
  }
  return null;
}

Debugger.prototype.readReply = function()
{
  var data = '';
  while (this.remote)
  {
    if (this.remote.canRead)
    {
      data = this.remote.readLine();
      //writeln(data);
    }
    else
    {
      if (!this.remote.canWrite)
        this.remote = null;
      else
        sleep(128);
      continue;
    }
    if (data.substr(0,3) == 'MSG')
    {
      this.messages.push(data.substr(4));
      if (data.indexOf('LOAD')!= -1) //load or unload a script
      {
        this.scripts = null;
         this.lines = null;
       }
      continue;
    }
    else
    {
      this.ready = true;
      if (data.substr(0,2) == 'OK')
      {
        var length = Number(data.substr(3));
        return length ? this.remote.read(length) : '';
      }
      else if (data.substr(0,4) == 'STOP')
      {
        if (data.indexOf('FUNCTION') != -1 || data.indexOf('BREAK'))
          this.stack = null;
        this.status = data;
        var cur = this.status.match(/STOP (\w+) (.*)#(.*)/);
        if (cur)
        {
          this.currentFile = cur[2];
          this.currentLine = cur[3];
        }
      }
      else if (data.substr(0,5) == 'ERROR')
      {
        this.status = data;
        this.messages.push(data);
        return '';
      }
      return data;
    }
  }
}

Debugger.prototype.printJS = function(client)
{
 if (!this.js)
  this.js = system.resource('ajax.html');
 else
  this.js.rewind();

 client.write(this.js.readFile());
 client.writeln('')
 
 client.writeln('<script>')
 client.writeln('function toggle(id) {')
 client.writeln('var obj = document.getElementById(id);')
 client.writeln('if (obj.style.display == "none")')
 client.writeln('  obj.style.display = "block";')
 client.writeln('else')
 client.writeln('  obj.style.display = "none";')
 client.writeln('}')
 client.writeln('</script>')
}

Debugger.prototype.printBreaks = function(client)
{
  if (this.breaks)
  for (var i in this.breaks)
  {
    client.write('<a href="clear?b=',encodeURL(this.breaks[i]),'">[x]</a> ')
    //client.writeln('<span onClick="toggleShow(this,\'bt',i,'\')">[+]</span>');
    client.write('<a onClick="javascript:toggleShow(null,\'bt',i,'\')">');
    client.write(this.breaks[i],'</a> ');
    client.write('<span id=bt',i,' style="display: none"> ');
    client.write('<tt><a target=_blank href="source?b=',encodeURL(this.breaks[i]),'">',this.breakText[i],'</a></tt></span>');
    if (i) client.writeln('<br>')
  }
  else
  client.writeln('None');
}

Debugger.prototype.printStack = function(client)
{
  if (this.stack)
  for (var i in this.stack)
  {
    client.write('<a target=_blank href="source?f=',encodeURL(this.stack[i]),'">')
    client.writeln(this.stack[i],'</a><br>');
  }
  else
  client.writeln('Not available');
}

Debugger.prototype.printScripts = function(client, target)
{
 if (!target) target = '_parent'
  if (this.scripts)
  for (var i in this.scripts)
  {
    client.write('<a target=' + target + ' href="source?f=',encodeURL(this.scripts[i]),'">')
    client.writeln(this.scripts[i],'</a><br>');
  }
  else
  client.writeln('Not available');
}
Debugger.prototype.printSource = function(client,source,base,line)
{
  if (source)
  for (var i in source)
  {
    var x = source[i].indexOf(' ');
    var num = source[i].substr(0,x);
    var code = source[i].substr(x+1);
    client.write('<tt><a href="break?b=',base,'%23',num,'">',num,'</a>')
    if (num == line)
     client.write('<span style="background: #ffff00">&gt;');
    else
     client.write('&nbsp;')
    client.writeln(encodeHTML(code));
    if (num == line)
     client.write('</span>');
    client.writeln('</tt><br>');
  }
  else
  client.writeln('Not available');
}

Debugger.prototype.source = function(client,file)
{
  this.send('SOURCE',file);
  var source = this.readReplyLines();
  if (file.indexOf('#') != -1)
  {
    var x = file.split('#');
    this.printSource(client,this.lines,x[0],x[1]);
  }
  else
  {
    this.printSource(client,this.lines,file,0);
  }
}


/*
Lefter plane
+ Scripts
+ Locals

Left pane
Stack
Breakpoints
Messages
History

Right pane
Evaluate
Status (popup history)
Source (iframe?)
*/

// results: clear history, send commands,
Debugger.prototype.home = function(client)
{
  if (this.ready)
  {
    if (!this.breaks)
    {
      this.send('BREAK')
      this.breaks = this.readReplyLines();
      if (this.breaks)
      {
        this.breakText = new Array(this.breaks.length);
        for (var i in this.breaks)
        {
          this.send('LINE',this.breaks[i]);
          this.breakText[i] = this.readReply();
        }
      }
    }
    if (!this.lines)
    {
      this.send('SOURCE');
      this.lines = this.readReplyLines();
    }
    if (!this.stack)
    {
      this.send('STACK');
      this.stack = this.readReplyLines();
    }
    if (!this.scripts)
    {
      this.send('SCRIPTS');
      this.scripts = this.readReplyLines();
    }
  }

  this.printJS(client);

//client = system.stdout
//-----
  var actions = "<a class=button href=run>Run</a> <a class=button href=step>Step</a>";
  if (this.status.indexOf('STOP FUNCTION')==0) actions += " <a class=button href=skip>Skip</a>";
  actions += " <a class=button href=done>Done</a> <a class=button href=stop>Stop</a>";
  if (!this.remote) actions = "<a class=button href=restart>Restart</a> <a class=button href=quit>Exit</a>"
//-----
  
  client.writeln('<table width=100% cellspacing=0 border=0>');
  
  //column 1
  client.writeln("<tr><td valign=top width=20%>")
 
  client.writeln('<div class=title><span onClick="toggle(\'scripts\')">Scripts</span></div>');
  client.writeln("<div id=scripts>");
  //client.writeln("<iframe id=scripts src=scripts width=100% height=60pt scrolling=yes hspace=0 vspace=0 frameborder=0 marginheight=0 marginwidth=0></iframe>");
  this.printScripts(client,'popup');
  client.writeln("</div>");
  
  client.writeln('<div class=title><span onClick="toggle(\'locals\')">Locals</span></div>');
  client.writeln("<div id=locals>");
  this.inspect(client,'','popup');
  client.writeln("</div>");
  
  client.writeln("</td>")
  
  //column 2
  client.writeln("<td valign=top width=20%>")
  
  client.writeln('<div class=title><span onClick="toggle(\'stack\')">Stack</span></div>');
  client.writeln("<div id=stack>")
  this.printStack(client);
  client.writeln("</div>");

  client.writeln('<div class=title><span onClick="toggle(\'breakpoints\')">Breakpoints</span></div>');
  client.writeln("<div id=breakpoints>")
  this.printBreaks(client);
  client.writeln("</div>");

  client.writeln('<P class=title><span onClick="toggle(\'messages\')">Messages</span></P>');
  client.writeln("<iframe style=\"display:none\" id=messages src=messages width=100% height=50pt scrolling=yes hspace=0 vspace=0 frameborder=0 marginheight=0 marginwidth=0></iframe>");
  
  client.writeln('<P class=title><span onClick="toggle(\'history\')">History</span></P>');
  client.writeln("<iframe style=\"display:none\" id=history src=history width=100% height=50pt scrolling=yes hspace=0 vspace=0 frameborder=0 marginheight=0 marginwidth=0></iframe>");
  
  client.writeln("</td>")
  
  //column 3
  client.writeln("<td valign=top>")
  
  client.writeln('<P class=title><span onClick="toggle(\'evaluate\')">Evaluate</span></P>');
  client.writeln("<div id=evaluate style=\"display:none\">");
  
  client.writeln('<form target=evaluate><input name=e>');
  client.writeln('<input onClick="form.action=\'evaluate\'" type=submit value="Evaluate">');
  client.writeln('<input onClick="form.action=\'inspect\'" type=submit value="Inspect"><br>');
  client.writeln('<iframe name=evaluate></iframe>');
  client.writeln('</form>');
  client.writeln('</div>')

  client.writeln('<P class=title>')
  client.writeln(actions);
  client.writeln('<b><a target=popup href=history>',this.status,'</a></b></p>')
  client.writeln('<div id=source>')
  this.printSource(client,this.lines,this.currentFile,this.currentLine);
  client.writeln('</div>')
  
  client.writeln("</td>")
  client.writeln("</tr>")
  client.writeln('</table>')
}

Debugger.prototype.close = function(client)
{
 client.writeln("<script language=JavaScript>\n<!--\nwindow.close()\n//--></script>");
 client.writeln("<P align=center>You may close this window<br>");
 client.writeln('<input type=button onClick="window.close()" value=Close>');
}

Debugger.prototype.stopped = function(client)
{
 client.writeln("<head><meta http-equiv=Refresh content=1000></head>");
 client.writeln("<P>Waiting for a connection at ",this.debug);
 if (this.commandLine) client.writeln("<P><a href=restart>Restart</a>");
 else client.writeln("<P><a href=/>Refresh</a>");
}

Debugger.prototype.evaluate = function(client,expr,popup)
{
 if (this.ready && expr)
 {
   this.send('EVALUATE',expr);
   client.writeln('<tt>',this.readReply(),'</tt>');
 }
 if (!popup) popup = '_self';

 /*
 client.writeln('<form target=',popup,'><input name=e size=25 value="',encodeHTML(expr),'"><br>');
 client.writeln('<input onClick="form.action=\'evaluate\'" type=submit value="Evaluate">');
 client.writeln('<input onClick="form.action=\'inspect\'" type=submit value="Inspect">');
 client.writeln('</form >');
 */
}

Debugger.prototype.inspect = function(client,expr,popup)
{
  if (!this.ready)
  {
    client.writeln('Not available');
    return;
  }

  if (!popup) popup = '_self';
  this.send('INSPECT',expr);
  var data = this.readReplyLines();
  if (expr)
    client.writeln('<b>',expr,'</b>');
  if (!data) return;
  if (data.length == 1)
  {
    client.writeln(data[0],'<br>');
  }
  else
  {
    if (expr) client.writeln('<br>');
    for (var i in data)
    {
      if (!data[i]) continue;
      var name = data[i]
      var type = '';
      var x = name.indexOf(' ');
      if (x != -1)
      {
        type=name.substr(x);
        name=name.substr(0,x);
      }

      var base = '';
      if (expr)
        base = expr + '.';
      if ("0123456789".indexOf(data[i][0]) == -1)
        client.writeln('<tt><a target=',popup,' href="inspect?e=',encodeURL(base+name),'">',name,'</a></tt>',type,'<br>')
      else
        client.writeln('<tt><a target=',popup,' href="inspect?e=',encodeURL(expr+'['+name),']">',name,'</a></tt>',type,'<br>')
    }
  }
}

Debugger.prototype.acceptHTTP = function(server)
{
  var client = server.accept();
   if (client == null)
    return null;

   var request = client.readLine().split(/\s+/); //GET /page?query HTTP/1.1

   client.method = request[0];
   client.uri = request[1];
   if (client.uri == null || client.uri == '') client.uri = '/';
   client.version = request[2];

  client.startTime = new Date();

   if (client.canRead)
   {
    client.header = new Record;
    client.readMIME(client.header);
   }
   client.page = client.uri.substr(1); // /page?query
   client.query = '';
   request = client.uri.match(/\/?([^?]*)\?(.*)/);
   if (request != null)
    {
     client.page = request[1];
     client.query = request[2];
    }

   if (client.method == "GET" && client.query)
    client.data = new Record(client.query,'&');
   else if (client.method = "POST" && client.header && client.header.get('Content-type') == 'application/x-www-form-urlencoded')
    client.data = new Record(client.read(client.header.get('Content-length')),'&');

   if (client.data)
   {for(x=0; x<client.data.length; x++)
      client.data.set(x, decodeURL(client.data.value(x)));
    }
   return client;
}

Debugger.prototype.run = function()
{
 var now = new Date
 var timeout = Number(now) + 5000;

 try 
 {
  system.browse('http://127.0.0.1:'+ this.http.port + '/');
 } catch(x) {writeln("Couldn't open the browser: ",x);}
 
 
//writeln("Debugger.run()")
try {
 while (!jsShouldStop() && !this.quit) /* invokes garbage collection, and may stall */
 {
   var now = new Date();
    
   if (!this.remote && this.debug.anyoneWaiting)
   {
    this.reset(this.debug.accept());
    this.messages = ['Debugging ' + this.remote];
    this.readReply();
    //not ready
  }

  if (this.remote && !this.remote.canWrite)
  {
    this.remote.close();
    this.ready = false;
    this.remote = null;
  }

  if (!this.http.anyoneWaiting)
  {
    if (Number(now) > timeout) 
    {
      writeln('watchdog expired');
      break;
    }
    sleep(100);
    continue;
  }

  timeout = Number(now) + 1000;

  client = this.acceptHTTP(this.http);

  if (!client) continue;

  if (client.page == 'quit')
  {
    this.printHeader(client);
    this.quit = true;
    this.close(client,"Done");
    if (this.remote) this.remote.close(); //deactivates debugger
  }
  else if (client.page == 'ping')
  {
    this.printTextHeader(client);
    client.writeln('ok');
  }
  else if (client.page == 'scripts')
  {
    this.printHeader(client);
    this.printScripts(client);
  }
  else if (client.page == 'messages')
  {
    this.printHeader(client);
    //client.writeln('<tt>');
    for (var i in this.messages)
      client.writeln(this.messages[i],'<br>');
    //client.writeln('</tt>');
  }
  else if (client.page == 'history')
  {
    this.printHeader(client);
    client.writeln('<tt>');
    for (var i in this.history)
      client.writeln(this.history[i],'<br>');
    client.writeln('</tt>');
  }
  else if (this.remote)
    this.respond(client);
  else
  {
    this.printJS(client);

    this.stopped(client);
    writeln('restart');
    if (client.page == 'restart' && this.commandLine)
    {
      system.execute(this.commandLine[0],this.commandLine[1]);
    }
  }

  client.close();
  continue;
 }
} catch( x) {writeln(x);}

 
 if (this.remote)
   this.remote.close();
 this.http.close();
 this.debug.close();
}

Debugger.prototype.respond = function(client)
{
  if (client.page == '' || client.page == 'home' || client.page == 'restart')
  {
    this.printHeader(client,"Debugger");
    this.home(client);
  }
  else if (client.page == 'evaluate')
  {
    this.printHeader(client,"Evaluate");
    this.evaluate(client,client.data.get('e'));
  }
  else if (client.page == 'inspect')
  {
    this.printHeader(client,"Inspect");
    this.inspect(client,client.data.get('e'));
  }
  else if (client.page == 'source')
  {
    this.printHeader(client,"Source - " + client.data.get('f'));
    this.source(client,client.data.get('f'));
  }
  else if (client.page == 'break' || client.page == 'clear')
  {
    this.printHeader(client,"Debugger");
    this.breaks = null;
    this.send(client.page.toUpperCase(),client.data.get('b'));
    this.readReply();
    this.home(client);
  }
  else if (client.page == 'step' || client.page == 'run' || client.page == 'skip' ||
        client.page == 'done' || client.page == 'stop')
  {
    this.send(client.page.toUpperCase());
    this.readReply();
    this.printHeader(client,"Debugger - " + client.page);
    this.home(client);
  }
  else
  {
    client.writeln("HTTP/1.1 404 NOT FOUND");
    client.writeln("Date: 0");
    client.writeln("Expires: 0");
    client.writeln("Server: JSDB/0.1");
    client.writeln("Content-type: text/html\n");
    client.writeln("<H2>HTTP/1.1 404 Not Found</H2>");
    client.writeln("<br><a href=/>Home</a>");
    client.writeln("<hr>URL:",client.uri);
    if (client.data) client.writeln("<br>",client.data.toString());
  }
}

var server = new Debugger(6000);
if (system.arguments.length)
{
 var s = '-debug ' + server.debug;
 for (var i in system.arguments)
 {
  s += ' "' + system.arguments[i] + '"';
 }
 writeln('running ',s)
  server.commandLine = [system.program,s];
  system.execute(server.commandLine[0],server.commandLine[1]);
}
else
  writeln(server.debug);
  
server.run();
delete server;

/* Venkman
Stop | continue | over | into | out | profile | pretty print
------------------------------------------------------------
Loaded scripts                   | Source code
Local / watch (name,value,type)  +-------------------
Break / call                     | Evaluate / history (save, replay)
---------------------------------+------------------------
*/
