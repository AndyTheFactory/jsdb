/**about
Description: JSDB reference web server
Creator: Shanti Rao
Version: 1
Date: 20119910
Requires: jsdb-1.8.0.6; javascript-1.7
*/
while (system.arguments.length)
{
 let data = eval(new Stream(system.arguments.shift()+'.package').readFile());
 let name = data.name+(data.version||'')
 writeln(name,'.zip')
 for each(let f in data.files) {write(f,'\t'); system.executeWait('zip.exe',name+' "'+f+'"');}
}
