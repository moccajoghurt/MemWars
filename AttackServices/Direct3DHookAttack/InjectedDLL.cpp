#include <windows.h>
#include <string> // for debugging
#include "InjectedDLLHelperFuncs.h"

DWORD WINAPI StartWork(LPVOID lpParam) {
    void* endScene = LocateEndSceneAddress();
    // MessageBoxA(NULL, to_string((int)endScene).c_str(), "MemWars Framework", MB_OK | MB_TOPMOST);
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