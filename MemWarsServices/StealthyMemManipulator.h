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

using namespace std;

struct SHARED_MEM_INFO {
	SIZE_T remoteExecMemSize = NULL;
	void* remoteExecMem = nullptr;
	SIZE_T sharedMemSize = NULL;
	void* ptrRemoteSharedMem = nullptr;
};

class StealthyMemInstaller {
public:
    BOOL Init();
    BOOL Install();
    BOOL CreateSharedFileMapping();
    BOOL InstanceAlreadyRunning();
    BOOL SetPrivilege(LPCSTR lpszPrivilege, BOOL bEnablePrivilege = TRUE);


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
    string globalMutex;
    HANDLE hGlobalMutex = NULL;
};

#endif