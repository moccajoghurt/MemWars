/*
* Functionality:
* Indirectly read / write memory of a target process. To do so we inject shellcode in another process
* that does the read / write operations and writes the results in a FileMapping. We then fetch the 
* results from the FileMapping.
* Installer:
* - create an inter-process communication system without creating new handles (stealthy). to do so we:
* - create a FileMapping that will be used for communication
* - find executable zeroed memory in a given process
* - inject shellcode into the process that enables the FileMapping communication
* - use a minimalistic IPC protocol inside the FileMapping
* - use a thread of the process to execute the shellcode
* Service:
* - use the communication system that has been setup for the read / write operations
*/

#ifndef _STEALTHY_MEM_MANIPULATOR_H
#define _STEALTHY_MEM_MANIPULATOR_H

#define SHARED_MEM_SIZE 4096
#define PADDING_IN_EXECUTABLE_MEM 8

#include <string>
#include <vector>
#include <map>

using namespace std;

struct SHARED_MEM_INFO {
	SIZE_T remoteExecMemSize = NULL;
	void* remoteExecMem = nullptr;
	SIZE_T sharedMemSize = NULL;
	void* ptrRemoteSharedMem = nullptr;
};

struct UNUSED_EXECUTABLE_MEM {
	MEMORY_BASIC_INFORMATION regionInfo;
	void* start = nullptr;
	SIZE_T size = NULL;
};

class StealthyMemInstaller {
public:
    BOOL Init(wstring targetProcessName = L"");
    BOOL Install();
    BOOL CreateSharedFileMapping();
    BOOL InstanceAlreadyRunning();
    vector <UNUSED_EXECUTABLE_MEM> FindExecutableMemory(const HANDLE, BOOL);
    map<wstring, DWORD64> GetModulesNamesAndBaseAddresses(DWORD);
    map<DWORD, wstring> GetTIDsModuleStartAddr(DWORD pid);

    // for testing
    void* getPtrLocalSharedMem() {
        return ptrLocalSharedMem;
    }
    HANDLE getHGlobalMutex() {
        return hGlobalMutex;
    }

private:
    SIZE_T sharedMemSize = SHARED_MEM_SIZE;
    HANDLE hLocalSharedMem = NULL;
    void* ptrLocalSharedMem = nullptr;
    string sharedMemName;
    HANDLE hSharedMemHandle;
    string globalMutex;
    HANDLE hGlobalMutex = NULL;
    wstring targetProcessName;
    DWORD targetProcessPID;
    HANDLE hTargetProcess;
    void* remoteExecutableMem = nullptr;
    SIZE_T remoteExecutableMemSize = 0;
};

#endif