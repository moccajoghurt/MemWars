#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "../MemWarsCore/MemWarsCore.h"

int main(int argc, char* argv[]) {
    int value1 = 133337;
    STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	CreateProcess(
        TEXT("memoryTestApp.exe"),
        NULL,NULL,NULL,FALSE,
        0,
        NULL,NULL,
        &si,
        &pi
    );
    Sleep(1000);
    HANDLE process;
    for (;;) {
        Sleep(1000);
        process = (HANDLE)GetProcessByNameEx(TEXT("memoryTestApp.exe"), FALSE, PROCESS_QUERY_INFORMATION);
        if (process == NULL) {
            CreateProcess(
                TEXT("memoryTestApp.exe"),
                NULL,NULL,NULL,FALSE,
                0,
                NULL,NULL,
                &si,
                &pi
            );
            Sleep(1000);
        }
    }
}