@ECHO OFF
cl.exe /EHsc ConsoleInterface.cpp ../PenetrationRoutines/ValueFinder/ValueFinder.cpp ../PenetrationRoutines/SPIAttackProvider/Client/Client.cpp ../AttackServices/SystemProcessInjectionAttack/StealthyMemManipulatorClient.cpp ../AttackServices/SystemProcessInjectionAttack/StealthyMemManipulatorGetHandleId.cpp  ../Core/MemWarsServicesCore.cpp ../Core/MemWarsCore.c ../AttackServices/SystemProcessInjectionAttack/spinlock.obj /link User32.lib Kernel32.lib Advapi32.lib Ntdll.lib