var header = new Stream('jsdb.h','rt')
header.readUntil("JSDB_VERSION");
var version = header.readLine().replace(/\"/g,'').replace(/\./g,',')
header.close();

var f = new Stream('jsdb.rc','wt');
var today = new Date;
f.writeln('1 VERSIONINFO');
f.writeln('FILEVERSION ',today.getYear()-100,',',today.getMonth()+1,',',today.getDate(),', 0');
f.writeln('PRODUCTVERSION ' + version);
f.writeln('FILEOS 0x00000004L');
f.writeln('FILETYPE 0x00000001L');
f.writeln('{');
f.writeln(' BLOCK "StringFileInfo"');
f.writeln(' {');
f.writeln('  BLOCK "040904E4"');
f.writeln('  {');
f.writeln('   VALUE "CompanyName", "Raosoft, Inc."');
f.writeln('   VALUE "FileDescription", "JavaScript for Databases by JSDB.org"');
f.writeln('   VALUE "FileVersion", "',today.getYear()-100,'.',today.getMonth()+1,',',today.getDate(),'"');
f.writeln('   VALUE "InternalName", "JSDB"');
f.writeln('   VALUE "LegalCopyright", "Copyright 2003-',today.getYear()+1900,' by Shanti Rao and others. JSDB is a registered trademark of Shanti Rao."');
f.writeln('   VALUE "OriginalFilename", "jsdb.exe"');
f.writeln('  }');
f.writeln('');
f.writeln(' }');
f.writeln('');
f.writeln(' BLOCK "VarFileInfo"');
f.writeln(' {');
f.writeln('  VALUE "Translation", 0x409, 1252');
f.writeln(' }');
f.writeln('');
f.writeln('}');
f.writeln('');
f.writeln('1 ICON "jsdb.ico"');
