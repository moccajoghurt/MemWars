#include <iostream>
#include <Shlwapi.h> // PathFileExists
#include "../../Core/MemWarsCore.h"
#include "../DLLInjectionAttack/Injector.h"

using namespace std;

void JmpHookTest() {
    system("start /B memoryTestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }

    if (!LoadDll(hProcess, L"InjectedDLL.dll")) {
        cout << "LoadDllTest() failed" << endl;
        goto Exit;
    }

    Sleep(100);

    if (!PathFileExists("jmpHookConfirmationFile")) {
        cout << "JmpHookTest() failed" << endl;
        goto Exit;
    } else {
        cout << "JmpHookTest() success" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        DeleteFile("jmpHookConfirmationFile");
        return;
    }

    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

int main() {
    JmpHookTest();
}