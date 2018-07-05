#include <windows.h>
#include <string>
#include <stdio.h>
#include "MemoryController.h"
#include "../CapcomDriverAttack/CapcomWrapper.h"

using namespace std;

struct ALLOCATE_DATA {
    SIZE_T size;
    PVOID memPtr = NULL;
};

void __stdcall AllocateKernelMemory(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
    ALLOCATE_DATA* pData = (ALLOCATE_DATA*)userData;
    pData->memPtr = ExAllocatePool(0ull, pData->size);
}

PVOID AllocateKernelMemory(SIZE_T size) {
    ALLOCATE_DATA data;
    data.size = size;
    RunInKernel(AllocateKernelMemory, &data);
    if (data.memPtr == NULL) {
        return NULL;
    }
    return data.memPtr;
}

BOOL ExposeKernelMemoryToProcess(PVOID memory, SIZE_T size, uint64_t eProcess) {

    
    return TRUE;
}


BOOL StealthInject(string processName, string dllPath) {

    if (!InitMemoryController()) {
        return FALSE;
    }

    PVOID kernelMemory = AllocateKernelMemory(1000);
    if (kernelMemory == NULL) {
        return FALSE;
    }

    
    // uint64_t currentEProcess = GetCurrentEProcess();
    // printf("%16llx\n", currentEProcess);
    // uint64_t currentPid = GetProcessId(currentEProcess);
    // printf("%16llx\n",  currentPid);

    // todo ExposeKernelMemoryToProcess benötigt EProcess

    // ExposeKernelMemoryToProcess(kernelMemory, 1000, 0);


    return TRUE;
}

