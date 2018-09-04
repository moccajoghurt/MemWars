#include <iostream>
#include "../../Core/MemWarsCore.h"
#include "Injector.h"

using namespace std;

void LoadDllTest() {
    system("start /B memoryTestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    int ret = LoadDll(hProcess, L"InjectedDLLa.dll");
    if (ret == 0) {
        cout << "LoadDllTest() success" << endl;
    } else {
        cout << "LoadDllTest() failed " << ret << endl;
    }

    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

int main() {
    LoadDllTest();
}