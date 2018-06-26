#include <windows.h>
#include <iostream>
// #include <intrin.h>
#include "CapcomLoader.h"
#include "CapcomWrapper.h"

using namespace std;

FARPROC GetRoutine(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, const wchar_t* routineName) {
	UNICODE_STRING routineNameU = { 0 };
	RtlInitUnicodeString(&routineNameU, routineName);
	return (FARPROC)pMmGetSystemRoutineAddress(&routineNameU);
}

BOOL InitDriver() {
    if (!LoadDriver()) {
        cout << "InitDriver::Loading driver failed!" << endl;
        return FALSE;
    }

    device = OpenDevice("Htsysm72FB");
    if (!device) {
        cout << "InitDriver::Could not retrieve device!" << endl;
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

    codePayload->pointerToPayload = codePayload->payload;

    ZeroMemory(codePayload->payload, PAYLOAD_BUFFER_SIZE);
    CopyMemory(codePayload->payload, codePayloadBuf, sizeof(codePayloadBuf));

    DWORD status = 0x0;
    DWORD bytesReturned = 0x0;
    DeviceIoControl(device, IOCTL_RunPayload64, &codePayload->pointerToPayload, sizeof(ULONG_PTR), &status, sizeof(status), &bytesReturned, 0);

}
