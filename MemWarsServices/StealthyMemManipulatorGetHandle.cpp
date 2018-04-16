#include "StealthyMemManipulatorGetHandle.h"
 
 
/* This function finds a handle to a process from its name.
It can also find handles to a process belonging to other processes.
Important: Does NOT return a valid HANDLE, it only returns the Handle ID */
HANDLE GetHandleIdTo(std::wstring targetProcessName, DWORD pidOwner) {
	if (targetProcessName == L"")
		return (HANDLE)0x0; // Trying to get a handle to an empty process name
 
	SetPrivilege(SE_DEBUG_NAME); // Getting required privileges
 
	if (pidOwner == NULL) // No owner PID for the handle specified, assuming we are looking for a handle belonging to this program
		pidOwner = GetCurrentProcessId();
 
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID buffer = NULL;
	ULONG buffersize = 0;
	while (true) {
		status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SYSTEMHANDLEINFORMATION, buffer, buffersize, &buffersize);
		if (!NT_SUCCESS(status)) {
			if (status == STATUS_INFO_LENGTH_MISMATCH) {
				if (buffer != NULL)
					VirtualFree(buffer, 0, MEM_RELEASE);
				buffer = VirtualAlloc(NULL, buffersize, MEM_COMMIT, PAGE_READWRITE);
			}
			continue;
		}
		else
			break;
	}
 
	// Enumerate all handles on system
	PSYSTEM_HANDLE_INFORMATION handleInfo = (PSYSTEM_HANDLE_INFORMATION)buffer;
 
	PVOID buffer2 = NULL;
	ULONG buffersize2 = 0;
	for (ULONG i = 0; i < handleInfo->HandleCount; i++) {
		PSYSTEM_HANDLE_TABLE_ENTRY_INFO Handle = (PSYSTEM_HANDLE_TABLE_ENTRY_INFO)&handleInfo->Handles[i];
		if (!Handle)
			continue; // Error, no handle
		if (!Handle->HandleValue)
			continue; // Error, empty handle value
		if (Handle->UniqueProcessId != pidOwner)
			continue; // The handle doesn't belong to the owner we target
 
		HANDLE localHandle = (HANDLE)Handle->HandleValue;
		if (pidOwner != GetCurrentProcessId()) { // Only if trying to get handle from another process (OpenProcess + DuplicateHandle)
			HANDLE hProcessHandleOwner = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pidOwner);
			//BOOL dupStatus = DuplicateHandle(hProcessHandleOwner, HANDLE(Handle->HandleValue), GetCurrentProcess(), &localHandle, PROCESS_QUERY_LIMITED_INFORMATION, FALSE, 0); // Can do with normal method instead of using native function
			NTSTATUS dupStatus = NtDuplicateObject(hProcessHandleOwner, HANDLE(Handle->HandleValue), GetCurrentProcess(), &localHandle, PROCESS_QUERY_LIMITED_INFORMATION, FALSE, 0);
			CloseHandle(hProcessHandleOwner);
			if (dupStatus != 0)
				continue; // Couldn't get a handle to get info, will not be able to define if it is a handle to our process, exiting
		}
 
		int trys = 0;
		while (true) {
			if (trys == 20)
				break;
			trys += 1;
 
			/* In rare cases, when a handle has been closed between the snapshot and this NtQueryObject, the handle is not valid at that line.
			This is problematic in system processes with a strict handle policy and can result in process termination, forcing a reboot (Windows 8+) or a BSOD (Windows 7)
			Note that this is not problematic in classic processes. */
			status = NtQueryObject(localHandle, ObjectTypeInformation, buffer2, buffersize2, &buffersize2); // Return objecttypeinfo into buffer
			if (!NT_SUCCESS(status)) {
				if (buffer2 != NULL)
					VirtualFree(buffer2, 0, MEM_RELEASE); // If buffer filled with anything, but call didnt succeed, assume its bullshit, so clear it
				buffer2 = VirtualAlloc(NULL, buffersize2, MEM_COMMIT, PAGE_READWRITE); // Allocate with new mem
			}
			else {
				if (wcsncmp(((POBJECT_TYPE_INFORMATION)buffer2)->TypeName.Buffer, L"Process", ((POBJECT_TYPE_INFORMATION)buffer2)->TypeName.Length + 1) == 0) {
					wchar_t process[MAX_PATH];
					if (GetModuleFileNameExW(localHandle, NULL, process, MAX_PATH)) {
						std::wstring processname = process;
						int pos = processname.find_last_of(L"\\");
						processname = processname.substr(pos + 1, processname.length());
						if (processname == targetProcessName) {
							HANDLE handleFound = (HANDLE)Handle->HandleValue;
							VirtualFree(buffer, 0, MEM_RELEASE); // Cleanup to avoid leaks
							VirtualFree(buffer2, 0, MEM_RELEASE);
							if (pidOwner != GetCurrentProcessId())
								CloseHandle(localHandle);
							SetPrivilege(SE_DEBUG_NAME, FALSE); // Removing special privileges to avoid detection vectors
							return handleFound; // TODO: Improve by returning a vector of handles, there might be several with different access rights
						}
						else
							break;
					}
				}
				else
					break;
			}
		}
		if (Handle->UniqueProcessId != GetCurrentProcessId())
			CloseHandle(localHandle); // Cleanup
		continue;
	}
	VirtualFree(buffer, 0, MEM_RELEASE); // Empties buffers to avoid memory leaks
	VirtualFree(buffer2, 0, MEM_RELEASE); // Empties buffers to avoid memory leaks
	SetPrivilege(SE_DEBUG_NAME, FALSE);
	return (HANDLE)0x0;
}


bool SetPrivilege(LPCSTR lpszPrivilege, BOOL bEnablePrivilege) {
	TOKEN_PRIVILEGES priv = { 0,0,0,0 };
	HANDLE hToken = NULL;
	LUID luid = { 0,0 };
	BOOL Status = true;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
		Status = false;
		goto EXIT;
	}
	if (!LookupPrivilegeValueA(0, lpszPrivilege, &luid)) {
		Status = false;
		goto EXIT;
	}
	priv.PrivilegeCount = 1;
	priv.Privileges[0].Luid = luid;
	priv.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : SE_PRIVILEGE_REMOVED;
	if (!AdjustTokenPrivileges(hToken, false, &priv, 0, 0, 0)) {
		Status = false;
		goto EXIT;
	}
EXIT:
	if (hToken)
		CloseHandle(hToken);
	return Status;
}