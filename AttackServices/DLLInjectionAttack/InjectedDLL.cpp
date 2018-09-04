#include <windows.h>


DWORD WINAPI StartWork(LPVOID lpParam) {
    // MessageBoxA(NULL, "DLL Attached!\n", "MemWars Framework", MB_OK | MB_TOPMOST);
	TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    lstrcatA(tempPath, "dllInjectionConfirmationFile");
    HANDLE h = CreateFileA(tempPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(h);
	FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 1;
}


BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD  fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			CreateThread(NULL, 0, &StartWork, hinstDLL, 0, NULL);
			break;
		case DLL_THREAD_ATTACH:
        	break;
	}
	return TRUE;
}