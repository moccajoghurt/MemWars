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
    // cout << availableExecutableMem.size() << endl;
    // cout << availableExecutableMem[0].size << endl;

    system("taskkill /IM memoryTestApp.exe /F >nul");
}

int main() {
    // GetPIDsOfProcessTest();
    // SMMInstall_CreateSharedFileMappingTest();
    // SMMInstall_InstanceAlreadyRunningTest();
    // SMMInstall_FindUnusedExecutableMemoryTest();
    
    
    SMMInstall_InstallTest();
}