@ECHO OFF
cl.exe /EHsc IATHookTest.cpp ../DLLInjectionAttack/Injector.cpp ../../Core/MemWarsCore.c /link user32.lib Advapi32.lib Shlwapi.lib