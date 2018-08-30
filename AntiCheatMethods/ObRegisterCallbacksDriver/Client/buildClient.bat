@ECHO OFF
cl.exe /EHsc /source-charset:utf-8 client.cpp utils.cpp /link user32.lib kernel32.lib ntdll.lib Advapi32.lib