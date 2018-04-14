#include <windows.h>
#include <iostream>
#include <process.h>
#include "../MemWarsCore/MemWarsCore.h"
#include "StealthyMemManipulator.h"
#include "MemWarsServices.h"

using namespace std;


void SMMInstall_CreateSharedFileMappingTest() {
    StealthyMemInstaller smi;
    vector<wstring> dummy;
    smi.Init(dummy);
    BOOL result = smi.CreateSharedFileMapping();

    if (!result) {
        cout << "SMMInstall_CreateSharedFileMappingTest() failed" << endl;
    }

    result = UnmapViewOfFile(smi.getPtrLocalSharedMem());
    if (!result) {
        cout << "SMMInstall_CreateSharedFileMappingTest() failed" << endl;
    } else {
        cout << "SMMInstall_CreateSharedFileMappingTest() success" << endl;
    }
}


void SMMInstall_InstanceAlreadyRunningTest() {

    StealthyMemInstaller smi;
    vector<wstring> dummy;
    smi.Init(dummy);
    if (smi.InstanceAlreadyRunning()) {
        cout << "SMMInstall_InstanceAlreadyRunningTest() failed. " 
        << "Instance is running even though we didn't create it." << endl;
        CloseHandle(smi.getHGlobalMutex());
        return;
    }

    if (!smi.InstanceAlreadyRunning()) {
        cout << "SMMInstall_InstanceAlreadyRunningTest() failed. " 
        << "Instance is not running even though we just created it." << endl;
        return;
    }

    if (!CloseHandle(smi.getHGlobalMutex())) {
        cout << "SMMInstall_InstanceAlreadyRunningTest() failed. " 
        << "Could not close Mutex HANDLE" << endl;
        return;
    }
    
    cout << "SMMInstall_InstanceAlreadyRunningTest() success" << endl;
}

void GetPIDsOfProcessTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    vector<DWORD> pids = GetPIDsOfProcess(L"memoryTestApp.exe");
    if (pids.empty()) {
        cout << "GetPIDsOfProcessTest() failed" << endl;
    } else {
        cout << "GetPIDsOfProcessTest() success" << endl;
    }
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void SMMInstall_FindUnusedExecutableMemoryTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    StealthyMemInstaller smi;
    vector<wstring> dummy;
    smi.Init(dummy);
    // smi.Init(L"lsass.exe");
    vector<UNUSED_EXECUTABLE_MEM> availableExecutableMem = smi.FindExecutableMemory(process, TRUE);

    if (availableExecutableMem.empty()) {
        cout << "SMMInstall_FindUnusedExecutableMemoryTest() failed" << endl;
    } else {
        cout << "SMMInstall_FindUnusedExecutableMemoryTest() success" << endl;
    }
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void GetModulesNamesAndBaseAddressesTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    vector<DWORD> pids = GetPIDsOfProcess(L"memoryTestApp.exe");
    if (pids.empty()) {
        cout << "GetModulesNamesAndBaseAddressesTest() failed. PID not found" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        return;
    }
    map<wstring, DWORD64> modsStartAddrs = GetModulesNamesAndBaseAddresses(pids[0]);

    if (modsStartAddrs.empty()) {
        cout << "GetModulesNamesAndBaseAddressesTest() failed" << endl;
    } else {
        cout << "GetModulesNamesAndBaseAddressesTest() success" << endl;
    }
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void GetTIDChronologicallyTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    vector<DWORD> pids = GetPIDsOfProcess(L"memoryTestApp.exe");
    if (pids.empty()) {
        cout << "GetTIDChronologicallyTest() failed. PID not found" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        return;
    }
    vector<DWORD> tids = GetTIDChronologically(pids[0]);

    if (tids.empty()) {
        cout << "GetTIDChronologicallyTest() failed" << endl;
    } else {
        cout << "GetTIDChronologicallyTest() success" << endl;
    }
    system("taskkill /IM memoryTestApp.exe /F >nul");
}



void GetThreadsStartAddressesTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    vector<DWORD> pids = GetPIDsOfProcess(L"memoryTestApp.exe");
    if (pids.empty()) {
        cout << "GetThreadsStartAddressesTest() failed. PID not found" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        return;
    }
    vector<DWORD> tids = GetTIDChronologically(pids[0]);

    if (tids.empty()) {
        cout << "GetThreadsStartAddressesTest() failed. TID not found" << endl;
    }

    map<DWORD, DWORD64> threadStartAddresses = GetThreadsStartAddresses(tids);
    if (threadStartAddresses.empty()) {
        cout << "GetThreadsStartAddressesTest() failed" << endl;
    } else {
        cout << "GetThreadsStartAddressesTest() success" << endl;
    }

    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void GetTIDsModuleStartAddrTest() {
    
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    vector<DWORD> pids = GetPIDsOfProcess(L"memoryTestApp.exe");
    if (pids.empty()) {
        cout << "GetTIDsModuleStartAddrTest() failed. PID not found" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        return;
    }

    map<DWORD, wstring> tidStartAddresses = GetTIDsModuleStartAddr(pids[0]);

    if (tidStartAddresses.empty()) {
        cout << "GetTIDsModuleStartAddrTest() failed" << endl;
    } else {
        cout << "GetTIDsModuleStartAddrTest() success" << endl;
    }

    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void SMMInstall_ShellcodeHijackedThreadFileMappingTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    vector<DWORD> pids = GetPIDsOfProcess(L"memoryTestApp.exe");
    if (pids.empty()) {
        cout << "GetTIDsModuleStartAddrTest() failed. PID not found" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        return;
    }

    map<DWORD, wstring> tidStartAddresses = GetTIDsModuleStartAddr(pids[0]);

    if (tidStartAddresses.empty()) {
        cout << "GetTIDsModuleStartAddrTest() failed" << endl;
    } else {
        cout << "GetTIDsModuleStartAddrTest() success" << endl;
    }

    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void SMMInstall_InstallTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    StealthyMemInstaller smi;
    // smi.Init(L"lsass.exe"); // need to run as admin
    vector<wstring> preferedThreadModuleNames;
    // preferedThreadModuleNames.push_back(L"samsrv.dll");
    // preferedThreadModuleNames.push_back(L"msvcrt.dll");
    // preferedThreadModuleNames.push_back(L"crypt32.dll");
    // smi.Init(preferedThreadModuleNames, L"lsass.exe");
    preferedThreadModuleNames.push_back(L"memoryTestApp.exe");
    smi.Init(preferedThreadModuleNames, L"memoryTestApp.exe");
    if (!smi.Install()) {
        cout << "SMMInstall_InstallTest() failed" << endl;
        goto Exit;
    }
    
    cout << "SMMInstall_InstallTest() success" << endl;
    Exit:;
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

int main() {
    // GetPIDsOfProcessTest();
    // GetModulesNamesAndBaseAddressesTest();
    // GetTIDChronologicallyTest();
    // GetThreadsStartAddressesTest();
    // GetTIDsModuleStartAddrTest();
    // SMMInstall_CreateSharedFileMappingTest();
    // SMMInstall_InstanceAlreadyRunningTest();
    // SMMInstall_FindUnusedExecutableMemoryTest();
    // SMMInstall_ShellcodeHijackedThreadFileMappingTest();

    // todo: add test for GetThreadsStartAddresses and 
    
    
    SMMInstall_InstallTest();
}