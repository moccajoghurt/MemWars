
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

BOOL StealthyMemInstaller::Init(wstring targetProcessName) {
    //todo create random name at startup
    sharedMemName = 'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M';
    globalMutex = 'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M'+'M'+'t'+'x';
    this->targetProcessName = targetProcessName;
    return TRUE;
}

BOOL StealthyMemInstaller::Install() {

	if (InstanceAlreadyRunning()) {
        cout << "Install() failed: Instance already running." << endl;
        return FALSE;
    }

    if (!SetProcessPrivilege(SE_DEBUG_NAME)) {
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
    
    map<DWORD, wstring> tidsStartModules = GetTIDsModuleStartAddr(targetProcessPID);

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

map<wstring, DWORD64> StealthyMemInstaller::GetModulesNamesAndBaseAddresses(DWORD pid) {
	map<wstring, DWORD64> modsStartAddrs;
 
	if (!pid) {
        return modsStartAddrs;
    }
 
	HMODULE hMods[1024];
	DWORD cbNeeded;
	unsigned int i;
 
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!hProcess) {
        return modsStartAddrs;
    }
 
	// Get a list of all the modules in this process
	if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		CloseHandle(hProcess);
		return modsStartAddrs;
	}
 
	// Get each module's infos
	for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
		WCHAR szModName[MAX_PATH];
		if (!GetModuleFileNameExW(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
            // Get the full path to the module's file
			continue;
        }
		wstring modName = szModName;
		int pos = modName.find_last_of(L"\\");
		modName = modName.substr(pos + 1, modName.length());
 
		MODULEINFO modInfo;
		if (!GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo))) {
            continue;
        }
 
		DWORD64 baseAddr = (DWORD64)modInfo.lpBaseOfDll;
        modsStartAddrs[modName] = baseAddr;
	}
 
	// Release the handle to the process
	CloseHandle(hProcess);
	return modsStartAddrs;
}

vector<DWORD> StealthyMemInstaller::GetTIDChronologically(DWORD pid) {
	map<ULONGLONG, DWORD> tidsWithStartTimes;
	vector<DWORD> tids;
 
	if (pid == NULL) {
        return tids;
    }
 
	DWORD dwMainThreadID = NULL;
	ULONGLONG ullMinCreateTime = MAXULONGLONG;
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap != INVALID_HANDLE_VALUE) {
		THREADENTRY32 th32;
		th32.dwSize = sizeof(THREADENTRY32);
		BOOL bOK = TRUE;
		for (bOK = Thread32First(hThreadSnap, &th32); bOK; bOK = Thread32Next(hThreadSnap, &th32)) {
			if (th32.th32OwnerProcessID == pid) {
				HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, th32.th32ThreadID);
				if (hThread) {
					FILETIME afTimes[4] = { 0 };
					if (GetThreadTimes(hThread, &afTimes[0], &afTimes[1], &afTimes[2], &afTimes[3])) {
						ULONGLONG ullTest = MAKEULONGLONG(afTimes[0].dwLowDateTime, afTimes[0].dwHighDateTime);
						tidsWithStartTimes[ullTest] = th32.th32ThreadID;
					}
					CloseHandle(hThread);
				}
			}
		}
		CloseHandle(hThreadSnap);
	}
 
	for (auto const& thread : tidsWithStartTimes) {
        // maps are natively ordered by key
		tids.push_back(thread.second);
    } 
 
	return tids;
}

map<DWORD, DWORD64> GetThreadsStartAddresses(vector<DWORD> tids) {
	map<DWORD, DWORD64> tidsStartAddresses;
 
	if (tids.empty()) {
        return tidsStartAddresses;
    }

	for (int i = 0; i < tids.size(); ++i) {
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tids[i]);
		PVOID startAddress = NULL;
		ULONG returnLength = NULL;
		NTSTATUS NtQIT = NtQueryInformationThread(hThread, (THREADINFOCLASS)ThreadQuerySetWin32StartAddress, &startAddress, sizeof(startAddress), &returnLength);
		CloseHandle(hThread);
		if (tids[i] && startAddress) {
            tidsStartAddresses[tids[i]] = (DWORD64)startAddress;
        }
	}
 
	return tidsStartAddresses;
}

map<DWORD, wstring> StealthyMemInstaller::GetTIDsModuleStartAddr(DWORD pid) {
	map<DWORD, wstring> tidsStartModule;
 
	map<wstring, DWORD64> modsStartAddrs = GetModulesNamesAndBaseAddresses(pid);
	if (modsStartAddrs.empty()) {
        return tidsStartModule;
    }
 
	vector<DWORD> tids = GetTIDChronologically(pid);
	if (tids.empty()) {
        return tidsStartModule;
    }
		
 
	map<DWORD, DWORD64> tidsStartAddresses = GetThreadsStartAddresses(tids);
	if (tidsStartAddresses.empty()) {
        return tidsStartModule;
    }
 
	// for (auto const& thisTid : tidsStartAddresses) {
	// 	DWORD tid = thisTid.first;
	// 	DWORD64 startAddress = thisTid.second;
	// 	DWORD64 nearestModuleAtLowerAddrBase = 0;
	// 	wstring nearestModuleAtLowerAddrName = L"";
	// 	for (auto const& thisModule : modsStartAddrs) {
	// 		wstring moduleName = thisModule.first;
	// 		DWORD64 moduleBase = thisModule.second;
	// 		if (moduleBase > startAddress)
	// 			continue;
	// 		if (moduleBase > nearestModuleAtLowerAddrBase) {
	// 			nearestModuleAtLowerAddrBase = moduleBase;
	// 			nearestModuleAtLowerAddrName = moduleName;
	// 		}
	// 	}
	// 	if (nearestModuleAtLowerAddrBase > 0 && nearestModuleAtLowerAddrName != L"")
	// 		tidsStartModule[tid] = nearestModuleAtLowerAddrName;
	// }
 
	return tidsStartModule;
}

