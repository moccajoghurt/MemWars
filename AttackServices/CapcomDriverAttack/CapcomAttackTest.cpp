#include <windows.h>
#include <iostream>
#include <vector>
#include "OpenProcessAttack.h"
#include "../../Core/MemWarsCore.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;

void CapcomAttackTest() {

    system("start /B memoryTestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }

    vector<DWORD> pids = GetPIDsOfProcess(L"memoryTestApp.exe");
    if (pids.empty()) {
        cout << "CapcomAttackTest() failed. Pids not found" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        return;
    }
    DWORD pid = pids[0];

    if (!StartAttack((HANDLE)pid, PROCESS_ALL_ACCESS)) {
        cout << "CapcomAttackTest() failed" << endl;
        return;
    } else {
        cout << "CapcomAttackTest() success" << endl;
    }
    
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

int main() {
    CapcomAttackTest();
}