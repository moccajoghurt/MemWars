#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "../../MemWarsCore/MemWarsCore.h"

void startMemoryTestApp(void*p) {
    // system("memoryTestApp.exe");
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
}

int main(int argc, char* argv[]) {
    int value1 = 133337;
    // STARTUPINFO si;
	// PROCESS_INFORMATION pi;
	
	// ZeroMemory(&si, sizeof(si));
	// si.cb = sizeof(si);
	// ZeroMemory(&pi, sizeof(pi));
	// CreateProcess(
    //     TEXT("memoryTestApp.exe"),
    //     NULL,NULL,NULL,FALSE,
    //     0,
    //     NULL,NULL,
    //     &si,
    //     &pi
    // );
    // system("memoryTestApp.exe");
    // HANDLE process = (HANDLE)GetProcessByNameEx(TEXT("memoryTestApp.exe"), TRUE, PROCESS_QUERY_INFORMATION);
    // while (process == NULL) {
    //     process = (HANDLE)GetProcessByNameEx(TEXT("memoryTestApp.exe"), TRUE, PROCESS_QUERY_INFORMATION);
    // }
    _beginthread(startMemoryTestApp, 0, 0);
    for (;;) {
        // if (process == NULL) {
            // CreateProcess(
            //     TEXT("memoryTestApp.exe"),
            //     NULL,NULL,NULL,FALSE,
            //     0,
            //     NULL,NULL,
            //     &si,
            //     &pi
            // );
        // }
    }
}