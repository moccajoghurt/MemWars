#include <windows.h>
#include <iostream>
#include "../../Core/MemWarsCore.h"
#include "../../Core/MemWarsServicesCore.h"
#include "Injector.h"

using namespace std;

void HiddenKernelDLLInjectionAttack() {

    system("start /B memoryTestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }

    if (!StealthInject("explorer.exe", "InjectedDLL.dll")) {
        cout << "HiddenKernelDLLInjectionAttack() failed" << endl;
    } else {
        cout << "HiddenKernelDLLInjectionAttack() success" << endl;
    }
    
    
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

int main() {
    HiddenKernelDLLInjectionAttack();
}