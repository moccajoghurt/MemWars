@ECHO OFF
cl.exe /EHsc Direct3DHookTest.cpp Direct3DHook.cpp ../DLLInjectionAttack/Injector.cpp ../../Core/MemWarsCore.c /link user32.lib Advapi32.lib Shlwapi.lib