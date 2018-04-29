#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <climits>
#include "../SPIAttackProvider/Client/SPIAttackProvider.h"
#include "../AttackProvider/AttackProvider.h"

class ValueFinder {
public:
    BOOL Init(string attackMethod, wstring targetProcess, wstring pivotProcess = L"");
    vector<void*> FindValueUsingVirtualQuery(void* value, const SIZE_T size, HANDLE hProcess);

    ~ValueFinder() {
        delete attackProvider;
    }

protected:
    AttackProvider* attackProvider = NULL;
    UINT maxReadSize = INT_MAX;
    string attackMethod;
    wstring targetProcess;
    wstring pivotProcess;
};
