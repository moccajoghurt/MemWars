#pragma once
#pragma warning( disable:4005 )
#include <windows.h>
#include <Winternl.h>
#include <ntstatus.h>
#include <Psapi.h>
#include <string>
#define SYSTEMHANDLEINFORMATION 16
#pragma comment (lib, "ntdll.lib")
 
typedef struct _SYSTEM_HANDLE {
	ULONG ProcessId;
	UCHAR ObjectTypeNumber;
	UCHAR Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;
 
typedef struct _SYSTEM_HANDLE_INFORMATION {
	ULONG HandleCount; // Or NumberOfHandles if you prefer
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;
 
typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO {
	DWORD UniqueProcessId;
	WORD HandleType;
	USHORT HandleValue;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;
 
typedef struct _OBJECT_TYPE_INFORMATION {
	UNICODE_STRING TypeName;
	ULONG TotalNumberOfObjects;
	ULONG TotalNumberOfHandles;
	ULONG TotalPagedPoolUsage;
	ULONG TotalNonPagedPoolUsage;
	ULONG TotalNamePoolUsage;
	ULONG TotalHandleTableUsage;
	ULONG HighWaterNumberOfObjects;
	ULONG HighWaterNumberOfHandles;
	ULONG HighWaterPagedPoolUsage;
	ULONG HighWaterNonPagedPoolUsage;
	ULONG HighWaterNamePoolUsage;
	ULONG HighWaterHandleTableUsage;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccessMask;
	BOOLEAN SecurityRequired;
	BOOLEAN MaintainHandleCount;
	UCHAR TypeIndex;
	CHAR ReservedByte;
	ULONG PoolType;
	ULONG DefaultPagedPoolCharge;
	ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;
 
EXTERN_C NTSTATUS NTAPI NtDuplicateObject(HANDLE, HANDLE, HANDLE, PHANDLE, ACCESS_MASK, BOOLEAN, ULONG);
 
bool SetPrivilege(LPCSTR lpszPrivilege, BOOL bEnablePrivilege = TRUE);
 
/* This function finds a handle to a process from its name.
It can also find handles to a process belonging to other processes.
Important: Does NOT return a valid HANDLE, it only returns the Handle ID */
HANDLE GetHandleToId(std::wstring targetProcessName, DWORD pidOwner = NULL);
