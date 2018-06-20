# Basic DLL Injection



This attack is a simple DLL injection that uses the function[VirtualAllocEx](https://msdn.microsoft.com/en-us/library/windows/desktop/aa366890%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396) to reserve memory on an external process. Then[WriteProcessMemory](https://msdn.microsoft.com/de-de/library/windows/desktop/ms681674(v=vs.85).aspx) is used to write the DLL to the reserved memory.
Finally,[CreateRemoteThread](https://msdn.microsoft.com/en-us/library/windows/desktop/ms682437(v=vs.85).aspx) starts a thread within the external process that executes the DLL.