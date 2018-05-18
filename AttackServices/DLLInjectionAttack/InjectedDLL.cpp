#include <windows.h>


DWORD WINAPI StartWork(LPVOID lpParam) {
    MessageBoxA(NULL, "DLL Attached!\n", "MemWars Framework", MB_OK | MB_TOPMOST);
    return 1;
}


BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD  fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			CreateThread(NULL, 0, &StartWork, NULL, 0, NULL); 
			break;
	}
	return TRUE;
}