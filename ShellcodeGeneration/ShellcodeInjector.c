#include <windows.h>
#include "../MemWarsCore/MemWarsCore.h"

int main() {
    system("start memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    
	PVOID pRemoteBuffer;
	pRemoteBuffer = VirtualAllocEx(process, NULL, 4096, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	if (!pRemoteBuffer) {
        printf("injectShellcode()::VirtualAllocEx() failed: %d", GetLastError());
	}


    FARPROC addrMessageBoxA = GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "MessageBoxA");
    FARPROC addrExitThread = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "ExitThread");
    if (addrMessageBoxA == NULL || addrExitThread == NULL) {
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
        0x48, 0x31, 0xC9,                           // xor    rcx,rcx
        0x48, 0x31, 0xD2,                           // xor    rdx,rdx
        0x4D, 0x31, 0xC0,                           // xor    r8,r8 (8)
        0x45, 0x31, 0xC9,                           // xor    r9d,r9d (11)
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,         // mov    rax,addr
        0xFF, 0xD0,                                 // call   rax
        0xEB, 0xFE                                  // nop + jmp rel8 -2
    };

    *(DWORD64*)((PUCHAR)injectBytes + 14) = (DWORD64)(ULONG_PTR)addrExitThread;
    CopyMemory(rwMemory, injectBytes, sizeof(injectBytes));
    // offset += sizeof(injectBytes);

    // for (int i = 0; i < sizeof(injectBytes); i++) {
    //     printf("0x%x\n", injectBytes[i]);
    // }
    // return;
    // UCHAR x64InfiniteLoop[] = { 0xEB, 0xFE }; // nop + jmp rel8 -2
	// CopyMemory((void*)addrEndOfShellCode, x64InfiniteLoop, sizeof(x64InfiniteLoop));
    // addrEndOfShellCode += sizeof(x64InfiniteLoop);
    
    
 
	// const char nameBuf[] = "hack.txt";
	// CopyMemory((void*)((DWORD64)rwMemory + offset), nameBuf, strlen(nameBuf));
    // offset += strlen(nameBuf);
    
    // DWORD64 lpNameInRemoteExecMemory = (DWORD64)pRemoteBuffer + offset - strlen(nameBuf);
	// CopyMemory((void*)((DWORD64)rwMemory + 42), &lpNameInRemoteExecMemory, sizeof(lpNameInRemoteExecMemory));

    
	if (!WriteProcessMemory(process, pRemoteBuffer, rwMemory, 4096, NULL)) {
        printf("injectShellcode()::WriteProcessMemory() failed: %d", GetLastError());
    }
    
    HANDLE hRemoteThread;
    // todo: add shellcode execution
	hRemoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteBuffer, NULL, 0, NULL);
	if (!hRemoteThread) {
        printf("injectShellcode()::CreateRemoteThread() failed: %d", GetLastError());
    }
    printf("press key to close memoryTestApp\n");
    getchar();

    Exit:;
    system("taskkill /IM memoryTestApp.exe /F >nul");
}