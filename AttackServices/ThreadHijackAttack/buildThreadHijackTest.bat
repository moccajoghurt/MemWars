@ECHO OFF
cl.exe /EHsc ThreadHijackTest.cpp ThreadHijack.cpp ../../Core/MemWarsCore.c ../../Core/MemWarsServicesCore.cpp /link User32.lib Kernel32.lib Advapi32.lib Ntdll.lib Shlwapi.lib