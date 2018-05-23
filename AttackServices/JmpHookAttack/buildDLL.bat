@ECHO OFF
cl.exe /EHsc /LD InjectedDLL.cpp ../../Core/MemWarsServicesCore.cpp /link user32.lib kernel32.lib Advapi32.lib Ntdll.lib