var newheader = new Stream
var header = new Stream('jsdb.h','rt')

header.readUntil("JSDB_VERSION",newheader);
var version = header.readLine().replace(/[\"\s]+/g,'')
newheader.writeln('JSDB_VERSION "',version,'"')

header.readUntil("JSDB_DATE");
header.readLine()

var now = new Date;
newheader.writeln('#define JSDB_DATE "', now.toLocaleFormat("%b %d %Y"), '"')

newheader.append(header);
header.close();

//make sure to write \n instead of \r\n or Sun chokes
var header = new Stream('jsdb.h','wb')
newheader.rewind()
header.append(newheader)
header.close()

var data = {version:version, password:'7uh65jkHH9', date:now.toLocaleFormat("%Y%m%d")}

//HELP's first line is "JSDB " JSDB_VERSION " " JSDB_DATE "\n"

d = new Stream('version.txt','wt')
d.writeln("JSDB ",version," ",now.toLocaleFormat("%b %d %Y"))

v = new Stream('version.js','wt')
v.writeln("a = system.resource('version.txt').readText()");
v.writeln("b = new Stream('http://www.jsdb.org/version.txt')");
v.writeln("b.readMIME()")
v.writeln("b=b.readText()")
v.writeln("if (a != b) system.browse('http://www.jsdb.org/update.html')");
v.writeln("else writeln('You have the latest version of JSDB.')");

d.close()

filelist = 'version.txt license.txt httpd.js test.jsp version.js svd.js matrix.js matrix.html jsdb.html debugconsole.js debug.js db.jsp db.js common.js test.cgi jsdbhelp.js jsdbhelp.xml jsdbmenu.html jsdbhelp.html perfect.js ado.js xml.js mailer.js json.js ';

makeimage = [
'echo {date}',
'zip jsdb_win_{version}.zip jsdb.exe',
'for %%x in (' , filelist, ') do zip jsdb_win_{version}.zip %%x',
'if !%1==!win goto upload',
'copy jsdb.linux jsdb',
'zip jsdb_linux_{version}.zip jsdb ',
'for %%x in (' , filelist, ') do zip jsdb_linux_{version}.zip %%x',
'copy jsdb.linuxnosql jsdb',
'zip jsdb_linux_min_{version}.zip jsdb ',
'for %%x in (' , filelist, ') do zip jsdb_linux_{version}.zip %%x',
'if !%1==!nomac goto upload',
'copy jsdb.osx jsdb',
'zip jsdb_mac_{version}.zip jsdb ',
'for %%x in (' , filelist, ') do zip jsdb_mac_{version}.zip %%x',
'rem jsdb',
':upload',
'scp -pw {password} jsdbhelp.html raosoft@www.raosoft.com:jsdb/jsdbhelp.html',
'scp -pw {password} jsdbbar.html raosoft@www.raosoft.com:jsdb/sidebar.html',
'scp -pw {password} jsdbmenu.html raosoft@www.raosoft.com:jsdb/jsdbmenu.html',
//'scp -pw {password} ezshelp.html raosoft@www.raosoft.com:jsdb/ezshelp.html',
//'scp -pw {password} ezrhelp.html raosoft@www.raosoft.com:jsdb/ezrhelp.html',
//'scp -pw {password} ezsbar.html raosoft@www.raosoft.com:jsdb/ezsbar.html',
//'scp -pw {password} ezrbar.html raosoft@www.raosoft.com:jsdb/ezrbar.html',
'scp -pw {password} version.txt raosoft@www.raosoft.com:jsdb/version.txt',
'scp -pw {password} rs\\wrap_com.cpp raosoft@www.raosoft.com:jsdb/wrap_com.cpp',
'scp -pw {password} rs\\wrap_debug.cpp raosoft@www.raosoft.com:jsdb/wrap_debug.cpp',
'scp -pw {password} jsdb.h raosoft@www.raosoft.com:jsdb/jsdb.h',
'scp -pw {password} source.zip raosoft@www.raosoft.com:jsdb/jsdb_source_{version}.zip ',
'if exist jsdb_win_{version}.zip scp -pw {password} jsdb_win_{version}.zip raosoft@www.raosoft.com:jsdb/jsdb_win_{version}.zip ',
'if !%1==!win goto end',
'if exist jsdb_linux_{version}.zip scp -pw {password} jsdb_linux_{version}.zip raosoft@www.raosoft.com:jsdb/jsdb_linux_{version}.zip ',
'if exist jsdb_linux_min_{version}.zip scp -pw {password} jsdb_linux_min_{version}.zip raosoft@www.raosoft.com:jsdb/jsdb_linux_min_{version}.zip ',
'if !%1==!nomac goto end',
'if exist jsdb_mac_{version}.zip scp -pw {password} jsdb_mac_{version}.zip raosoft@www.raosoft.com:jsdb/jsdb_mac_{version}.zip ',
':end',
'scp -pw {password} latest.html raosoft@www.raosoft.com:jsdb/latest.html',
'rem scp -pw {password} download.html raosoft@www.raosoft.com:jsdb/download.html',
]

d = new Stream('makeimage.bat','wt')
for each (var s in makeimage)
 d.writeln(s.replace(/{(.*?)}/g,function(f,g){return data[g]}))
d.close()

latest = [
'Download <!--#if expr="$HTTP_USER_AGENT = /Windows/" -->',
'<a href=jsdb_win_{version}.zip>JSDB for Windows</a>.',
'<br>Unzip the package anywhere on your computer.',
'<br>Double-click the JSDB icon.',
'<!--#elif expr="$HTTP_USER_AGENT = /Linux/" -->',
'Download <a href=jsdb_linux_{version}.zip>Download JSDB for Linux (x86, with ODBC)</a> or',
'<a href=jsdb_linux_min_{version}.zip>(x86, without ODBC)</a>.<br>',
'Unzip the package anywhere on your computer.',
'<br><tt>chmod +x jsdb</tt>',
'<br><tt>./jsdb</tt>',
'<!--#elif expr="$HTTP_USER_AGENT = /Intel Mac OS X/" -->',
'<a href=jsdb_mac_{version}.zip>JSDB for Intel Mac</a>.',
'<br>Unzip the package anywhere on your computer.',
'<br>Open a terminal window and <tt>cd</tt> to that folder.',
'<br><tt>chmod +x jsdb</tt>',
'<!--#else -->',
'JSDB for <a href=jsdb_win_{version}.zip>Windows</a> or ',
'<a href=jsdb_linux_{version}.zip>Linux (x86)</a>, or',
'<a href=jsdb_linux_min_{version}.zip>Linux (x86, no ODBC)</a>, or',
'<a href=jsdb_mac_{version}.zip>Mac OS X (x86)</a>, or',
'<a href=jsdb_source_{version}.zip>compile the source code</a>.',
'<!--#endif -->'
]

d = new Stream('latest.html','wt')
for each (var s in latest)
 d.writeln(s.replace(/{version}/g,version))
d.close()
