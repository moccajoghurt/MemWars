#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "../SPIAttackProvider/Client/SPIAttackProvider.h"
#include "../AttackProvider/AttackProvider.h"

class ValueFinder {
public:
    BOOL Init(string attackMethod, wstring targetProcess, wstring pivotProcess = L"");
    vector<void*> FindValueUsingVirtualQuery(void* value, const SIZE_T size, HANDLE hProcess);
    void RemoveNotMatchingValues(vector<void*>& memPtrs, void* value, SIZE_T size, HANDLE hProcess);

    ~ValueFinder() {
        delete attackProvider;
    }

protected:
    AttackProvider* attackProvider = NULL;
    string attackMethod;
    wstring targetProcess;
    wstring pivotProcess;
};
