For license terms, please read license.txt.

To compile JSDB for Linux

1. chmod +x makejsdb.csh
2. ./makejsdb.csh
3. Copy the support files to /etc/jsdb/

To compile JSDB for Linux with iODBC

1. chmod +x makejsdbiodbc.csh
2. ./makejsdbiodbc.csh
3. Copy the support files to /etc/jsdb/

To compile JSDB for Linux without ODBC

1. chmod +x makejsdbnosql.csh
2. ./makejsdbnosql.csh
3. Copy the support files to /etc/jsdb/

To generate the Linux build script

   ./jsdb make.js jsdb.project linux linux /preview > makejsdb.csh

   or, without ODBC,

   ./jsdb make.js jsdb.project linux linuxnosql /preview > makejsdb.csh

To compile JSDB for Windows with GCC

1. jsdb.exe make.js jsdb.project win32

To compile JSDB for OSX with XCode

1. ./jsdb make.js jsdb.project osx

To compile JSDB for Win32 with the Borland C++ compiler

1. Open jsdb.project in your favorite text editor.
2. Search for "bcc55" in 3 places, and change the include and library paths as appropriate
3. jsdb.exe make.js jsdb.project winbcc

The source code for JSDB is based on SpiderMonkey, with the following changes:

1. Add parenthesis around ambiguous shift operators.
2. Insert before jsobj.c#2503
     lasbobj = obj = cx->fp->scopeChain;
     if (obj) 
3. Disable __declspec(dllimport) in jstypes.h
4. Replace JSLL_INIT macros in prmjtime.c and jslong.c

JSDB is a trademark registered to Shanti Rao, who would like to hear from you at jsdb@shantirao.com.
