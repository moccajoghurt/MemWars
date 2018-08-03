#ifndef _MEM_WARS_SERVICES_H
#define _MEM_WARS_SERVICES_H

#include <Windows.h>
#include <inttypes.h>
// #include <winternl.h>
#include <vector>
#include <map>
#include <string>

using namespace std;

#ifndef MAKEULONGLONG
#define MAKEULONGLONG(ldw, hdw) ((ULONGLONG(hdw) << 32) | ((ldw) & 0xFFFFFFFF))
#endif

#define ThreadQuerySetWin32StartAddress 9

using fnFreeCall = uint64_t(__fastcall*)(...);

typedef struct _SYSTEM_MODULE_ENTRY {
	HANDLE section;
	PVOID mappedBase;
	PVOID imageBase;
	ULONG imageSize;
	ULONG flags;
	USHORT loadOrderIndex;
	USHORT initOrderIndex;
	USHORT loadCount;
	USHORT offsetToFileName;
	UCHAR fullPathName[256];
} SYSTEM_MODULE_ENTRY, *PSYSTEM_MODULE_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION {
	ULONG count;
	SYSTEM_MODULE_ENTRY module[0];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

#define SystemModuleInformation 0xBull

vector<DWORD> GetPIDsOfProcess(wstring targetProcessName);
map<wstring, DWORD64> GetModulesNamesAndBaseAddresses(DWORD pid);
vector<DWORD> GetTIDChronologically(DWORD pid);
map<DWORD, DWORD64> GetThreadsStartAddresses(vector<DWORD> tids);
map<DWORD, wstring> GetTIDsModuleStartAddr(DWORD tid);
HANDLE GetProcessHandleByName(wstring name, DWORD access = PROCESS_ALL_ACCESS, BOOL inherit = FALSE);
uint32_t FindProcess(const std::string& name);
BOOL InitKernelModuleInfo();

extern HMODULE ntLib;
extern uint64_t ntBase;
static bool kernelModuleInitialized = FALSE;
template<typename T = fnFreeCall>
T GetKernelProcAddress(const char* proc) {
	if (!kernelModuleInitialized) {
		InitKernelModuleInfo();
		kernelModuleInitialized = TRUE;
	}
	FARPROC locProc = GetProcAddress(ntLib, proc);

	if (!locProc) {
		return (T) (nullptr);
	}

	uint32_t delta = (uintptr_t) (locProc) - (uintptr_t) (ntLib);

	return (T) (ntBase + delta);
}

#endif