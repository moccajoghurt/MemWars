#include <windows.h>
#include <iostream>
#include <Shlwapi.h>
#include "../../Core/MemWarsCore.h"
#include "Direct3DHook.h"

using namespace std;

void Direct3DHookTest() {
    system("start Direct3DTestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("Direct3DTestApp.exe");
    }
    
    if (!LoadDirect3DDll(hProcess, L"InjectedDLL.dll")) {
        cout << "LoadDllTest() failed" << endl;
    }

    Sleep(100);
    
    if (!PathFileExists("direct3DConfirmationFile")) {
        cout << "Direct3DHookTest() failed" << endl;
        goto Exit;
    } else {
        cout << "Direct3DHookTest() success" << endl;
        system("taskkill /IM Direct3DTestApp.exe /F >nul");
        DeleteFile("direct3DConfirmationFile");
        return;
    }

    Exit:
    system("taskkill /IM Direct3DTestApp.exe /F >nul");
}


int main() {
    Direct3DHookTest();
}