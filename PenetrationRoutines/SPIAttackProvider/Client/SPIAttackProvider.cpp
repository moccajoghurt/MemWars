#include "SPIAttackProvider.h"
#include "../../../Core/MemWarsCore.h"
#include "../../../Core/MemWarsServicesCore.h"
#include <iostream>
#include <map>

using namespace std;

BOOL SPIAttackProvider::ReadProcessMemory(void* address, void* readBuf, SIZE_T readSize, SIZE_T* bytesRead) {
    SIZE_T bytesReadBuf;
    if (smc.ReadVirtualMemory(address, readBuf, readSize, &bytesReadBuf)) {
        return TRUE;
    }
    return FALSE;
}
BOOL SPIAttackProvider::WriteProcessMemory(void* address, void* writeBuf, SIZE_T writeSize, SIZE_T* bytesWritten) {
    SIZE_T bytesWrittenBuf;
    if (smc.WriteVirtualMemory(address, writeBuf, writeSize, &bytesWrittenBuf)) {
        return TRUE;
    }
    return FALSE;
}

BOOL SPIAttackProvider::Init(wstring targetProcess, wstring pivotProcess) {
    this->targetProcess = targetProcess;
    this->pivotProcess = pivotProcess;

    if (!smc.Init(pivotProcess)) {
        cout << "Init failed" << endl;
        return FALSE;
    }
    if (!smc.SetTargetProcessHandle(targetProcess)) {
        cout << "Setting Handle failed" << endl;
        return FALSE;
    }
    return TRUE;
}



// int main() {

    
    

//     HANDLE gameHandle = GetProcessHandleByName(/*c.GetwTargetProcessExe()*/L"Warcraft III.exe");
//     if (!gameHandle) {
//         cout << "invalid handle" << endl;
//         return 1;
//     }

//     // c.FindValueRoutine(gameHandle);

//     // c.MemoryMapRoutine();
// }