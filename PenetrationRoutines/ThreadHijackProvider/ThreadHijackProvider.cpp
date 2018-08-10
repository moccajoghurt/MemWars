#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <Shlwapi.h>
#include "ThreadHijackProvider.h"


bool ThreadHijackProvider::SetTargetProcessByName(const string _name) {
    wstring name(_name.begin(), _name.end());
    this->processNameW = name;
    this->processName = _name;
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

bool ThreadHijackProvider::HijackThread() {

    if (hProcess == NULL) {
        results += "[-] HijackThread() failed. Process HANDLE is NULL.\n";
        return FALSE;
    }

    vector<DWORD> pids = GetPIDsOfProcess(this->processNameW);
    if (pids.empty()) {
        results += "[-] HijackThread() failed. Could not get PIDs of ";
        results += this->processName;
        results += ". System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        return FALSE;
    }
    DWORD pid = pids[0];
    vector<DWORD> tids = GetTIDChronologically(pid);
    if (tids.empty()) {
        results += "[-] HijackThread() failed. Could not get TIDs of ";
        results += this->processName;
        results += ". System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        return FALSE;
    }

    DWORD tid = tids[0];
    
    int status = ThreadHijack(hProcess, tid);
    if (status != 0) {
        results += "[-] ThreadHijack() failed. Could not hijack thread. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        if (status == 1) results += "[-] Could not get process address of CreateFileA.\n";
        if (status == 2) results += "[-] Could call VirtualAllocEx() on target process.\n";
        if (status == 3) results += "[-] Could call OpenThread() on target process.\n";
        if (status == 4) results += "[-] Could not call SuspendThread() on target thread\n";
        if (status == 5) results += "[-] Could not call GetThreadContext() on target thread.\n";
        if (status == 6) results += "[-] Could not call SetThreadContext() on target thread.\n";
        if (status == 7) results += "[-] Could not call ResumeThread() on target thread.\n";
        results += "[+] This indicates that ";
        results += this->processName;
        results += " is protected from Thread hijacking.\n";
        return FALSE;
    }

    TCHAR processPath[MAX_PATH];
    GetModuleFileNameExA(hProcess, NULL, processPath, MAX_PATH);
    memset(processPath + strlen(processPath) - strlen(this->processName.c_str()), 0, MAX_PATH - strlen(processPath));
    lstrcatA(processPath, "hijackConfirmationFile");

    if (!PathFileExists(processPath)) {
        results += "[+] HijackThread() was successful but the confirmation file could not be found.\n";
        results += "[-] " ;
        results += this->processName;
        results += " is likely to run in a folder where it has no write permissions.\n";
        results += "[-] If " ;
        results += this->processName;
        results += " crashed it might be because injected shellcode (x64) is not compatible with the bit architecture.\n";
        results += "[+] " ;
        results += this->processName;
        results += " is vulnerable to thread hijacking.\n";
        return TRUE;
    }
    results += "[+] HijackThread() was successful.\n[+] ";
    results += this->processName;
    results += " is vulnerable to thread hijacking.\n";
    if (!DeleteFile(processPath)) {
        results += "[-] Warning: confirmation file could not be deleted! Make sure to delete it before running further tests. The containing folder is:\n";
        results += processPath;
        results += "\n";
    }
    return TRUE;
}



// int main() {

//     ThreadHijackProvider a;
//     a.SetTargetProcessByName("memoryTestApp.exe");
//     a.HijackThread();

//     cout << a.GetAttackResults() << endl;
// }