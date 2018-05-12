@ECHO OFF
cl.exe /EHsc ThreadHijackTest.cpp ThreadHijack.cpp ../../Core/MemWarsCore.c /link User32.lib Kernel32.lib Advapi32.lib