/*
* This file injects shellcode that calls MessageBoxA
* into a test app and starts a remote threat to 
* execute the shellcode
*/


#include <windows.h>
#include "../MemWarsCore/MemWarsCore.h"

int main() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    
	PVOID pRemoteBuffer;
	pRemoteBuffer = VirtualAllocEx(process, NULL, 4096, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	if (!pRemoteBuffer) {
        printf("VirtualAllocEx() failed: %d", GetLastError());
	}


    FARPROC addrMessageBoxA = GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "MessageBoxA");
    if (addrMessageBoxA == NULL) {
        printf("GetProcAddress returned NULL\n");
        goto Exit;
    }
    void* rwMemory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (rwMemory == NULL) {
        printf("Virtual Alloc returned NULL\n");
        goto Exit;
    }
    DWORD offset = 0;
    BYTE injectBytes[] = {
        0x48, 0x31, 0xC9,                           // xor    rcx,rcx
        0x48, 0x31, 0xD2,                           // xor    rdx,rdx
        0x4D, 0x31, 0xC0,                           // xor    r8,r8 (8)
        0x45, 0x31, 0xC9,                           // xor    r9d,r9d (11)
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,         // mov    rax,addr
        0x48, 0x83, 0xEC, 0x20,                     // sub rsp, 0x20
        0xFF, 0xD0,                                 // call   rax
        0xEB, 0xFE                                  // nop + jmp rel8 -2
    };

    *(DWORD64*)((PUCHAR)injectBytes + 14) = (DWORD64)(ULONG_PTR)addrMessageBoxA;
    CopyMemory(rwMemory, injectBytes, sizeof(injectBytes));
    
	if (!WriteProcessMemory(process, pRemoteBuffer, rwMemory, 4096, NULL)) {
        printf("WriteProcessMemory() failed: %d", GetLastError());
    }
    
    HANDLE hRemoteThread;
	hRemoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteBuffer, NULL, 0, NULL);
	if (!hRemoteThread) {
        printf("CreateRemoteThread() failed: %d", GetLastError());
    }
    printf("press Enter to close memoryTestApp\n");
    getchar();

    Exit:;
    system("taskkill /IM memoryTestApp.exe /F >nul");
}