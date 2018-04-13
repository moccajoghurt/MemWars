
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <Winternl.h>
#include <Psapi.h>
#include <Winnt.h>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include "../MemWarsCore/MemWarsCore.h"
#include "MemWarsServices.h"
#include "StealthyMemManipulator.h"

using namespace std;

BOOL StealthyMemInstaller::Init(vector<wstring> preferedTIDsModules, wstring targetProcessName) {
    // hiding names
    sharedMemName = 'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M';
	globalMutex = 'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M'+'M'+'t'+'x';
	// explExeName = L'e'+L'x'+L'p'+L'l'+L'o'+L'r'+L'e'+L'r'+L'.'+L'e'+L'x'+L'e';
	explExeName = L"explorer.exe";
	this->targetProcessName = targetProcessName;
	this->preferedTIDsModules = preferedTIDsModules;
    return TRUE;
}

BOOL StealthyMemInstaller::Install() {

	if (InstanceAlreadyRunning()) {
        cout << "Install() failed: Instance already running." << endl;
        return FALSE;
    }

    if (!SetProcessPrivilege(SE_DEBUG_NAME, TRUE)) {
        cout << "Install() failed: Privilege failed." << endl;
        return FALSE;
    }
    
    vector<DWORD> pidsTargetProcess = GetPIDsOfProcess(targetProcessName);
	if (pidsTargetProcess.empty()) {
        cout << "Install() failed: no PIDs found." << endl;
        return FALSE;
    }
    // in case the target process has multiple PIDs, take the first one
    sort(pidsTargetProcess.begin(), pidsTargetProcess.end());
    targetProcessPID = pidsTargetProcess[0];
	if (!targetProcessPID) {
        cout << "Install() failed: Invalid PID." << endl;
        return FALSE;
    }

    // Check for already existing installations
	hSharedMemHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, sharedMemName.c_str());
	if (hSharedMemHandle) {
        cout << "Install() failed: already installed." << endl;
        return TRUE; // Already installed
    }

    hTargetProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
        FALSE,
        targetProcessPID
    );
	if (!hTargetProcess) {
        cout << "Install() failed: Cannot open process. " << GetLastError() << endl;
        return FALSE;
    }

    vector<UNUSED_EXECUTABLE_MEM> availableExecutableMem = FindExecutableMemory(hTargetProcess, TRUE);
    if (availableExecutableMem.empty() || 
        availableExecutableMem[0].start == nullptr || 
        availableExecutableMem[0].size == NULL ||
        availableExecutableMem[0].size <= PADDING_IN_EXECUTABLE_MEM
    ) {
        cout << "Install() failed: No avaiable mem." << endl;
        return FALSE;
    }
    remoteExecutableMem = (void*)((DWORD64)availableExecutableMem[0].start + PADDING_IN_EXECUTABLE_MEM);
    remoteExecutableMemSize = availableExecutableMem[0].size - PADDING_IN_EXECUTABLE_MEM;
	
	if (!FindUsableTID()) {
		cout << "Install() failed: No TID found." << endl;
		return FALSE;
	}
	
	hTargetThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, targetTID);
	if (!hTargetThread) {
		cout << "Install() failed: could not open thread." << endl;
		return FALSE;
	}

	// Creating shared memory
	if (!CreateSharedFileMapping()) {
		cout << "Install() failed: SharedFileMApping failed." << endl;
		return FALSE;
	}

	// if (!CreateExternalGatekeeperHandleToFileMapping()) {
	// 	cout << "Install() failed: Gatekeeperhandle failed." << endl;
	// 	return FALSE;
	// }


	
		
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
	usableSharedMemSize = sharedMemSize - sizeof(SHARED_MEM_INFO);
    return TRUE;
}

