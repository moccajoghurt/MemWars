#pragma once
#include <windows.h>
#include <winternl.h>

#define IOCTL_RunPayload64 0xAA013044

typedef PVOID(NTAPI* MmGetSystemRoutineAddress_t)(PUNICODE_STRING);
static HANDLE device;

BOOL InitDriver();
BOOL StartAttack();