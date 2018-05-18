#include <windows.h>

DWORD WINAPI StartWork(LPVOID lpParam) {
    MessageBoxA(NULL, "DLL Attached!\n", "MemWars Framework", MB_OK | MB_TOPMOST);
    return 1;
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, &StartWork, NULL, 0, NULL); 
            break;
        }
    return TRUE;
}