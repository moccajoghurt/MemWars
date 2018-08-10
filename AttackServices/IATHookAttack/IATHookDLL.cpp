#include <windows.h>
#include <iostream>
#include "../../libs/PolyHook/PolyHook.hpp"

using namespace std;

typedef DWORD(__stdcall* tGetCurrentThreadId)();
tGetCurrentThreadId pGetCurrentThreadID;

DWORD __stdcall GetCurrentThreadIDHook() {
    // CreateFileA("IATHookConfirmationFile", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    lstrcatA(tempPath, "dllInjectionConfirmationFile");
    HANDLE h = CreateFileA(tempPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(h);
	return pGetCurrentThreadID();
}

DWORD WINAPI InitializeHook(LPVOID lpParam) {
    shared_ptr<PLH::IATHook> IATHook_Ex(new PLH::IATHook);
    IATHook_Ex->SetupHook("kernel32.dll", "GetCurrentThreadId", (BYTE*)&GetCurrentThreadIDHook, "IATHookDLL.dll");
    IATHook_Ex->Hook();
    pGetCurrentThreadID = IATHook_Ex->GetOriginal<tGetCurrentThreadId>();
    GetCurrentThreadId();
    IATHook_Ex->UnHook();
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 1;
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved) {
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, &InitializeHook, hModule, 0, NULL); 
            break;
        }
    return TRUE;
}