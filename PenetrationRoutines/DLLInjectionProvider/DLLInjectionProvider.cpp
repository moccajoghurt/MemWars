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
    DeleteConfirmationFile(); // make sure there is no old confirmation file
    if (this->dllPath == L"" || this->hProcess == NULL) {
        results += "[-] InjectDLL() failed. Could not inject DLL. DLL or process is invalid.\n";
        return FALSE;
    }
    DWORD status;
    if (this->timeout <= 0) {
        if (!requireConfirmationFile) {
            status = LoadDll(hProcess, dllPath.c_str());
        } else {
            status = LoadDllNoShellcode(hProcess, dllPath.c_str());
        }
    } else {

        INJECTION_DATA data;
        data.dllPath = this->dllPath;
        data.hProcess = this->hProcess;
        data.useShellcode = requireConfirmationFile ? FALSE : TRUE;
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartThreadedInjection, &data, 0, NULL);
        DWORD ret = WaitForSingleObject(hThread, this->timeout);

        if (ret == WAIT_TIMEOUT) {
            TerminateThread(hThread, 0);
            results += "[-] InjectDLL() failed. Injection timed out.\n";
            DeleteConfirmationFile();
            return FALSE;
        } else {
            GetExitCodeThread(hThread, &status);
        }
    }
    
    if (status != 0 && status != 10) {
        results += "[-] InjectDLL() failed. Could not inject DLL. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        if (status == 1) results += "[-] Could not run VirtualAllocEx() on target process.\n";
        if (status == 2) results += "[-] Could not run WriteProcessMemory() on target process.\n";
        if (status == 3) results += "[-] Could not retrieve module handle of kernel32.dll.\n";
        if (status == 4) results += "[-] Could not retrieve process address of LoadLibraryW or ExitThread.\n";
        if (status == 5) results += "[-] Could not run CreateRemoteThread() on target process.\n";
        if (status == 6) results += "[-] WaitForSingleObject() returned WAIT_FAILED.\n";
        if (status == 8) results += "[-] GetExitCodeThread() failed. Could not retrieve status of remote thread.\n";
        results += "[+] This indicates that ";
        results += this->processName;
        results += " is protected from DLL Injections.\n";
        return FALSE;
    }
    Sleep(1000); // give the DLL time for the file creation
    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    lstrcatA(tempPath, "dllInjectionConfirmationFile");
    
    if (status == 0) {
        results += "[+] InjectDLL() was successful.\n[+] ";
        results += this->processName;
        results += " is vulnerable to DLL injections (confirmed by the return value of LoadLibrary).\n";

    } else if (status == 10 && !PathFileExists(tempPath)) {
        if (!requireConfirmationFile) {
            results += "[-] InjectDLL() failed. LoadLibrary returned NULL and confirmation file could not be found.\n";
            results += "[-] This indicates that ";
            results += this->processName;
            results += " is protected from DLL Injections.\n";

        } else {
            results += "[-] InjectDLL() failed. Confirmation file could not be found.\n";
        }
        
        return FALSE;

    } else {
        results += "[+] InjectDLL() was successful.\n[+] ";
        results += this->processName;
        results += " is vulnerable to DLL injections (confirmed by the confirmation file creation).\n";
    }
    
    if (PathFileExists(tempPath) && !DeleteFile(tempPath)) {
        results += "[-] Warning: confirmation file could not be deleted! Make sure to delete it before running further tests. The containing folder is:\n";
        results += tempPath;
        results += "\n";
    }
    return TRUE;
}


void DLLInjectionProvider::SetTimeout(int milliSeconds) {
    this->timeout = milliSeconds;
}

void DLLInjectionProvider::RequireConfirmationFile() {
    requireConfirmationFile = TRUE;
}

bool DLLInjectionProvider::AssertCompatible() {
    // would be nice to have (check if DLL and process share bit architecture)
    return TRUE;
}

DWORD StartThreadedInjection(LPVOID param) {
    INJECTION_DATA* data = (INJECTION_DATA*)param;
    if (data->useShellcode) {
        return LoadDll(data->hProcess, data->dllPath.c_str());
    } else {
        return LoadDllNoShellcode(data->hProcess, data->dllPath.c_str());
    }
}

bool DeleteConfirmationFile() {
    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    lstrcatA(tempPath, "dllInjectionConfirmationFile");
    return DeleteFile(tempPath);
}


// int main() {

//     DLLInjectionProvider a;
//     a.SetTargetProcessByName(L"explorer.exe");
//     a.SetTargetDLL(L"InjectedDLL.dll");
//     a.ExecuteAttack();

//     cout << a.GetAttackResults() << endl;

// }