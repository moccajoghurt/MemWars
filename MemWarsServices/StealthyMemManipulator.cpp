
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <Winternl.h>
#include <Psapi.h>
#include <Winnt.h>
#include "../MemWarsCore/MemWarsCore.h"
#include "StealthyMemManipulator.h"

using namespace std;

BOOL StealthyMemInstaller::Init() {
    //todo create random name at startup
    sharedMemName = 'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M';
    globalMutex = 'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M'+'M'+'t'+'x';
    return TRUE;
}

BOOL StealthyMemInstaller::Install() {
    
	if (InstanceAlreadyRunning()) {
        cout << "Install() Instance already running." << endl;
        return FALSE;
    }

    if (!SetPrivilege(SE_DEBUG_NAME)) {
        return FALSE;
    }
		
    return TRUE;
}

BOOL StealthyMemInstaller::InstanceAlreadyRunning() {
    hGlobalMutex = CreateMutexA(NULL, TRUE, globalMutex.c_str());
	if (hGlobalMutex == NULL) {
        // something weird went wrong, we return true so the installer knows it can't continue
        return TRUE;
        
    } else if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // An instance of either the installer or the client is already running, terminate now
        return TRUE; 

    } else {
        return FALSE;
    }
}

BOOL StealthyMemInstaller::SetPrivilege(LPCSTR lpszPrivilege, BOOL bEnablePrivilege) {
    TOKEN_PRIVILEGES priv = {0, 0, 0, 0};
    HANDLE hToken = NULL;
    LUID luid = {0, 0};
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        if (hToken) {
            CloseHandle(hToken);
        }
        return FALSE;
    }
    if (!LookupPrivilegeValueA(0, lpszPrivilege, &luid)) {
        if (hToken){
            CloseHandle(hToken);
        }
        return FALSE;
    }
    priv.PrivilegeCount = 1;
    priv.Privileges[0].Luid = luid;
    priv.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : SE_PRIVILEGE_REMOVED;
    if (!AdjustTokenPrivileges(hToken, FALSE, &priv, 0, 0, 0)) {
        if (hToken) {
            CloseHandle(hToken);
        }
        return FALSE;
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return TRUE;
}

BOOL StealthyMemInstaller::CreateSharedFileMapping() {
    hLocalSharedMem = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE | SEC_COMMIT | SEC_NOCACHE,
        0,
        sharedMemSize,
        sharedMemName.c_str()
    );
	if (!hLocalSharedMem) {
        return FALSE;
    }
		
	ptrLocalSharedMem = MapViewOfFile(hLocalSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, sharedMemSize);
	if (!ptrLocalSharedMem) {
        return FALSE;
    }
    return TRUE;
}