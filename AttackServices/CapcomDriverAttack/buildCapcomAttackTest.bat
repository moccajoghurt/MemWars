@ECHO OFF
cl.exe /EHsc CapcomAttackTest.cpp CapcomAttack.cpp CapcomLoader.cpp /link user32.lib ntdll.lib Advapi32.lib Shlwapi.lib