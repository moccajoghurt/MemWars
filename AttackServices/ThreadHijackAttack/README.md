# Thread hijack attack



This attack uses [VirtualAllocEx](https://msdn.microsoft.com/en-us/library/windows/desktop/aa366890%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396) to reserve memory on an external process.



Then, the function [WriteProcessMemory](https://msdn.microsoft.com/de-de/library/windows/desktop/ms681674(v=vs.85).aspx) writes shellcode to that memory.


The main thread of the external process is then captured to execute the shellcode.



Finally, the original state of the thread is restored and executed further.