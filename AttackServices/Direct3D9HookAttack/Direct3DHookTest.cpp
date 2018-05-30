#include <windows.h>
#include <iostream>
#include "../../Core/MemWarsCore.h"
#include "Direct3DHook.h"

using namespace std;

void Direct3DHookTest() {
    system("start Direct3DTestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("Direct3DTestApp.exe");
    }
    // Sleep(15000);
    if (LoadDirect3DDll(hProcess, L"InjectedDLL.dll")) {
        cout << "LoadDllTest() success" << endl;
    } else {
        cout << "LoadDllTest() failed" << endl;
    }

    // Exit:
    // system("taskkill /IM Direct3DTestApp.exe /F >nul");
}


int main() {
    Direct3DHookTest();
}