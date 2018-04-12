#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include "MemWarsServices.h"

vector<DWORD> GetPIDsOfProcess(wstring targetProcessName) {
	vector<DWORD> pids;
	if (targetProcessName == L"") {
        return pids;
    }
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof entry;
	if (!Process32FirstW(snap, &entry))
		return pids;
	do {
		if (wstring(entry.szExeFile) == targetProcessName) {
			pids.emplace_back(entry.th32ProcessID);
		}
	} while (Process32NextW(snap, &entry));
	return pids;
}


