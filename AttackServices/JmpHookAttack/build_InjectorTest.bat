@ECHO OFF
cl.exe /EHsc InjectorTest.cpp ../DLLInjectionAttack/Injector.cpp ../../Core/MemWarsCore.c /link User32.lib Kernel32.lib Advapi32.lib Shlwapi.lib