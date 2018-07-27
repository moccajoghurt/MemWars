#include <windows.h>
#include <iostream>
#include "../../Core/MemWarsCore.h"
#include "../../Core/MemWarsServicesCore.h"
#include "Injector.h"

using namespace std;

void HiddenKernelDLLInjectionAttack() {

    system("start /B TestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("TestApp.exe");
    }

    if (!StealthInject("TestApp.exe", "InjectedDLL.dll")) {
        cout << "HiddenKernelDLLInjectionAttack() failed" << endl;
    } else {
        cout << "HiddenKernelDLLInjectionAttack() success" << endl;
    }
    
    
    system("taskkill /IM TestApp.exe /F >nul");
}

int main() {
    HiddenKernelDLLInjectionAttack();
}