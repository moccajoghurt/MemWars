
#include <string>
#include <algorithm>
#include "StealthyMemManipulatorClient.h"

using namespace std;

BOOL StealthyMemClient::Init() {
    string e = "";
	string mutexNoStr = e+'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'J'+'2'+'M'+'t'+'x';
	m_hMutex = CreateMutexA(NULL, TRUE, mutexNoStr.c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // exit(EXIT_FAILURE);
        return FALSE;
    }
 
	wstring we = L"";
	wstring lsassNoStr = we + L'l' + L's' + L'a' + L's' + L's' + L'.' + L'e' + L'x' + L'e';
	vector<DWORD> pidsLsass = GetPIDs(lsassNoStr);
	if (pidsLsass.empty()) {
        return FALSE;
    }
	sort(pidsLsass.begin(), pidsLsass.end());
	m_pivotPID = pidsLsass[0];
	if (!m_pivotPID) {
        return FALSE;
    }
 
	// Test if bypass is installed with gatekeeper
	string sharedMemNameNoStr = e+'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M';
	HANDLE hLocalSharedMem = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, sharedMemNameNoStr.c_str());
	if (!hLocalSharedMem) {
        return FALSE;
    }
	return Reconnect(hLocalSharedMem);
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

BOOL StealthyMemClient::Reconnect(HANDLE hLocalSharedMem) {
    if (!hLocalSharedMem) {
        return FALSE;
    }
        
    // Remapping shared memory
    m_ptrLocalSharedMem = MapViewOfFile(hLocalSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
    CloseHandle(hLocalSharedMem);
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

    return TRUE;
}

NTSTATUS StealthyMemClient::ReadWriteVirtualMemory(HANDLE hProcess, void* lpBaseAddress, void* lpBuffer, SIZE_T nSize, SIZE_T* nBytesReadOrWritten, BOOL read) {
    if (!lpBuffer || !lpBaseAddress || !nSize || nSize >= m_usableSharedMemSize || !hProcess) {
        return (NTSTATUS)0xFFFFFFFF;
    }

    // Preparing order structure
    _REMOTE_COMMAND_INFO rpmOrder;
    rpmOrder.hProcess = hProcess;
    rpmOrder.lpBaseAddress = (DWORD64)lpBaseAddress;
    rpmOrder.nSize = nSize;
    rpmOrder.nBytesReadOrWritten = nBytesReadOrWritten;

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

    return rpmOrder.status;
}