#ifndef _MEM_WARS_SERVICES_H
#define _MEM_WARS_SERVICES_H

#include <vector>
#include <map>
#include <string>

#ifndef MAKEULONGLONG
#define MAKEULONGLONG(ldw, hdw) ((ULONGLONG(hdw) << 32) | ((ldw) & 0xFFFFFFFF))
#endif

#define ThreadQuerySetWin32StartAddress 9

using namespace std;

vector<DWORD> GetPIDsOfProcess(wstring targetProcessName);
map<wstring, DWORD64> GetModulesNamesAndBaseAddresses(DWORD pid);
vector<DWORD> GetTIDChronologically(DWORD pid);
map<DWORD, DWORD64> GetThreadsStartAddresses(vector<DWORD> tids);
map<DWORD, wstring> GetTIDsModuleStartAddr(DWORD tid);
HANDLE GetProcessHandleByName(wstring name, DWORD access = PROCESS_ALL_ACCESS, BOOL inherit = FALSE);
uint32_t FindProcess(const std::string& name);

#endif