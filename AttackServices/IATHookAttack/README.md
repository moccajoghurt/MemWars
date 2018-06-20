# Import Address Table Hook



This attack is based on a DLL injection that changes the IAT addresses of the game process.
In this case, the function [GetCurrentThreadId](https://msdn.microsoft.com/de-de/library/windows/desktop/ms683183%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396) is hooked, which first calls our own function before the original function is executed.



The library [PolyHook](https://github.com/stevemk14ebr/PolyHook) is used to change the pointers in the IAT.


