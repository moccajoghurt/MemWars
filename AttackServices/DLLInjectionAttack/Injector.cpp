#include <windows.h>
#include "Injector.h"


BOOL LoadDll(HANDLE hProcess, const WCHAR* dllPath) {
	int namelen = wcslen(dllPath) + 1;
    LPVOID remoteMemory = VirtualAllocEx(hProcess, NULL, namelen * sizeof(WCHAR), MEM_COMMIT, PAGE_EXECUTE);
    if (remoteMemory == NULL) {
        return FALSE;
    }
    SIZE_T ret = WriteProcessMemory(hProcess, remoteMemory, dllPath, namelen * sizeof(WCHAR), NULL);
    if (ret == 0) {
        return FALSE;
    }

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    if (k32 == NULL) {
        return FALSE;
    }
    LPVOID funcAddr = GetProcAddress(k32, "LoadLibraryW");
    if (funcAddr == NULL) {
        return FALSE;
    }

    HANDLE thread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)funcAddr, remoteMemory, NULL, NULL);
    if (thread == NULL) {
        return FALSE;
    }
	
    DWORD status = WaitForSingleObject(thread, INFINITE);
    if (status == WAIT_FAILED) {
        return FALSE;
    }
    CloseHandle(thread);
    
    return TRUE;
}