vector <UNUSED_EXECUTABLE_MEM> StealthyMemInstaller::FindExecutableMemory(const HANDLE hProcess, BOOL onlyInBase) {
	MEMORY_BASIC_INFORMATION memInfo;
	vector<MEMORY_BASIC_INFORMATION> memInfos;
	vector<MEMORY_BASIC_INFORMATION> execMemInfos;
	vector<UNUSED_EXECUTABLE_MEM> freeExecutableMems;
	void* baseAddr = nullptr;
 
	if (onlyInBase) {
        baseAddr = GetProcessBaseAddress(hProcess);
        if (baseAddr == NULL) {
            return freeExecutableMems;
        }
    }
 
	// Getting all MEMORY_BASIC_INFORMATION of the target process
	unsigned char* addr = NULL;
	for (addr = NULL; VirtualQueryEx(hProcess, addr, &memInfo, sizeof(memInfo)) == sizeof(memInfo); addr += memInfo.RegionSize)
		if (!onlyInBase || (onlyInBase && memInfo.AllocationBase == baseAddr)) {
            memInfos.push_back(memInfo);
        }
	if (memInfos.empty()) {
        return freeExecutableMems;
    }
 
	// Filtering only executable memory regions
	for (int i = 0; i < memInfos.size(); ++i) {
		DWORD prot = memInfos[i].Protect;
		if (prot == PAGE_EXECUTE || prot == PAGE_EXECUTE_READ || prot == PAGE_EXECUTE_READWRITE || prot == PAGE_EXECUTE_WRITECOPY)
			execMemInfos.push_back(memInfos[i]);
	}
	if (execMemInfos.empty()) {
        return freeExecutableMems;
    }
 
	// Duplicating memory locally for analysis, finding unused memory at the end of executable regions
	for (int i = 0; i < execMemInfos.size(); ++i) {
		void* localMemCopy = VirtualAlloc(NULL, execMemInfos[i].RegionSize, MEM_COMMIT, PAGE_READWRITE);
		if (localMemCopy == NULL) {
            continue;
        }
 
		// Copying executable memory content locally
		SIZE_T bytesRead = 0;
		if (!ReadProcessMemory(hProcess, execMemInfos[i].BaseAddress, localMemCopy, execMemInfos[i].RegionSize, &bytesRead)) {
			VirtualFree(localMemCopy, execMemInfos[i].RegionSize, MEM_RELEASE);
			continue;
		}
 
		// Analyzing unused executable memory size and location locally
		BYTE currentByte = 0;
		SIZE_T unusedSize = 0;
		DWORD64 analysingByteAddr = (DWORD64)localMemCopy + execMemInfos[i].RegionSize - 1;
		while (analysingByteAddr >= (DWORD64)localMemCopy) {
			CopyMemory(&currentByte, (void*)analysingByteAddr, sizeof(BYTE));
			if (currentByte != 0) {
                break;
            }
			unusedSize++;
			analysingByteAddr--;
		}
		if (unusedSize == 0) {
			VirtualFree(localMemCopy, execMemInfos[i].RegionSize, MEM_RELEASE);
			continue;
		}
 
		UNUSED_EXECUTABLE_MEM unusedExecutableMem;
		unusedExecutableMem.regionInfo = execMemInfos[i];
		unusedExecutableMem.size = unusedSize;
		unusedExecutableMem.start = (void*)((DWORD64)execMemInfos[i].BaseAddress + execMemInfos[i].RegionSize - unusedSize);
		freeExecutableMems.push_back(unusedExecutableMem);
 
        VirtualFree(localMemCopy, execMemInfos[i].RegionSize, MEM_RELEASE);
	}
 
	return freeExecutableMems;
}

BOOL StealthyMemInstaller::FindUsableTID() {
	map<DWORD, wstring> tidsStartModules = GetTIDsModuleStartAddr(targetProcessPID);
	wstring modName = L"";
	for (int i = 0; i < preferedTIDsModules.size(); ++i) {
		for (auto const& thisTid : tidsStartModules) {
			DWORD tid = thisTid.first;
			modName = thisTid.second;
			if (modName == preferedTIDsModules[i]) {
				targetTID = tid;
				break;
			}
		}
		if (targetTID) {
			break;
		}
			
	}
	if (!targetTID) {
		return FALSE; // Could not find any of the threads starting in one of the target modules
	}
	return TRUE;
}


