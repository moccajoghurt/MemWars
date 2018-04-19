#include <windows.h>
#include <iostream>
#include <process.h>
#include "../MemWarsCore/MemWarsCore.h"
#include "MemWarsServices.h"
#include "StealthyMemManipulatorInstaller.h"
#include "StealthyMemManipulatorClient.h"

using namespace std;


void SMMInstall_CreateSharedFileMappingTest() {
    StealthyMemInstaller smi;
    vector<wstring> dummy;
    smi.Init(dummy);
    BOOL result = smi.CreateSharedFileMapping();

    if (!result) {
        cout << "SMMInstall_CreateSharedFileMappingTest() failed" << endl;
        return;
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
    StealthyMemInstaller smi;
    vector<wstring> preferedThreadModuleNames;
    preferedThreadModuleNames.push_back(L"memoryTestApp.exe");
    smi.Init(preferedThreadModuleNames, L"memoryTestApp.exe");

    if (!SetProcessPrivilege(SE_DEBUG_NAME, TRUE)) {
        cout << "SMMInstall_ShellcodeHijackedThreadFileMappingTest() failed: Privilege failed." << endl;
        goto Exit;
    }
	
	if (!smi.GetTargetProcessPID()) {
        cout << "SMMInstall_ShellcodeHijackedThreadFileMappingTest() failed: GetTargetProcessPID failed." << endl;
        goto Exit;
    }

    if (!smi.GetTargetProcessHandle()) {
		cout << "SMMInstall_ShellcodeHijackedThreadFileMappingTest() failed: GetTargetProcessHandle failed." << endl;
        goto Exit;
	}

    if (!smi.GetRemoteExecutableMemory()) {
		cout << "SMMInstall_ShellcodeHijackedThreadFileMappingTest() failed: GetRemoteExecutableMemory failed." << endl;
        goto Exit;
	}
	
	if (!smi.FindUsableTID()) {
		cout << "SMMInstall_ShellcodeHijackedThreadFileMappingTest() failed: FindUsableTID failed." << endl;
		goto Exit;
	}

	if (!smi.CreateSharedFileMapping()) {
		cout << "SMMInstall_ShellcodeHijackedThreadFileMappingTest() failed: CreateSharedFileMapping failed." << endl;
		goto Exit;
    }
    
    if (!smi.InjectFileMappingShellcodeIntoTargetThread()) {
        cout << "SMMInstall_ShellcodeHijackedThreadFileMappingTest() failed: InjectFileMappingShellcodeIntoTargetThread failed." << endl;
		goto Exit;
    }

    cout << "SMMInstall_ShellcodeHijackedThreadFileMappingTest() success" << endl;

    Exit:;
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void SMMInstall_InstallTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    StealthyMemInstaller smi;
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

void SMMClient_InitTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    StealthyMemInstaller smi;
    vector<wstring> preferedThreadModuleNames;
    preferedThreadModuleNames.push_back(L"memoryTestApp.exe");
    smi.Init(preferedThreadModuleNames, L"memoryTestApp.exe");
    if (!smi.Install()) {
        cout << "SMMClient_InitTest() failed. Installer failed" << endl;
        system("taskkill /IM memoryTestApp.exe /F >nul");
        return;
    }

    StealthyMemClient smc;
    if (!smc.Init(L"memoryTestApp.exe")) {
        cout << "SMMClient_InitTest() failed" << endl;
        goto Exit;
    }

    cout << "SMMClient_InitTest() success" << endl;

    Exit:;
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void SMMClient_ReadWriteMemoryWithPivotTest() {

    HANDLE process = (HANDLE)GetProcessByName("TestPivotApp.exe");
    if (process == NULL) {
        system("start /B TestPivotApp.exe");
    }
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("TestPivotApp.exe");
    }
    
    StealthyMemInstaller smi;
    vector<wstring> preferedThreadModuleNames;
    preferedThreadModuleNames.push_back(L"TestPivotApp.exe");
    smi.Init(preferedThreadModuleNames, L"TestPivotApp.exe");
    if (!smi.Install()) {
        cout << "SMMClient_ReadMemoryTest() failed. Installer failed" << endl;
        return;
    }

    MEMPTRS ptrBuf = {0};
    BYTEARRAY bArr = {0};
    BYTEARRAY bArr1 = {0};
    BYTEARRAY bArr2 = {0};
    SIZE_T bytesReadBuf = 0;

    StealthyMemClient smc;
    if (!smc.Init(L"TestPivotApp.exe")) {
        cout << "SMMClient_ReadMemoryTest() failed. Init failed" << endl;
        goto Exit;
    }

    IntToByteArray(&bArr, 133337);
    FindValueInProcess(&bArr, process, &ptrBuf);

    if (ptrBuf.size <= 0) {
        cout << "SMMClient_ReadMemoryTest() failed. Val in TestApp not found." << endl;
        goto Exit;
    }
    if (!smc.SetTargetProcessHandle(L"memoryTestApp.exe")) {
        // Reminder: lsass.exe does not have handles to all processes. Only to processes that do networking.
        cout << "SMMClient_ReadMemoryTest() failed. Could not get Handle" << endl;
        goto Exit;
    }
    cout << smc.GetTargetHandle() << endl;
    smc.ReadVirtualMemory(ptrBuf.memPointerArray[0], bArr1.values, sizeof(int), &bytesReadBuf);
    cout << *(int*)bArr.values << endl;
    cout << *(int*)bArr1.values << endl;
    bArr1.size = sizeof(int);
    if (!ValueIsMatching(&bArr, &bArr1)) {
        cout << "SMMClient_ReadMemoryTest() failed. ReadVirtualMemory failed" << endl;
    }
    IntToByteArray(&bArr2, 733331);
    smc.WriteVirtualMemory(ptrBuf.memPointerArray[0], bArr2.values, sizeof(int), &bytesReadBuf);
    bArr1 = {0};
    // ReadProcessMemoryAtPtrLocation(ptrBuf.memPointerArray[0], sizeof(int), process, &bArr1);
    smc.ReadVirtualMemory(ptrBuf.memPointerArray[0], bArr1.values, sizeof(int), &bytesReadBuf);
    bArr1.size = sizeof(int);
    cout << *(int*)bArr2.values << endl;
    cout << *(int*)bArr1.values << endl;

    if (ValueIsMatching(&bArr2, &bArr1)) {
        cout << "SMMClient_ReadMemoryTest() success" << endl;
    } else {
        cout << "SMMClient_ReadMemoryTest() failed" << endl;
    }

    Exit:;
    // system("taskkill /IM memoryTestApp.exe /F >nul");
}

void SMMClient_ReadWriteMemoryWithLsass() {

    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("SkypeApp.exe");
        if (process == NULL) {
            cout << "Open Skype to start testing..." << endl;
            Sleep(5000);
        }
    }
    
    StealthyMemInstaller smi;
    vector<wstring> preferedThreadModuleNames;
    preferedThreadModuleNames.push_back(L"samsrv.dll");
    preferedThreadModuleNames.push_back(L"msvcrt.dll");
    preferedThreadModuleNames.push_back(L"crypt32.dll");
    smi.Init(preferedThreadModuleNames, L"lsass.exe");
    if (!smi.Install()) {
        cout << "SMMClient_ReadMemoryTest() failed. Installer failed" << endl;
        return;
    }

    MEMPTRS ptrBuf = {0};
    BYTEARRAY bArr = {0};
    BYTEARRAY bArr1 = {0};
    SIZE_T bytesReadBuf = 0;

    StealthyMemClient smc;
    if (!smc.Init(L"lsass.exe")) {
        cout << "SMMClient_ReadMemoryTest() failed. Init failed" << endl;
        return;
    }

    IntToByteArray(&bArr, 133337);
    FindValueInProcess(&bArr, process, &ptrBuf);

    if (ptrBuf.size <= 0) {
        cout << "SMMClient_ReadMemoryTest() failed. Val in TestApp not found." << endl;
        return;
    }
    if (!smc.SetTargetProcessHandle(L"SkypeApp.exe")) {
        // Reminder: lsass.exe does not have handles to all processes. Only to processes that do networking.
        cout << "SMMClient_ReadMemoryTest() failed. Could not get Handle" << endl;
        return;
    }

    smc.ReadVirtualMemory(ptrBuf.memPointerArray[0], bArr1.values, sizeof(int), &bytesReadBuf);
    cout << *(int*)bArr1.values << endl;
    bArr1.size = sizeof(int);
    if (!ValueIsMatching(&bArr, &bArr1)) {
        cout << "SMMClient_ReadMemoryTest() failed. ReadVirtualMemory failed" << endl;
        return;
    }
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
    
    // cannot run InstallTest() in combination with other tests because of Mutexes
    // SMMInstall_InstallTest(); // need to restart explorer.exe after this test because of the handle to the file mapping in explorer.exe
    // SMMClient_InitTest(); // need to restart explorer.exe after this test because of the handle to the file mapping in explorer.exe
    SMMClient_ReadWriteMemoryWithPivotTest();
    // SMMClient_ReadWriteMemoryWithLsass();
    
}