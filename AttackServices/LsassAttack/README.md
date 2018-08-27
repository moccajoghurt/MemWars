# Lsass Attack



This is an advanced attack method that wants to bypass anti-cheat methods.



The attack is divided into two parts (installer and client), which should be executed separately, as the installer manipulates system processes, which are detected by anti-cheat systems. The installer will be executed before the game starts and the client after the game process has started.



The attack method can be summarized as follows:



In the system process lsass.exe shellcode is written, which contains the functions [ReadProcessMemory](https://msdn.microsoft.com/de-de/library/windows/desktop/ms680553%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396) and [WriteProcessMemory](https://msdn.microsoft.com/de-de/library/windows/desktop/ms681674(v=vs.85).aspx). This bypasses the anti-cheat method, which uses ObRegisterCallbacks to prohibit the authorization of HANDLE operations, since lsass.exe cannot get permissions revoked without this leading to system instabilities.



The manipulation of lsass.exe and the communication with the client is as inconspicuous as possible in order not to be detected by anti-cheat methods.




## Installer



- A file mapping is created, which serves as communication interface between lsass.exe and the client

- So that lsass.exe has no HANDLE on a file mapping (this is a detection vector), the file mapping is opened in the shellcode, the address of the file mapping is written to the memory of lsass.exe and the HANDLE is closed again. To ensure that file mapping continues to exist, explorer.exe is captured and a HANDLE for the file mapping is created there permanently. Thus the file mapping remains without the client or lsass.exe needing a HANDLE to it. A pointer to the file mapping is sufficient for communication.

- The installer looks for zeroed executable memory inside lsass.exe where it can write shellcode.
- To execute the shellcode in lsass.exe, an existing unnecessary thread of lsass.exe is captured. The dispensable threads execute the modules "samsrv.dll", "msvcrt.dll" and "crypt32.dll". Creating a new thread on lsass.exe would be another detection vector.
- After the installer is finished, the infiltrated shellcode of lsass.exe is constantly executed by one of the threads.
- The shellcode continuously checks one bit within the file mapping to check if a new command has arrived from the client.

- The file mapping contains a minimal Inter Process Communication protocol with which the client and lsass.exe communicate. The IPC contains, among other things, which process is to be read/written and which memory address is to be read/written.
- To signal to the client that new commands can be received, the bit in the file mapping is reset to 0.
- The results of [ReadProcessMemory](https://msdn.microsoft.com/de-de/library/windows/desktop/ms680553%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396) and [WriteProcessMemory](https://msdn.microsoft.com/de-de/library/windows/desktop/ms681674(v=vs.85).aspx) are written to the file mapping




## Client

- The client connects to the file mapping and uses the IPC protocol to send commands to lsass.exe