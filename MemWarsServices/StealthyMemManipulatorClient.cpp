
#include <string>
#include <algorithm>
#include <iostream> // for debugging, remove later
#include "StealthyMemManipulatorClient.h"
#include "StealthyMemManipulatorGetHandleId.h"

using namespace std;

BOOL StealthyMemClient::Init(wstring pivotProcessName) {
    
    // if (InstanceAlreadyRunning()) {
    //     return FALSE;
    // }

	if (!SetPivotProcess(pivotProcessName)) {
        return FALSE;
    }
    
    if (!ConnectToFileMapping()) {
        return FALSE;
    }
    
	return Reconnect();
}

BOOL StealthyMemClient::InstanceAlreadyRunning() {
    string e = "";
	string mutexNoStr = e+'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M'+'M'+'t'+'x';
	m_hMutex = CreateMutexA(NULL, TRUE, mutexNoStr.c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // exit(EXIT_FAILURE);
        return TRUE;
    }
    return FALSE;
}

BOOL StealthyMemClient::SetPivotProcess(wstring pivotProcessName) {
	vector<DWORD> pidsLsass = GetPIDs(pivotProcessName);
	if (pidsLsass.empty()) {
        return FALSE;
    }
	sort(pidsLsass.begin(), pidsLsass.end());
	m_pivotPID = pidsLsass[0];
	if (!m_pivotPID) {
        return FALSE;
    }
    return TRUE;
}

BOOL StealthyMemClient::ConnectToFileMapping() {
    string e = "";
	string sharedMemNameNoStr = e+'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M';
	hSharedMem = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, sharedMemNameNoStr.c_str());
	if (!hSharedMem) {
        return FALSE;
    }
    return TRUE;
}


vector<DWORD> StealthyMemClient::GetPIDs(wstring targetProcessName) {
	vector<DWORD> pids;
	if (targetProcessName == L"") {
        return pids;
    }
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(entry);
	if (!Process32FirstW(snap, &entry)) {
        return pids;
    }
	do {
		if (wstring(entry.szExeFile) == targetProcessName) {
			pids.emplace_back(entry.th32ProcessID);
		}
	} while (Process32NextW(snap, &entry));
	return pids;
}

BOOL StealthyMemClient::Reconnect() {
    if (!hSharedMem) {
        return FALSE;
    }
        
    // Remapping shared memory
    m_ptrLocalSharedMem = MapViewOfFile(hSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
    CloseHandle(hSharedMem);
    if (!m_ptrLocalSharedMem) {
        return FALSE;
    }
    
    // Restoring variables from backup in shared memory
    _SHARED_MEM_INFO cfgBackup;
    void* endOfUsableLocalSharedMem = (void*)((DWORD64)m_ptrLocalSharedMem + SHARED_MEM_SIZE - sizeof(_REMOTE_COMMAND_INFO));
    void* backupAddrInSharedMem = (void*)((DWORD64)endOfUsableLocalSharedMem - sizeof(_SHARED_MEM_INFO));
    CopyMemory(&cfgBackup, backupAddrInSharedMem, sizeof(cfgBackup));

    // consistency check
    if (!cfgBackup.ptrRemoteSharedMem || 
        !cfgBackup.sharedMemSize || 
        !cfgBackup.remoteExecMem || 
        !cfgBackup.remoteExecMemSize || 
        cfgBackup.sharedMemSize != SHARED_MEM_SIZE
    ) {
        return FALSE;
    }
    m_usableSharedMemSize = cfgBackup.sharedMemSize - sizeof(_SHARED_MEM_INFO);
    // m_usableSharedMemSize = cfgBackup.sharedMemSize - sizeof(_SHARED_MEM_INFO) - sizeof(_REMOTE_COMMAND_INFO);

    return TRUE;
}

NTSTATUS StealthyMemClient::ReadWriteVirtualMemory(void* lpBaseAddress, void* lpBuffer, SIZE_T nSize, SIZE_T* nBytesReadOrWritten, BOOL read) {
    if (!lpBuffer || !lpBaseAddress || !nSize || nSize >= m_usableSharedMemSize || !m_hHiJack) {
        return (NTSTATUS)0xFFFFFFFF;
    }

    // Preparing order structure
    _REMOTE_COMMAND_INFO rpmOrder;
    rpmOrder.hProcess = m_hHiJack;
    rpmOrder.lpBaseAddress = (DWORD64)lpBaseAddress;
    rpmOrder.nSize = nSize;
    rpmOrder.nBytesReadOrWritten = nBytesReadOrWritten;

    // SecureZeroMemory(m_ptrLocalSharedMem, m_usableSharedMemSize);
    // cout << "m_ptrLocalSharedMem: " << (*(int*)m_ptrLocalSharedMem) << endl;
    // For write operations, changing order and placing data to write in shared memory
    if (!read) {
        rpmOrder.order = 1;
        CopyMemory(m_ptrLocalSharedMem, lpBuffer, nSize);
    }

    // Pushing parameters
    void* controlLocalAddr = (void*)((DWORD64)m_ptrLocalSharedMem + SHARED_MEM_SIZE - sizeof(rpmOrder));
    CopyMemory(controlLocalAddr, &rpmOrder, sizeof(rpmOrder));

    // Triggering execution and waiting for completion with the configured synchronisation method
    BYTE exec = 0;
    CopyMemory(controlLocalAddr, &exec, sizeof(exec));
    SpinLockByte(controlLocalAddr, 1);
    // Moving from shared memory to lpBuffer and returning
    if (read) {
        CopyMemory(lpBuffer, m_ptrLocalSharedMem, nSize);
    }
    // cout << "m_ptrLocalSharedMem: " << (*(int*)m_ptrLocalSharedMem) << endl;


    return rpmOrder.status;
}

BOOL StealthyMemClient::SetTargetProcessHandle(wstring targetProcessName) {
    m_hHiJack = GetHandleToId(targetProcessName, m_pivotPID);
    
    if (!m_hHiJack) {
        return FALSE;
    }
    return TRUE;
}

NTSTATUS StealthyMemClient::ReadVirtualMemory(void* lpBaseAddress, void* lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead) {
    return ReadWriteVirtualMemory(lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead, TRUE);
}

NTSTATUS StealthyMemClient::WriteVirtualMemory(void* lpBaseAddress, void* lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten) {
    return ReadWriteVirtualMemory(lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten, FALSE);
}


BOOL StealthyMemClient::Disconnect() {
	if (m_ptrLocalSharedMem) {
        UnmapViewOfFile(m_ptrLocalSharedMem);
    }
	return TRUE;
}

StealthyMemClient::~StealthyMemClient() {
	Disconnect();
}