var server = null;
try {
 server = new Server(6000);
 } catch(x) {server = null;}
if (server == null ) server = new Server(0);

if (jsArguments.length)
{
  var s = '-debug ' + server.toString();
  for (var i in system.arguments)
  {
   s += ' "' + system.arguments[i] + '"';
  }
  writeln('running ',s)
  
  system.execute('jsdb.exe',s);
 }
else
  writeln(server);
  
var lastLine = '';
var client = null;

var history = new Array;
var script = new Array;
client = server.accept();

while (client.canWrite)
{
  if (client.canRead)
   lastLine = client.readLine();
  else
   {
    lastLine = '';
    sleep(1024);
    continue;
   }

  if (lastLine.substr(0,3) == 'MSG')
  {
    writeln(lastLine.substr(4));
  }
  else
  {
    if (lastLine.substr(0,2) == 'OK')
    {
      var length = Number(lastLine.substr(3));
      if (length)
      {
        var data = client.read(length);
        writeln(data);
      }
    }
    else if (lastLine) writeln(lastLine);
    var cmd = '';
    while (client.canWrite && (cmd == '' || cmd == '?' || cmd.toLowerCase() == 'help'))
    {
      if (cmd.length)
      {
writeln("LINE filename#line     script source code (one line only)")
writeln("SOURCE filename#line   script source code (entire compilation unit)")
writeln("SOURCE filename        all scripts for that filename")
writeln("SCRIPTS                lists scripts: filename#lines version")
writeln("BREAK                  lists breakpoints")
writeln("BREAK filename#line    sets a breakpoint")
writeln("CLEAR filename         removes a breakpoint")
writeln("STACK                  print the current stack")
writeln("RUN                    runs until a breakpoint or error")
writeln("STEP                   enters line-by-line mode")
writeln("SKIP                   steps over the next function call")
writeln("RETURN [rval]          exit the current scope, if possible")
writeln("STOP                   sets a debug error and aborts")
writeln("INSPECT                current stack frame")
writeln("INSPECT object         object.toSource()")
writeln("THROW error            sets an error  condition")
writeln("EVALUATE code          evaluates instructions")
       }
       
      write('>');
      if (script.length) {cmd = script.shift(); writeln(cmd);}
      else {cmd = readln(); history.push(cmd);}
      
      if (!cmd.length) 
      writeln("LINE SOURCE STACK SCRIPTS EVALUATE INSPECT BREAK\nCLEAR RUN STEP SKIP RETURN STOP THROW HELP");

    }
    if (cmd.toLowerCase() == 'quit') break; //debugger will stop automatically.
    else
    {
       client.writeln(cmd);
    }
  }
}
writeln('DONE');
client.close();

for (var i in history)
 writeln(history[i]);