#include "LsassInstallProvider.h"
#include <iostream>

using namespace std;

void SPIInstallProvider::Init() {
    wstring we = L"";
    samsrvDll = we+L's'+L'a'+L'm'+L's'+L'r'+L'v'+L'.'+L'd'+L'l'+L'l';
    msvcrtDll = we+L'm'+L's'+L'v'+L'c'+L'r'+L't'+L'.'+L'd'+L'l'+L'l';
    crypt32Dll = we+L'c'+L'r'+L'y'+L'p'+L't'+L'3'+L'2'+L'.'+L'd'+L'l'+L'l';
    lsassExe = we+L'l'+L's'+L'a'+L's'+L's'+L'.'+L'e'+L'x'+L'e';
}

bool SPIInstallProvider::Install() {
    this->Init();

    StealthyMemInstaller smi;
    vector<wstring> preferedThreadModuleNames;
    preferedThreadModuleNames.push_back(this->samsrvDll);
    preferedThreadModuleNames.push_back(this->msvcrtDll);
    preferedThreadModuleNames.push_back(this->crypt32Dll);
    if (!smi.Init(preferedThreadModuleNames, this->lsassExe)) {
        return FALSE;
    }

    int status = smi.Install();
    if (status != 0) {
        results += "[-] Install() failed. System Error Code: ";
        results += to_string(GetLastError());
        results += "\n";
        if (status == 1) results += "[-] An installing routine is already running.\n";
        if (status == 2) results += "[-] Could not set process privilege to SE_DEBUG_NAME.\n";
        if (status == 3) results += "[-] Could not retrieve target process PID.\n";
        if (status == 4) results += "[-] Could not retrieve target process HANDLE.\n";
        if (status == 5) results += "[-] Could not find usable executable memory in target process.\n";
        if (status == 6) results += "[-] Could not get a usable TID.\n";
        if (status == 7) results += "[-] Could not create a shareable file mapping.\n";
        if (status == 8) results += "[-] Gatekeeper process (explorer.exe) could not get a HANDLE to the file mapping.\n";
        if (status == 9) results += "[-] Could not inject file mapping shellcode into lsass.exe.\n";
        if (status == 10) results += "[-] Could not inject communication shellcode into lsass.exe.\n";
        results += "[-] Hint: The process needs to be run as administrator.\n";
        return FALSE;
    }
    results += "[+] Successfully installed the lsass.exe attack\n";

    // TODO Assert no anti-cheat running

    return TRUE;
}

// int main() {
//     SPIInstallProvider inst;
//     inst.Install();
// }