#ifndef _MEM_WARS_SERVICES_H
#define _MEM_WARS_SERVICES_H

#include <vector>

using namespace std;

vector<DWORD> GetPIDsOfProcess(wstring targetProcessName);
BOOL SetProcessPrivilege(LPCSTR lpszPrivilege, BOOL bEnablePrivilege = TRUE);


#endif