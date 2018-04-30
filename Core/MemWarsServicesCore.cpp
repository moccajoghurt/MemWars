#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <Winternl.h>
#include <iostream>
#include "MemWarsServicesCore.h"

vector<DWORD> GetPIDsOfProcess(wstring targetProcessName) {
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

map<wstring, DWORD64> GetModulesNamesAndBaseAddresses(DWORD pid) {
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

vector<DWORD> GetTIDChronologically(DWORD pid) {
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

map<DWORD, wstring> GetTIDsModuleStartAddr(DWORD pid) {
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
 
	for (auto const& thisTid : tidsStartAddresses) {
		DWORD tid = thisTid.first;
		DWORD64 startAddress = thisTid.second;
		DWORD64 nearestModuleAtLowerAddrBase = 0;
		wstring nearestModuleAtLowerAddrName = L"";
		for (auto const& thisModule : modsStartAddrs) {
			wstring moduleName = thisModule.first;
			DWORD64 moduleBase = thisModule.second;
			if (moduleBase > startAddress) {
				continue;
			}
			if (moduleBase > nearestModuleAtLowerAddrBase) {
				nearestModuleAtLowerAddrBase = moduleBase;
				nearestModuleAtLowerAddrName = moduleName;
			}
		}
		if (nearestModuleAtLowerAddrBase > 0 && nearestModuleAtLowerAddrName != L"") {
			tidsStartModule[tid] = nearestModuleAtLowerAddrName;
		}
	}
 
	return tidsStartModule;
}


HANDLE GetProcessHandleByName(wstring name, DWORD access, BOOL inheritHandle) {
	vector<DWORD> pids = GetPIDsOfProcess(name);
	if (pids.empty()) {
		return NULL;
	}
	DWORD processID = pids[0];
    HANDLE hProc = OpenProcess(access, inheritHandle, processID);
    return hProc;
}