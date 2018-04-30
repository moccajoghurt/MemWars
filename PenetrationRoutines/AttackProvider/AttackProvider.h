#pragma once;
#include <windows.h>

class AttackProvider {
public:
    virtual BOOL ReadProcessMemory(HANDLE hProcess, void* address, void* readBuf, SIZE_T readSize, SIZE_T* bytesRead) {return TRUE;};
    virtual BOOL WriteProcessMemory(HANDLE hProcess, void* address, void* writeBuf, SIZE_T writeSize, SIZE_T* bytesWritten) {return TRUE;};
};