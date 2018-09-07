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
- (advanced) [Lsass Attack](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/LsassAttack)
- (advanced) [Capcom Driver Attack](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/CapcomDriverAttack)
- (advanced) [Hidden Kernel DLL Injection](https://github.com/moccajoghurt/MemWars/tree/master/AttackServices/HiddenKernelDLLInjectionAttack)


The project is still under development.

## TODO:
- formalize tests
- development of a method that determines whether network data are encrypted
- Security checks before execution of certain attacks
