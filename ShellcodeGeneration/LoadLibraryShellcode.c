/*
* This file injects shellcode that calls CreateFileA
* into a test app and starts a remote threat to 
* execute the shellcode
*/


#include <windows.h>
#include "../Core/MemWarsCore.h"

int main() {
    system("start memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    
	void* remoteMemory = VirtualAllocEx(process, NULL, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (remoteMemory == NULL) {
		printf("VirtualAllocEx returned NULL\n");
	}

    FARPROC addrLoadLibraryA = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "LoadLibraryA");
    FARPROC addrExitThread = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "ExitThread");
    
    if (addrCreateFileA == NULL || addrCloseHandle == NULL || addrLoadLibraryA == NULL || addrExitThread == NULL) {
        printf("GetProcAddress returned NULL\n");
        return 1;
	}
    void* rwMemory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (rwMemory == NULL) {
        printf("Virtual Alloc returned NULL\n");
        goto Exit;
    }

    // getchar();

    BYTE loadLibraryCodeCave[] = {
        0x48, 0xB9, 0, 0, 0, 0, 0, 0, 0, 0,         // mov rcx (DLL name)
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,         // mov rax (LoadLibraryA Process Address)
        0x48, 0x83, 0xEC, 0x20,				        // sub rsp 0x20
		0xFF, 0xD0,							        // call rax
		0x48, 0x83, 0xC4, 0x20,				        // add rsp, 0x20
        0x48, 0x31, 0xC9,                           // xor rcx, rcx
        0x48, 0x83, 0xF8, 0x00,                     // cmp rax, 0x00
        0x74, 0x07,                                 // je rel + 7
        0x48, 0xC7, 0xC1, 0x01, 0x00, 0x00, 0x00,   // mov rcx, 0x1
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,         // mov rax (ExitThread Process Address)
        0x48, 0x83, 0xEC, 0x20,				        // sub rsp 0x20
		0xFF, 0xD0,							        // call rax
    };

    

    size_t shellCodeSize = sizeof(loadLibraryCodeCave);
    const TCHAR dllName[] = "InjectedDLL.dll";

    *(DWORD64*)((PUCHAR)loadLibraryCodeCave + 2) = (DWORD64)(ULONG_PTR)remoteMemory + shellCodeSize;
	*(DWORD64*)((PUCHAR)loadLibraryCodeCave + 12) = (DWORD64)(ULONG_PTR)addrLoadLibraryA;
    *(DWORD64*)((PUCHAR)loadLibraryCodeCave + 48) = (DWORD64)(ULONG_PTR)addrExitThread;

    

    CopyMemory(rwMemory, loadLibraryCodeCave, sizeof(loadLibraryCodeCave));
    CopyMemory((void*)((DWORD64)rwMemory + shellCodeSize), &dllName, sizeof(dllName));

    WriteProcessMemory(process, remoteMemory, rwMemory, 4096, NULL);
    
    HANDLE hRemoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)remoteMemory, NULL, 0, NULL);
	if (!hRemoteThread) {
        printf("CreateRemoteThread() failed: %d", GetLastError());
    }
    

    Sleep(200); // give the shellcode a bit time

    DWORD threadStatus;
    if (GetExitCodeThread(hRemoteThread, &threadStatus)) {
        printf("%x\n", threadStatus);
    } else {
        printf("error\n");
    }

    

    Exit:;
    // system("taskkill /IM memoryTestApp.exe /F >nul");
}