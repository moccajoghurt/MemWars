@ECHO OFF
cl.exe /EHsc InjectorTest.cpp Injector.cpp ../../Core/MemWarsCore.c /link User32.lib Kernel32.lib Advapi32.lib