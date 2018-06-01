#include <windows.h>
#include <iostream>
#include <Shlwapi.h>
#include "../../Core/MemWarsCore.h"
#include "../DLLInjectionAttack/Injector.h"

using namespace std;

void IATHookTest() {
    system("start memoryTestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    
    if (!LoadDll(hProcess, L"InjectedDLL.dll")) {
        cout << "IATHookTest() failed" << endl;
    }

    Sleep(100);
    
    if (!PathFileExists("IATHookConfirmationFile")) {
        cout << "IATHookTest() failed" << endl;
        goto Exit;
    } else {
        cout << "IATHookTest() success" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        DeleteFile("IATHookConfirmationFile");
        return;
    }

    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}


int main() {
    IATHookTest();
}