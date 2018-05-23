#include <iostream>
#include "../../Core/MemWarsCore.h"
#include "../DLLInjectionAttack/Injector.h"

using namespace std;

void JmpHookTest() {
    system("start /B memoryTestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }

    if (LoadDll(hProcess, L"InjectedDLL.dll")) {
        cout << "LoadDllTest() success" << endl;
    } else {
        cout << "LoadDllTest() failed" << endl;
    }

    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

int main() {
    JmpHookTest();
}