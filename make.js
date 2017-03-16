var debug = false; // debug the build script

var doBuild = true; // true = build the program
                    // false = write a build script
var linkOnly = false; // true = don't compile, just link and post-process
                      // false = compile and build
var quiet = false;  
var verbose = false; // not a command line option
var stopEarly = false;
var globalDefines = [];
var startAt = '';
var configure = true;

var logFile = "make.log"
/*
runMake
  doConfigure
    runCommand

  doCompile
    compileFile
      runCommand

  doLink
    runCommand

  doConfigure
    runCommand

  What about dependency information? Compiling with -MMD generates a .d file as a by-product.
  After compiling, we would need to record the dependency timestamps.
  Then, before compiling, test that the timestamps of the .d and .o outputs match, read the timestamps,
  and check the dependencies for changes. Whew!
*/

function objMatches(a,platform,mode)
{
  //var ret = ((!a.get('platform') || a.get('platform') == (platform?platform.get('name'):'')) && 
  //        (!a.get('mode') || a.get('mode') == (mode?mode.get('name'):'')));

  return ((!a.get('platform') || a.get('platform').split(',').indexOf(platform?platform.get('name'):'')!= -1) && 
          (!a.get('mode') || a.get('mode').split(',').indexOf(mode?mode.get('name'):'')!= -1));
  /*
  if (platform && a.get('platform'))
  {
   if (a.get('platform').split(',').indexOf(platform.get('name')) == -1) return false
  }
  if (mode && a.get('mode'))
  {
   if (a.get('mode').split(',').indexOf(mode.get('name')) == -1) return false
  }
  return true;
  */
}

function filter(list,platform,mode)
{
 if (!list) return list
 var ret = []
 for (var i=0;i<list.length;i++)
 {
  if (objMatches(list[i],platform,mode))
   ret.push(list[i])
 }
 return ret
}

function getProperty(name,define,node,platform,mode)
{
   var d = [];
   var l = node.find(name);
   for (var i=0; i<l.length; i++)
   { 
       var n = l[i]
       if (n.get('require') && define.indexOf(n.get('require'))==-1) continue;  
       if (objMatches(n,platform,mode))
         d.push(n.cdata);
   }
   return d;
}

function getAttribute(name,node)
{
   var l = node.get(name);
   if (l) return l;
   return '';
}
     
//BCC Error E2285 rs/tbl_main.cpp 78: Message
//gcc filename:error:line: description
function scanErrors(msgs,errors)
{
 var l;
 while(l=msgs.readln())
 {  
  if (l.indexOf("Error") == 0) //Borland C++
   return false;
  
  if (l.indexOf('error:') != -1) //gcc
   return false;

  if (l.indexOf('error ') != -1) //vc
   return false;

  if ((l.indexOf('g++.exe') == 0 || l.indexOf('gcc.exe:') == 0) && l.indexOf('warning:') == -1) //gcc
   return false;
  
  //skip "in file included from ..." notes before warnings.
  if (l.search(/^[\w/]+\:\d+\:/) >= 0 && l.indexOf("warning") != -1) //gcc
   return false;
  
  if (l.indexOf("(.text") >= 0) //gcc link
   return false;  
 }
 return true;
}

