/*
* This file injects shellcode that calls CreateFileA
* into a test app and starts a remote threat to 
* execute the shellcode
*/


#include <windows.h>
#include "../Core/MemWarsCore.h"

int main() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    
	PVOID pRemoteBuffer;
	pRemoteBuffer = VirtualAllocEx(process, NULL, 4096, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	if (!pRemoteBuffer) {
        printf("VirtualAllocEx() failed: %d", GetLastError());
    }
    
    getchar();


    FARPROC addrCreateFileA = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "CreateFileA");
    if (addrCreateFileA == NULL) {
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
        0x48, 0xB9, 0, 0, 0, 0, 0, 0, 0, 0, // +0 mov rcx, 64bit
        0x48, 0xBA, 0, 0, 0, 0, 0, 0, 0, 0, // +10 mov rdx, 64bit
        0x45, 0x31, 0xC0,                   // +20 xor r8d,r8d
        0x45, 0x31, 0xC9,                   // +23 xor r9d,r9d
        0x6A, 0x00,                         // +26 push 0x0 (hTemplateFile)
        0x68, 0x80, 0x00, 0x00, 0x00,       // +28 push 0x80 (dwFlagsAndAttributes)
        0x6A, 0x01,                         // +33 push 0x1 (dwCreationDisposition)
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0, // +35 mov rax, 64bit
        0x48, 0x83, 0xEC, 0x20,             // +45 sub rsp, 0x20
        0xFF, 0xD0,                         // +49 call rax
        0x48, 0x83, 0xC4, 0x38,             // +51 add rsp, 0x38
        0xEB, 0xFE                          // nop + jmp rel8 -2
    };


    *(DWORD64*)((PUCHAR)injectBytes + 2) = (DWORD64)(ULONG_PTR)pRemoteBuffer + sizeof(injectBytes);
    *(DWORD64*)((PUCHAR)injectBytes + 12) = (DWORD64)0x40000000;
    *(DWORD64*)((PUCHAR)injectBytes + 37) = (DWORD64)(ULONG_PTR)addrCreateFileA;

    CopyMemory(rwMemory, injectBytes, sizeof(injectBytes));

    UCHAR filename[] = "testfile1234567.txt";

    CopyMemory((void*)((DWORD64)rwMemory + sizeof(injectBytes)), filename, sizeof(filename));
    
	if (!WriteProcessMemory(process, pRemoteBuffer, rwMemory, 4096, NULL)) {
        printf("WriteProcessMemory() failed: %d", GetLastError());
    }
    
    HANDLE hRemoteThread;
	hRemoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteBuffer, NULL, 0, NULL);
	if (!hRemoteThread) {
        printf("CreateRemoteThread() failed: %d", GetLastError());
    }
    

    Sleep(100000000); // give the shellcode a bit time
    Exit:;
    // system("taskkill /IM memoryTestApp.exe /F >nul");
}