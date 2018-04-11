#include <windows.h>
#include <iostream>
#include <process.h>
#include "StealthyMemManipulator.h"

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

void SMMInstall_InstallTest() {
    StealthyMemInstaller smi;
    smi.Init();
    if (!smi.Install()) {
        cout << "SMMInstall_InstallTest() failed" << endl;
        return;
    }

    
        

    cout << "SMMInstall_InstallTest() success" << endl;
}

int main() {
    SMMInstall_CreateSharedFileMappingTest();
    // SMMInstall_InstanceAlreadyRunningTest();
    SMMInstall_InstallTest();
}