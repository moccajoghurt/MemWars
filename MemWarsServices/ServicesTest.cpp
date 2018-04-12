#include <windows.h>
#include <iostream>
#include <process.h>
#include "../MemWarsCore/MemWarsCore.h"
#include "StealthyMemManipulator.h"
#include "MemWarsServices.h"

using namespace std;


void SMMInstall_CreateSharedFileMappingTest() {
    StealthyMemInstaller smi;
    smi.Init();
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
    smi.Init();
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
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
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
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    StealthyMemInstaller smi;
    smi.Init(L"memoryTestApp.exe");
    // smi.Init(L"lsass.exe");
    vector<UNUSED_EXECUTABLE_MEM> availableExecutableMem = smi.FindExecutableMemory(process, TRUE);

    if (availableExecutableMem.empty()) {
        cout << "SMMInstall_FindUnusedExecutableMemoryTest() failed" << endl;
    } else {
        cout << "SMMInstall_FindUnusedExecutableMemoryTest() success" << endl;
    }
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void SMMInstall_GetModulesNamesAndBaseAddressesTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    StealthyMemInstaller smi;
    smi.Init(L"memoryTestApp.exe");
    // smi.Init(L"lsass.exe");
    vector<DWORD> pids = GetPIDsOfProcess(L"memoryTestApp.exe");
    if (pids.empty()) {
        cout << "SMMInstall_GetModulesNamesAndBaseAddressesTest() failed. PID not found" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        return;
    }
    map<wstring, DWORD64> modsStartAddrs = smi.GetModulesNamesAndBaseAddresses(pids[0]);

    if (modsStartAddrs.empty()) {
        cout << "SMMInstall_GetModulesNamesAndBaseAddressesTest() failed" << endl;
    } else {
        cout << "SMMInstall_GetModulesNamesAndBaseAddressesTest() success" << endl;
    }
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void SMMInstall_InstallTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    StealthyMemInstaller smi;
    // smi.Init(L"lsass.exe"); // need to run as admin
    smi.Init(L"memoryTestApp.exe");
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
    // SMMInstall_CreateSharedFileMappingTest();
    // SMMInstall_InstanceAlreadyRunningTest();
    // SMMInstall_FindUnusedExecutableMemoryTest();
    SMMInstall_GetModulesNamesAndBaseAddressesTest();

    // todo: add test for GetThreadsStartAddresses, GetTIDChronologically and GetThreadsStartAddresses
    
    
    // SMMInstall_InstallTest();
}