#include <windows.h>
#include <string> // for debugging
#include "InjectedDLLHelperFuncs.h"

void* endSceneAddress;

DWORD WINAPI StartWork(LPVOID lpParam) {
    endSceneAddress = LocateEndSceneAddress();
    void* trampoline = CreateTrampolineFunc();
    originalCodeBuf = HookWithJump(endSceneAddress, trampoline);
    MessageBoxA(NULL, to_string((uintptr_t)hookedDevice).c_str(), "MemWars Framework", MB_OK | MB_TOPMOST);
    return 1;
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved) {
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, &StartWork, NULL, 0, NULL); 
            break;
        }
    return TRUE;
}