function runCommand(cmd,errors)
{
   if (verbose) writeln(cmd);
   if (cmd.substr(0,2) == '#!') return true;
   var w = cmd.match(/[^ "]+|"[^"]*"/g);
   for (var x =0; x<w.length; x++) w[x] = w[x].replace(/^"|"$/g,'')
   if (w[0] == 'cp' || w[0] == 'copy')
   {
    system.copy(w[1],w[2])
    return true;
   }
   if (w[0] == 'mv' || w[0] == 'move')
   {
    system.move(w[1],w[2],true)
    return true;
   }
   if (w[0] == 'echo')
   {
    writeln(cmd.replace(/^echo/,''));
    return true;
   }
   var msgs = new Stream;
   var out 
   if (cmd.indexOf('>') != -1)
   {
     var append = cmd.indexOf('>>') != -1;
     cmd = cmd.split(/>+/)
     out = new Stream(cmd[1].replace(/^\s+/,''),append ? 'at' : 'wt')
     cmd = cmd[0].replace(/^\.\//,'').replace(/\s+$/,'')
   }
   var pc = new Stream('exec://'+cmd);  
   var se = pc.stderr;
   while (pc.canRead || pc.canWrite)
     {
      sleep(10);
      while (se.canRead)
      {
       var c = se.read(1);
       if (verbose) write(c);
       msgs.write(c);
      }
      while (pc.canRead)
      {
       var c = pc.read(1);
       if (!c) continue;
       if (verbose) write(c);
       if (out) out.write(c)
       else msgs.write(c);
      }
     }
   pc.close();
   if (out) out.close()
   msgs.rewind();
   if (!scanErrors(msgs)) //returns false on error
    {
     msgs.rewind();
     errors.writeln(cmd);
     errors.append(msgs);
     return false;
    }
   return true;
}

function templateize(template,array)
{
 var ret = []; 
 array.forEach(function(x) {if (x) ret.push(template.replace(/\$value/g,x));})
 return ret;
}

function escapeIntermediate(name,compiler)
{
 return name.replace(/\.\w*$/,compiler.get('output')).replace(/[\/\ \\]/g,'_');
}

function compileFile(cmds,compiler,target,platform,mode,group,file,run)
{
  var define = globalDefines.concat([]);
      define = define.concat(getProperty('define',define,mode,platform,null));
      define = define.concat(getProperty('define',define,platform,null,mode));
      define = define.concat(getProperty('define',define,target,platform,mode));
      define = define.concat(getProperty('define',define,group,platform,mode));
      define = define.concat(getProperty('define',define,file,platform,mode));
      define = define.concat(getProperty('define',define,compiler,platform,mode));

  if (file.get('require') && define.indexOf(file.get('require'))==-1) return null;
  
  var include = [] //group.get('path');
      //if (include) include += ';';
      include = include.concat(getAttribute('include',mode).split(';'));
      include = include.concat(getAttribute('include',platform).split(';'));
      include = include.concat(getAttribute('include',group).split(';'));
      include = include.concat(getAttribute('include',file).split(';'));
      include = include.map(function(x) fixPath(x,platform,mode));
         
  var cmd = fixPath(compiler.get('path'),platform,mode);
  var params = compiler.get('parameters');
  var infile = fixPath(group.get('path'),platform,mode) + file.cdata;  
  var outfile = escapeIntermediate(file.cdata,compiler)
 //might need this for VC.
// var outfile = file.cdata.replace(/\.\w*$/,compiler.get('output'));
  
  var options = [getAttribute('options',file)];
  options = options.concat(getProperty('options',define,compiler,platform,mode));
      
  var templates = compiler.find('template','name','define');
  if (templates.length)
  {
    define = templateize(templates[0].cdata,define).join(' ')
  }
  
  templates = compiler.find('template','name','include');
  if (templates.length)
  {
    include = templateize(templates[0].cdata,include).join(' ')
  }  
  
  params = params.replace(/\$input/g,infile);
  params = params.replace(/\$output/g,outfile);
  params = params.replace(/\$options/g,options.join(' '));
  params = params.replace(/\$define/g,define);
  params = params.replace(/\$include/g,include);
  params = params.replace(/\$intermediate/g,target.get('intermediate'));
  
  if (run)
  {
    if (doBuild)
    { 
        writeln(file.cdata);
       if (!runCommand(cmd + ' ' + params,cmds)) 
          throw(file.cdata); //+'\r\n'+cmd+' '+params);   
    }
    else
    {
     cmds.writeln(cmd + ' ' + params);  
    }
  }
  return outfile;
}

function fixPath(path,platform,mode)
{
 if (path)
 {
   var replacements = filter(project.find('path'),platform,mode);
   if (replacements)
      for (var i=0;i<replacements.length; i++) 
      {
       var r = new RegExp(replacements[i].get('name'),'g')
       path = path.replace(r,replacements[i].cdata);
      }
 }
 return path;
}

function evaluateMacro(obj,platform,mode)
{
  var p = obj.get('macro')
  if (!p) return obj.cdata
  var macro = filter(platform.find('macro','name',p),null,mode)
  if (!macro.length) macro = filter(mode.find('macro','name',p),platform,null)
  if (!macro.length) macro = filter(project.find('macro','name',p),platform,mode)
  if (macro.length)
  {
    return macro[0].cdata.replace(/\$\w+/g, function(n) {return obj.get(n.substr(1))})
  }
  else
    return obj.cdata  
}

function doConfigure(commands,configure,platform,mode)
{
 try 
 {
  for (var i in configure)
  {
    if (objMatches(configure[i],platform,mode)) 
    {
      var cmd = evaluateMacro(configure[i],platform,mode) //configure[i].cdata;
      if (doBuild)
      { 
        writeln(cmd);
        if (!runCommand(cmd,commands)) 
           throw(cmd); //+'\r\n'+cmd+' '+params);
      }
      else
      {
        commands.writeln(cmd);  
      }    
    }
  }
 } catch(err)
 {
   writeln(err)
   return false
 }
}
//defines and includes are additive options

function doCompile(cmds,project,target,platform,mode,filters)
{
  var defaultCompiler = platform.find('compile')[0];
  
  var objFiles = [];
  
  var gnames = target.find('group');
  var groups = [target]; //include top-level <file> tags
  var i, j, files, filename, type, compiler;

  for (i in gnames)
  {
   if (gnames[i].get('version'))
     groups = groups.concat(project.find('group','name',gnames[i].cdata,'version',gnames[i].get('version')));
   else
     groups = groups.concat(project.find('group','name',gnames[i].cdata));
  }
  
  for (i in groups)
  {
    if (objMatches(groups[i],platform,mode))
    {
      files = groups[i].find('file');
      for (j in files)
      {
        filename = files[j].cdata;
        var run = false;
        
        if (filters.length)
        {
         for each(f in filters)
         {
          if (f.indexOf('*') != -1)
          {
           if (filename.match(new RegExp('^' + f.replace(/\*/g,'.*'))))
             run = true;
          }
          else if (filename.toUpperCase().search(f.toUpperCase()) != -1)
          {
           run = true;
           break;
          }
         }
        }
        else run = !linkOnly;

        if (objMatches(files[j],platform,mode))
        {
          type = filename.substr(filename.lastIndexOf('.'));
          compiler = platform.find('compile','input',type)[0];
          if (!compiler) compiler = defaultCompiler;
        
          if (startAt)
          {
           if (filename == startAt)
             startAt = '';
           else
           {           
             //objFiles.push(filename.replace(/\.\w*$/,compiler.get('output')));
             objFiles.push(escapeIntermediate(filename,compiler))
             continue;
           }
          }
          
          var file = compileFile(cmds,compiler,target,platform,mode,groups[i],files[j],run);
          if (file) objFiles.push(file);
        }             
      }
     }
  } 
  return objFiles;
}

function findFile(target,linker,file)
{
 if (!linker.get('resolve')) return file;
 return target.get('intermediate')+file;
 return file;
}

function doLink(cmds,objFiles,target,platform,mode)
{
  //link step

  var linker = platform.find('link','mode',mode)[0];
  if (!linker) linker = platform.find('link')[0];  
  var cmd = fixPath(linker.get('path'),platform,mode);
  var params = linker.get('parameters');
  var infiles = ''; 
  var rfiles = '';
  for (var i in objFiles) 
   { 
     if (objFiles[i].indexOf('.o') > 0) infiles += findFile(target,linker,objFiles[i])+' ';
     else rfiles += findFile(target,linker,objFiles[i]) + ' ';
    }
  var options = [getAttribute('options',target)];
  options = options.concat(getProperty('options',globalDefines,linker,platform,mode));
  
  //<link parameters="$options, C0X32 $input,$output,$mapfile,IMPORT32 CW32MT,$deffile " >
  //<target platform=win32 output="jsdb.exe" mapfile="jsdb.map" deffile="jsdb.def" intermediate="obj\">

  var libraries = target.get('libraries').split(';');

  var templates = linker.find('template','name','library');
  if (templates.length)
  {
      libraries = templateize(templates[0].cdata,libraries).join(' ')
  }
  
  params = params.replace(/\$input/g,infiles);
  params = params.replace(/\$resource/g,rfiles);
  params = params.replace(/\$output/g,target.get('output'));
  params = params.replace(/\$mapfile/g,target.get('mapfile'));
  params = params.replace(/\$deffile/g,target.get('deffile'));
  params = params.replace(/\$libraries/g,libraries);
  params = params.replace(/\$options/g,options.join(' '));  
  params = params.replace(/\$intermediate/g,target.get('intermediate'));
  
  if (doBuild)
  { 
    writeln(target.get('output'));
    if (!runCommand(cmd + ' ' + params,cmds)) 
      throw(target.get('output')+'\r\n'+cmd+' '+params);
  }
  else
  {
    cmds.writeln(cmd + ' ' + params); 
  }
}

function runMake(project,platform)
{ 
 var targetNames = [];
 var mode = null; 
 var filters = [];
 
 var msg = ''
 var error = null

 while (system.arguments.length)
 { 
   var p = system.arguments.shift()
   if (p.toLowerCase() == '/preview')
     doBuild = false;
   else if (p.toLowerCase() == '/quiet')
     quiet = true;
   else if (p.toLowerCase() == '/link')
     linkOnly = true;
   else if (p.toLowerCase() == '/verbose')
     verbose = true;
   else if (p.toLowerCase() == '/stopwarn')
     stopEarly = true;
   else if (p.toLowerCase() == '/resume')
   {
     var x = new Stream(logFile)
     startAt = x.readLine()
     x.close()
     configure = false;
   }
   else if (p == '/define') 
   {
    globalDefines = system.arguments.shift() + ';'
   }
   else if (p[0] == '/') 
   {
    var x = project.find('mode','name',p.substr(1));
    if (!mode && x.length)
      mode = x[0];
   }
   else 
   {
    var x = project.find('target','name',p);
    if (x.length) 
    {
     targetNames.push(p);
     msg += "Targeting " + p;
    }
    else
    {
     filters.push(p);
     msg += "Compiling *" + p + "* ";
    }
   }
   // else other flags     
 }

 if (doBuild) writeln(msg)
 if (!mode) mode = project.find('mode')[0];
 
 var targets = project.find('target');

 if (doBuild) writeln('Building ',project.get('name'),' for ',platform.get('name'));
 
 var commands = new Stream; 
 
 if (debug) writeln(1)
 // run global and platform-dependent configure scripts
 
 if (configure && !linkOnly && filters.length == 0)
 {
  doConfigure(commands,platform.find('configure'),null,mode);
  doConfigure(commands,project.find('configure'),platform,mode);
 }

 // look over potential targets
 try {
 for (var i in targets)
  {
   if (targetNames.length && targetNames.indexOf(targets[i].get('name')) == -1) continue;
   if (!targetNames.length && targets[i].get('automatic') == '0') continue;

   if (objMatches(targets[i],platform,mode)) 
   {
     if (doBuild) 
     { 
       writeln('Target: ',targets[i].get('name'))
       writeln('Platform: ',platform.get('name'))
       writeln('Output: ',targets[i].get('output'))
       writeln('Mode: ',mode.get('name'))
     }
     if (!linkOnly)
      doConfigure(commands,targets[i].find('configure'),platform,mode);

     var objs = doCompile(commands,project,targets[i],platform,mode,filters);

     if (linkOnly || filters.length == 0) // don't link if we're just filtering
     {
      doLink(commands,objs,targets[i],platform,mode);
     }
   }
  } 
 } catch(err) 
 {
  error = (err).toString()
  writeln("Error in ", err); 
 }

 // run platform-dependent configure scripts if linking (no compile filters)
 if (filters.length == 0)
 {
  doConfigure(commands,platform.find('finish'),null,mode);
  doConfigure(commands,project.find('finish'),platform,mode);
 }
 // done!
 if (!quiet)
 {
   commands.rewind();
   write(commands);
 }
 commands.rewind();
 log = new Stream(logFile,'wt');
 if (error) log.writeln(error)
 log.append(commands);
 log.close()
}

var help = "Usage: jsdb make.js jsdb.project platform [target ...] [file ...] [/debug] [/preview] [/quiet] [/verbose] [/stopwarn] [/link] [/resume] [/define ...]"
if (typeof(system.arguments) == 'undefined' || system.arguments.length == 0)
{
  writeln(help)
}
else
{
 load('xml.js')

 var projectName = system.arguments.shift()
 if (projectName.indexOf(".project") == -1 && system.exists(projectName+ '.project')) 
     projectName += ".project"

 logFile = projectName.replace(".project",".makelog")
// writeln(projectName)

 var project = XML.read(new Stream(projectName),'target,project,mode,configure,finish,define,compile,link,group,file,platform,options,template,macro,path');

 var platformName = '' //project.get('platform')

 if (system.arguments.length)
   platformName = system.arguments.shift()
   
 if (platformName)
 {
   var platform = project.find('platform','name',platformName)[0];
   if (platform) 
     runMake(project,platform)
   else
     writeln("Unknown platform: ",platform)
 }
 else
 {
  writeln(help) 

  writeln("Platforms")
  var l = project.find('platform');
  for (var i in l) writeln(" ",l[i].get('name'));
  
  writeln("Modes")
  var l = project.find('mode');
  for (var i in l) writeln(" ",l[i].get('name'));
  
  writeln("Targets")
  var l = project.find('target');
  for (var i in l) writeln(" ",l[i].get('name'),": ",l[i].get('output')," (",l[i].get('platform'),")");
 }
}
