#include "SPIAttackProvider.h"
#include "../../../Core/MemWarsCore.h"
#include "../../../Core/MemWarsServicesCore.h"
#include <iostream>
#include <map>

using namespace std;

BOOL SPIAttackProvider::ReadProcessMemory(HANDLE hProcess, void* address, void* readBuf, SIZE_T readSize, SIZE_T* bytesRead) {
    if (smc.ReadVirtualMemory(address, readBuf, readSize, bytesRead)) {
        return TRUE;
    }
    return FALSE;
}
BOOL SPIAttackProvider::WriteProcessMemory(HANDLE hProcess, void* address, void* writeBuf, SIZE_T writeSize, SIZE_T* bytesWritten) {
    if (smc.WriteVirtualMemory(address, writeBuf, writeSize, bytesWritten)) {
        return TRUE;
    }
    return FALSE;
}

BOOL SPIAttackProvider::Init(wstring targetProcess, wstring pivotProcess) {
    this->targetProcess = targetProcess;
    this->pivotProcess = pivotProcess;

    if (!smc.Init(pivotProcess)) {
        cout << "SPIAttackProvider::Init Pivot process or HANDLE ID of target process inside pivot process not found" << endl;
        return FALSE;
    }
    if (!smc.SetTargetProcessHandle(targetProcess)) {
        cout << "SPIAttackProvider::Init Setting Handle failed" << endl;
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