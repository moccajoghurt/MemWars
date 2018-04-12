@ECHO OFF
cl.exe /EHsc ServicesTest.cpp MemWarsServices.cpp StealthyMemManipulator.cpp ../MemWarsCore/MemWarsCore.c /link User32.lib Kernel32.lib Advapi32.lib