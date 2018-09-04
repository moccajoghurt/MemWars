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


    FARPROC addrCloseHandle = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "CloseHandle");
	FARPROC addrCreateFileA = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "CreateFileA");
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

    getchar();

    BYTE loadLibraryCodeCave[] = {
        0x48, 0xB9, 0, 0, 0, 0, 0, 0, 0, 0, // mov rcx (DLL name)
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0, // mov rax (LoadLibraryA Process Address)
        0x48, 0x83, 0xEC, 0x20,				// sub rsp 0x20
		0xFF, 0xD0,							// call rax
		0x48, 0x83, 0xC4, 0x20,				// add rsp, 0x20
        0x48, 0x83, 0xF8, 0x00,             // cmp rax, 0x00
        0x75, 0x01,                         // jne rel + 1
        0xC3,                               // ret
    };

    BYTE fileCreationCodeCave[] = {
		0x48, 0x83, 0xE4, 0xF0,				// +0 and rsp, 0x0f (make sure stack 16-byte aligned)
		0x48, 0xB9, 0, 0, 0, 0, 0, 0, 0, 0, // +4 mov rcx (lpFileName)
        0x48, 0xBA, 0, 0, 0, 0, 0, 0, 0, 0, // +14 mov rdx (dwDesiredAccess)
        0x4D, 0x31, 0xC0,                   // +24 xor r8,r8 (dwShareMode)
		0x4D, 0x31, 0xC9,                   // +27 xor r9,r9 (lpSecurityAttributes)
		0x48, 0x83, 0xEC, 0x08,             // +30 sub rsp, 0x08 (We are pushing 3 Parameters = 24 byte. Stack must be 16 byte aligned, so we add 8 byte beforehand)
        0x68, 0x00, 0x00, 0x00, 0x00,       // +34 push 0x0 (hTemplateFile)
        0x68, 0x80, 0x00, 0x00, 0x00,       // +39 push 0x80 (dwFlagsAndAttributes)
        0x68, 0x02, 0x00, 0x00, 0x00,       // +44 push 0x2 (dwCreationDisposition)
		0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0, // +49 mov rax (CreateFileA Process Address)
		0x48, 0x83, 0xEC, 0x20,             // +59 sub rsp, 0x20 (save 32 byte for Windows parameters - must be always done)
		0xFF, 0xD0,                         // +63 call rax
		0x48, 0x83, 0xC4, 0x40,				// +65 add rsp, 0x40 (0x20 + 3 Parameters + 8 Byte alignment buf)
		0x48, 0x89, 0xC1,					// +69 mov rcx, rax (HANDLE of CreateFileA)
		0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0, // +72 mov rax (CloseHandle Process Address)
		0x48, 0x83, 0xEC, 0x20,				// sub rsp 0x20
		0xFF, 0xD0,							// call rax
		0x48, 0x83, 0xC4, 0x20,				// add rsp, 0x20
        0xC3                                // ret
        // 0xEB, 0xFE                          // nop + jmp rel8 -2
	};

    size_t shellCodeSize = sizeof(loadLibraryCodeCave) + sizeof(fileCreationCodeCave);
    const TCHAR dllName[] = "InjectedDLLa.dll";
    const TCHAR filename[] = "testfile123.txt";

    *(DWORD64*)((PUCHAR)loadLibraryCodeCave + 2) = (DWORD64)(ULONG_PTR)remoteMemory + shellCodeSize;
	*(DWORD64*)((PUCHAR)loadLibraryCodeCave + 12) = (DWORD64)(ULONG_PTR)addrLoadLibraryA;
    
	
	*(DWORD64*)((PUCHAR)fileCreationCodeCave + 6) = (DWORD64)(ULONG_PTR)remoteMemory + shellCodeSize + sizeof(dllName);
	*(DWORD64*)((PUCHAR)fileCreationCodeCave + 16) = GENERIC_READ | GENERIC_WRITE;
    *(DWORD64*)((PUCHAR)fileCreationCodeCave + 51) = (DWORD64)(ULONG_PTR)addrCreateFileA;
	*(DWORD64*)((PUCHAR)fileCreationCodeCave + 74) = (DWORD64)(ULONG_PTR)addrCloseHandle;

    

    CopyMemory(rwMemory, loadLibraryCodeCave, sizeof(loadLibraryCodeCave));
    CopyMemory((void*)((DWORD64)rwMemory + sizeof(loadLibraryCodeCave)), fileCreationCodeCave, sizeof(fileCreationCodeCave));
    CopyMemory((void*)((DWORD64)rwMemory + shellCodeSize), &dllName, sizeof(dllName));
    CopyMemory((void*)((DWORD64)rwMemory + shellCodeSize + sizeof(dllName)), &filename, sizeof(filename));

    WriteProcessMemory(process, remoteMemory, rwMemory, 4096, NULL);
    
    HANDLE hRemoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)remoteMemory, NULL, 0, NULL);
	if (!hRemoteThread) {
        printf("CreateRemoteThread() failed: %d", GetLastError());
    }
    

    Sleep(10); // give the shellcode a bit time
    Exit:;
    // system("taskkill /IM memoryTestApp.exe /F >nul");
}