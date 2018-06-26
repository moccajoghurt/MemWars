#pragma once
#include <Windows.h>
#include <winternl.h>

typedef struct  _EPROCESS *PEPROCESS;
typedef struct  _ACCESS_STATE *PACCESS_STATE;
typedef struct  _OBJECT_TYPE *POBJECT_TYPE;
typedef CCHAR   KPROCESSOR_MODE;

static POBJECT_TYPE* PsProcessType;
static NTSTATUS (NTAPI* PsLookupProcessByProcessId)(HANDLE, PEPROCESS*);
static VOID (NTAPI* ObDereferenceObject)(PVOID);
static NTSTATUS (NTAPI* ObOpenObjectByPointer)(PVOID,ULONG,PACCESS_STATE,ACCESS_MASK,POBJECT_TYPE,KPROCESSOR_MODE,PHANDLE);

struct OPENPROCESS_DATA {
    HANDLE processId;
    ACCESS_MASK access;
    HANDLE returnedHandle;
};

HANDLE OpenProcessFromKernel(HANDLE processId, ACCESS_MASK access);