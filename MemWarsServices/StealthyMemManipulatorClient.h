/*
*
*
* The client only uses specific MemWars libraries since we dont want any strings in the binary
*/


#ifndef _STEALTHY_MEM_MANIPULATOR_CLIENT_H
#define _STEALTHY_MEM_MANIPULATOR_CLIENT_H

#define SHARED_MEM_SIZE 4096

#include <windows.h>
#include <vector>
#include <TlHelp32.h>

using namespace std;

extern "C" void SpinLockByte(volatile void* byteAddr, volatile BYTE valueExit);

struct _REMOTE_COMMAND_INFO {
	DWORD64 exec = 1; // Least significant byte used to release the spinlock
	DWORD order = 0; // 0: Read, 1: Write
	NTSTATUS status = 0xFFFFFFFF;
	HANDLE hProcess = NULL;
	DWORD64 lpBaseAddress = NULL;
	SIZE_T nSize = 0;
	SIZE_T* nBytesReadOrWritten = 0;
}; // Important: Must be 8 bytes aligned, otherwise garbage data is added in the structure

struct _SHARED_MEM_INFO {
	SIZE_T remoteExecMemSize = NULL;
	void* remoteExecMem = nullptr;
	SIZE_T sharedMemSize = NULL;
	void* ptrRemoteSharedMem = nullptr;
};

class StealthyMemClient {
public:
    // StealthyMemClient();
	~StealthyMemClient();
    BOOL Init(wstring pivotProcessName);
    BOOL InstanceAlreadyRunning();
    BOOL SetPivotProcess(wstring pivotProcessName);
    BOOL ConnectToFileMapping();
    vector<DWORD> GetPIDs(wstring targetProcessName);
    BOOL Reconnect();
    BOOL Disconnect();
    BOOL SetTargetProcessHandle(wstring targetProcessName);

    NTSTATUS ReadWriteVirtualMemory(void* lpBaseAddress, void* lpBuffer, SIZE_T nSize, SIZE_T* nBytesReadOrWritten, BOOL read);
    NTSTATUS WriteVirtualMemory(void* lpBaseAddress, void* lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);
    NTSTATUS ReadVirtualMemory(void* lpBaseAddress, void* lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead);

protected:
    HANDLE m_hMutex = NULL;
    DWORD m_pivotPID = NULL;
    void* m_ptrLocalSharedMem = nullptr;
    SIZE_T m_usableSharedMemSize = NULL;
    HANDLE m_hHiJack = NULL;
    HANDLE hSharedMem = NULL;
};


#endif