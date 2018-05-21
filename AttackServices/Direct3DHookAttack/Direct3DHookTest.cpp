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

    if (LoadDirect3DDll(hProcess, L"InjectedDLL.dll")) {
        cout << "LoadDllTest() success" << endl;
    } else {
        cout << "LoadDllTest() failed" << endl;
    }

    Sleep(5000);

    Exit:
    system("taskkill /IM Direct3DTestApp.exe /F >nul");
}


int main() {
    Direct3DHookTest();
}