BOOL StealthyMemInstaller::CreateExternalGatekeeperHandleToFileMapping() {
	vector<DWORD> explorerPIDs = GetPIDsOfProcess(explExeName);
	if (explorerPIDs.empty()) {
		cout << "here" << endl;
		return FALSE;
	}
		
	hGateKeeperProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, explorerPIDs[0]);
	if (!hGateKeeperProcess) {
		return FALSE;
	}
	HANDLE hGateKeeper = NULL;
	if (!DuplicateHandle(
		GetCurrentProcess(), 
		hLocalSharedMem, 
		hGateKeeperProcess, 
		&hGateKeeper, 
		NULL, 
		FALSE, 
		DUPLICATE_SAME_ACCESS)
	) {
		return FALSE;
	}
	CloseHandle(hGateKeeperProcess);
	return TRUE;
}

BOOL StealthyMemInstaller::ConnectFileMappingWithTargetThread() {
	// Getting function addresses
	FARPROC addrOpenFileMappingA = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "OpenFileMappingA");
	FARPROC addrMapViewOfFile = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "MapViewOfFile");
	FARPROC addrCloseHandle = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "CloseHandle");
	if (!addrOpenFileMappingA || !addrMapViewOfFile || !addrCloseHandle) {
		return FALSE;
	}
		
 
	// Get RW memory to assemble full shellcode from parts
	void* rwMemory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (rwMemory == nullptr) {
		return FALSE;
	}
		
	DWORD64 addrEndOfShellCode = (DWORD64)rwMemory;
 
	UCHAR x64OpenFileMappingA[] = {
		0x48, 0xc7, 0xc1, 0x1f, 0, 0x0f, 0,	// mov rcx, dwDesiredAccess			+0 (FILE_MAP_ALL_ACCESS = 0xf001f @ +3)
		0x48, 0x31, 0xd2,					// xor rdx, rdx						+7 (bInheritHandle = FALSE)
		0x49, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0,	// mov r8, &lpName					+10 (&lpName +12)
		0x4d, 0x31, 0xc9,					// xor r9, r9						+20
		0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, // mov rax, addrOpenFileMappingA	+23 (addrOpenFileMappingA +25)
		0x48, 0x83, 0xec, 0x20,				// sub rsp, 0x20					+33
		0xff, 0xd0,							// call rax							+37
		0x48, 0x83, 0xc4, 0x20,				// add rsp, 0x20					+39
		0x49, 0x89, 0xc7					// mov r15, rax						+43
	};
	*(DWORD64*)((PUCHAR)x64OpenFileMappingA + 25) = (DWORD64)(ULONG_PTR)addrOpenFileMappingA;
	CopyMemory((void*)addrEndOfShellCode, x64OpenFileMappingA, sizeof(x64OpenFileMappingA));
	addrEndOfShellCode += sizeof(x64OpenFileMappingA);
 
	UCHAR x64MapViewOfFile[] = {
		0x48, 0x89, 0xc1,					// mov rcx, rax						+0
		0x48, 0xc7, 0xc2, 0x1f, 0, 0x0f, 0,	// mov rdx, dwDesiredAccess			+3 (FILE_MAP_ALL_ACCESS = 0xf001f @ +6)
		0x4d, 0x31, 0xc0,					// xor r8, r8						+10
		0x4d, 0x31, 0xc9,					// xor r9, r9						+13
		0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, // mov rax, dwNumberOfBytesToMap	+16 (dwNumberOfBytesToMap +18)
		0x50,								// push rax							+26
		0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0,	// mov rax, addrMapViewOfFile		+27 (addrMapViewOfFile +29)
		0x48, 0x83, 0xec, 0x20,				// sub rsp, 0x20					+37
		0xff, 0xd0,							// call rax							+41
		0x48, 0x83, 0xc4, 0x28,				// add rsp, 0x28					+43
		0x49, 0x89, 0xc6,					// mov r14, rax						+47
		// Writing to shared memory the virtual address in pivot process
		0x4d, 0x89, 0x36					// mov [r14], r14					+50
	};
	*(SIZE_T*)((PUCHAR)x64MapViewOfFile + 18) = (SIZE_T)(ULONG_PTR)sharedMemSize;
	*(DWORD64*)((PUCHAR)x64MapViewOfFile + 29) = (DWORD64)(ULONG_PTR)addrMapViewOfFile;
	CopyMemory((void*)addrEndOfShellCode, x64MapViewOfFile, sizeof(x64MapViewOfFile));
	addrEndOfShellCode += sizeof(x64MapViewOfFile);
 
	UCHAR x64CloseHandle[] = {
		0x4C, 0x89, 0xF9,					// mov rcx, r15						+0
		0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0,	// mov rax, addrCloseHandle			+3 (addrCloseHandle +5)
		0x48, 0x83, 0xec, 0x20,				// sub rsp, 0x20					+13
		0xff, 0xd0,							// call rax							+17
		0x48, 0x83, 0xc4, 0x20				// add rsp, 0x20					+19
	};
	*(DWORD64*)((PUCHAR)x64CloseHandle + 5) = (DWORD64)(ULONG_PTR)addrCloseHandle;
	CopyMemory((void*)addrEndOfShellCode, x64CloseHandle, sizeof(x64CloseHandle));
	addrEndOfShellCode += sizeof(x64CloseHandle);
 
	UCHAR x64InfiniteLoop[] = { 0xEB, 0xFE }; // nop + jmp rel8 -2
	CopyMemory((void*)addrEndOfShellCode, x64InfiniteLoop, sizeof(x64InfiniteLoop));
	addrEndOfShellCode += sizeof(x64InfiniteLoop);
 
	UCHAR lpNameBuffer[30];
	SecureZeroMemory(lpNameBuffer, sizeof(lpNameBuffer));
	CopyMemory(lpNameBuffer, sharedMemName.c_str(), sharedMemName.size());
	CopyMemory((void*)addrEndOfShellCode, lpNameBuffer, sizeof(lpNameBuffer));
	addrEndOfShellCode += sizeof(lpNameBuffer);
 
	// Calculating full size of shellcode
	SIZE_T fullShellcodeSize = addrEndOfShellCode - (DWORD64)rwMemory;
 
	// Placing pointer to the buffer integrated with the shellcode containing the name
	DWORD64 lpNameInRemoteExecMemory = (DWORD64)remoteExecMem + fullShellcodeSize - sizeof(lpNameBuffer);
	CopyMemory((void*)((DWORD64)rwMemory + 12), &lpNameInRemoteExecMemory, sizeof(lpNameInRemoteExecMemory));
 
	bool pushShellcodeStatus = PushShellcode(rwMemory, fullShellcodeSize);
	VirtualFree(rwMemory, 0, MEM_RELEASE);
	if (!pushShellcodeStatus) {
		return FALSE;
	}
		
 
	if (!ExecWithThreadHiJacking(fullShellcodeSize - sizeof(lpNameBuffer), false)) {
		// The shellcode ends before since the end is just memory
		return FALSE;
	} 
 
	CopyMemory(&ptrRemoteSharedMem, ptrLocalSharedMem, sizeof(void*));
	if (ptrRemoteSharedMem == nullptr) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

BOOL StealthyMemInstaller::ExecShellcodeWithHijackedThread(SIZE_T shellcodeSize, bool thenRestore) {
	return TRUE;
}