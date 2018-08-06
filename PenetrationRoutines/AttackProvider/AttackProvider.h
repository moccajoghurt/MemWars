#pragma once;
#include <windows.h>
#include <string>

using namespace std;

class AttackProvider {
public:
    // virtual BOOL ReadProcessMemory(HANDLE hProcess, void* address, void* readBuf, SIZE_T readSize, SIZE_T* bytesRead) {return TRUE;};
    // virtual BOOL WriteProcessMemory(HANDLE hProcess, void* address, void* writeBuf, SIZE_T writeSize, SIZE_T* bytesWritten) {return TRUE;};
    virtual BOOL SetTargetProcessByName(wstring){return TRUE;};
    virtual BOOL ExecuteAttack(){return TRUE;};
    virtual string GetAttackResults() {
        return results;
    };
protected:
    string results = "";
};