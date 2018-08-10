#include <iostream>
#include <Shlwapi.h>
#include "DLLInjectionProvider.h"

bool DLLInjectionProvider::SetTargetProcessByName(const string _name) {
    this->processName = _name;
    wstring name(_name.begin(), _name.end());
    hProcess = GetProcessHandleByName(name);
    if (hProcess == NULL) {
        results += "[-] SetTargetProcessByName() failed. Could not get HANDLE to ";
        results += this->processName;
        results += ". System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        return FALSE;
    }
    return TRUE;
}

bool DLLInjectionProvider::SetTargetDLL(const string _dllPath) {
    wstring dllPath(_dllPath.begin(), _dllPath.end());
    if (!PathFileExistsW(dllPath.c_str())) {
        results += "[-] SetTargetDLL() failed. File not found. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        return FALSE;
    }
    this->dllPath = dllPath;
    return TRUE;
}

bool DLLInjectionProvider::InjectDLL() {
    if (dllPath == L"" || hProcess == NULL) {
        results += "[-] InjectDLL() failed. Could not inject DLL. DLL or process is invalid.\n";
        return FALSE;
    }
    int status = LoadDll(hProcess, dllPath.c_str());
    if (status != 0) {
        results += "[-] InjectDLL() failed. Could not inject DLL. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        if (status == 1) results += "[-] Could not run VirtualAllocEx() on target process.\n";
        if (status == 2) results += "[-] Could not run WriteProcessMemory() on target process.\n";
        if (status == 3) results += "[-] Could not retrieve module handle of kernel32.dll.\n";
        if (status == 4) results += "[-] Could not retrieve process address of LoadLibraryW.\n";
        if (status == 5) results += "[-] Could not run CreateRemoteThread() on target process.\n";
        if (status == 6) results += "[-] Wait for remote thread timed out.\n";
        results += "[+] This indicates that ";
        results += this->processName;
        results += " is protected from DLL Injections.\n";
        return FALSE;
    }
    Sleep(500); // give the DLL time for the file creation
    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    lstrcatA(tempPath, "dllInjectionConfirmationFile");
    if (!PathFileExists(tempPath)) {
        results += "[-] InjectDLL() was successful but the confirmation file could not be found.\n";
        results += "[-] This indicates that " ;
        results += this->processName;
        results += " is vulnerable to DLL injections but the injection failed for other reasons.\n";
        results += "[-] Possible reasons: \n[-] - Process and DLL differ in bit architecture (x86/x64).\n";
        results += "[-] - The DLL could not be found by the target process because the DLL path was not absolute.\n";
        results += "[-] - The DLL has already been injected to the target process.\n";
        results += "[-] - The DLL contains runtime errors.\n";
        return FALSE;
    }
    results += "[+] InjectDLL() was successful.\n[+] ";
    results += this->processName;
    results += " is vulnerable to DLL injections.\n";
    if (!DeleteFile(tempPath)) {
        results += "[-] Warning: confirmation file could not be deleted! Make sure to delete it before running further tests. The containing folder is:\n";
        results += tempPath;
        results += "\n";
    }
    return TRUE;
}



// int main() {

//     DLLInjectionProvider a;
//     a.SetTargetProcessByName(L"explorer.exe");
//     a.SetTargetDLL(L"InjectedDLL.dll");
//     a.ExecuteAttack();

//     cout << a.GetAttackResults() << endl;

// }