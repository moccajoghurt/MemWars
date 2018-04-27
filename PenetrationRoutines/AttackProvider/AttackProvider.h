#pragma once;
#include <windows.h>

class AttackProvider {
public:
    virtual BOOL ReadProcessMemory(void* address, void* readBuf, SIZE_T readSize, SIZE_T* bytesRead);
    virtual BOOL WriteProcessMemory(void* address, void* writeBuf, SIZE_T writeSize, SIZE_T* bytesWritten);
};