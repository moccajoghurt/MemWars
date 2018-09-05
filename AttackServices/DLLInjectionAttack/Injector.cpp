#include <windows.h>
#include <iostream>
#include "Injector.h"

using namespace std;

int LoadDll(HANDLE hProcess, const WCHAR* dllName) {
	int namelen = wcslen(dllName) + 1;
    LPVOID remoteMemory = VirtualAllocEx(hProcess, NULL, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (remoteMemory == NULL) {
        return 1;
    }

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    if (k32 == NULL) {
        return 3;
    }

    FARPROC addrLoadLibraryW = GetProcAddress(k32, "LoadLibraryW");
    FARPROC addrExitThread = GetProcAddress(k32, "ExitThread");
    if (addrLoadLibraryW == NULL || addrExitThread == NULL) {
        return 4;
    }

    void* rwMemory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (rwMemory == NULL) {
        cout << "VirtualAlloc returned NULL. Memory full? Error code: " << GetLastError() <<endl;
        return 1;
    }

    BYTE loadLibraryCodeCave[] = {
        0x48, 0xB9, 0, 0, 0, 0, 0, 0, 0, 0,         // mov rcx (DLL name)
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,         // mov rax (LoadLibraryW Process Address)
        0x48, 0x83, 0xEC, 0x20,				        // sub rsp 0x20
		0xFF, 0xD0,							        // call rax
		0x48, 0x83, 0xC4, 0x20,				        // add rsp, 0x20
        0x48, 0x31, 0xC9,                           // xor rcx, rcx (Set Parameter for ExitThread to 0)
        0x48, 0x83, 0xF8, 0x00,                     // cmp rax, 0x00 (Check if LoadLibrary return NULL)
        0x74, 0x07,                                 // je rel + 7 (Skip next instruction if LoadLibrary returned NULL)
        0x48, 0xC7, 0xC1, 0x01, 0x00, 0x00, 0x00,   // mov rcx, 0x1 (Set Parameter for ExitThread to 1)
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,         // mov rax (ExitThread Process Address)
        0x48, 0x83, 0xEC, 0x20,				        // sub rsp 0x20
		0xFF, 0xD0,							        // call rax
    };
    

    size_t shellCodeSize = sizeof(loadLibraryCodeCave);

    *(DWORD64*)((PUCHAR)loadLibraryCodeCave + 2) = (DWORD64)(ULONG_PTR)remoteMemory + shellCodeSize;
	*(DWORD64*)((PUCHAR)loadLibraryCodeCave + 12) = (DWORD64)(ULONG_PTR)addrLoadLibraryW;
    *(DWORD64*)((PUCHAR)loadLibraryCodeCave + 48) = (DWORD64)(ULONG_PTR)addrExitThread;
    

    CopyMemory(rwMemory, loadLibraryCodeCave, sizeof(loadLibraryCodeCave));
    CopyMemory((void*)((DWORD64)rwMemory + shellCodeSize), dllName, namelen * sizeof(WCHAR));

    SIZE_T ret = WriteProcessMemory(hProcess, remoteMemory, rwMemory, 4096, NULL);
    if (ret == 0) {
        return 2;
    }


    HANDLE thread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)remoteMemory, NULL, 0, NULL);
    if (thread == NULL) {
        return 5;
    }
	
    DWORD status = WaitForSingleObject(thread, INFINITE);
    if (status == WAIT_FAILED) {
        return 6;
    }

    DWORD threadStatus;
    if (GetExitCodeThread(thread, &threadStatus)) {
        // cout << threadStatus << endl;
        if (threadStatus == 1) {
            return 0;
        } else {
            return 7;
        }
    } else {
        return 8;
    }
}

