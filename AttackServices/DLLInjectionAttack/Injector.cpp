#include <windows.h>
#include "Injector.h"


int LoadDll(HANDLE hProcess, const WCHAR* dllPath) {
	int namelen = wcslen(dllPath) + 1;
    LPVOID remoteMemory = VirtualAllocEx(hProcess, NULL, namelen * sizeof(WCHAR), MEM_COMMIT, PAGE_EXECUTE);
    if (remoteMemory == NULL) {
        return 1;
    }
    SIZE_T ret = WriteProcessMemory(hProcess, remoteMemory, dllPath, namelen * sizeof(WCHAR), NULL);
    if (ret == 0) {
        return 2;
    }

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    if (k32 == NULL) {
        return 3;
    }
    LPVOID funcAddr = GetProcAddress(k32, "LoadLibraryW");
    if (funcAddr == NULL) {
        return 4;
    }

    HANDLE thread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)funcAddr, remoteMemory, NULL, NULL);
    if (thread == NULL) {
        return 5;
    }
	
    DWORD status = WaitForSingleObject(thread, INFINITE);
    if (status == WAIT_FAILED) {
        return 6;
    }
    CloseHandle(thread);
    
    return 0;
}

