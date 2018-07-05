# MemWars - Game Penetration Testing Framework

MemWars is a framework that executes popular and current attack methods on video games to detect vulnerabilities. It helps game developers to quickly discover and understand security vulnerabilities.

The attack methods can be divided into two categories:
- Basic attacks that manipulate the game process without being inconspicuous
- Advanced attacks that manipulate the game process and attempt to bypass anti-cheat methods.

## Attack methods

Each attack method has its own test environment and can be compiled separately.
The following attack methods are implemented:

- (basic) [DLL Injection](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/DLLInjectionAttack)
- (basic) [Direct3D 11 Hook](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/Direct3D11HookAttack)
- (basic) [Import Address Table Hook](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/IATHookAttack)
- (basic) [JMP Hook](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/JmpHookAttack)
- (basic) [Thread Hijacking](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/ThreadHijackAttack)
- (advanced) [System Process Injection](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/SystemProcessInjectionAttack)
- (advanced) [Capcom Driver Attack](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/CapcomDriverAttack)
- (under construction) [Hidden Kernel DLL Injection](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/HiddenKernelDLLInjectionAttack)


The project is still under development.

## TODO:

- Finishing the [Hidden Kernel DLL Injection](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/HiddenKernelDLLInjectionAttack)
	- finishing the physical memory controller that takes care of the kernel memory
	- implementing the copy on write hook that executes the DLL inside the kernel
- Development of automated execution of attacks on a desired process
- Development of a user interface for the framework
- Designing an output protocol for the test results of the framework