
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <Winternl.h>
#include <Psapi.h>
#include <Winnt.h>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include "../../Core/MemWarsCore.h"
#include "../../Core/MemWarsServicesCore.h"
#include "StealthyMemManipulatorInstaller.h"

using namespace std;

BOOL StealthyMemInstaller::Init(vector<wstring> preferedTIDsModules, wstring targetProcessName) {
    // hiding names
    // sharedMemName = 'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M';
	// globalMutex = 'G'+'l'+'o'+'b'+'a'+'l'+'\\'+'S'+'M'+'e'+'m'+'M'+'M'+'t'+'x';
	// explExeName = L'e'+L'x'+L'p'+L'l'+L'o'+L'r'+L'e'+L'r'+L'.'+L'e'+L'x'+L'e';
	sharedMemName = "Global\\SMemM";
	globalMutex = "Global\\SMemMMtx";
	explExeName = L"explorer.exe";
	this->targetProcessName = targetProcessName;
	this->preferedTIDsModules = preferedTIDsModules;
    return TRUE;
}

BOOL StealthyMemInstaller::Install() {

	if (AlreadyInstalled()) {
		cout << "Install() failed: already installed." << endl;
        return TRUE;
	}

	if (InstanceAlreadyRunning()) {
        cout << "Install() failed: Instance already running." << endl;
        return FALSE;
	}

    if (!SetProcessPrivilege(SE_DEBUG_NAME, TRUE)) {
        cout << "Install() failed: Privilege failed." << endl;
        return FALSE;
    }
	
	if (!GetTargetProcessPID()) {
        cout << "Install() failed: GetTargetProcessPID failed." << endl;
        return FALSE;
    }

    if (!GetTargetProcessHandle()) {
		cout << "Install() failed: GetTargetProcessHandle failed." << endl;
        return FALSE;
	}

    if (!GetRemoteExecutableMemory()) {
		cout << "Install() failed: GetRemoteExecutableMemory failed." << endl;
        return FALSE;
	}
	
	if (!FindUsableTID()) {
		cout << "Install() failed: FindUsableTID failed." << endl;
		return FALSE;
	}

	if (!CreateSharedFileMapping()) {
		cout << "Install() failed: CreateSharedFileMapping failed. " << GetLastError() <<endl;
		return FALSE;
	}

	if (!CreateExternalGatekeeperHandleToFileMapping()) {
		cout << "Install() failed: Gatekeeperhandle failed." << endl;
		return FALSE;
	}

	if (!InjectFileMappingShellcodeIntoTargetThread()) {
        cout << "Install() failed: ConnectFileMappingWithTargetThread failed." << endl;
		return FALSE;
	}
	CloseHandle(hLocalSharedMem);

	if (!InjectCommunicationShellcodeIntoTargetThread()) {
		cout << "Install() failed: InjectCommunicationShellcodeIntoTargetThread failed." << endl;
		return FALSE;
	}

	WriteReconnectionInfoIntoSharedMemory();
	CleanUp();
		
    return TRUE;
}

BOOL StealthyMemInstaller::InstanceAlreadyRunning() {
    hGlobalMutex = CreateMutexA(NULL, TRUE, globalMutex.c_str());
	if (hGlobalMutex == NULL) {
        // something weird went wrong, we return true so the installer knows it can't continue
        return TRUE;
        
    } else if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // An instance of either the installer or the client is already running, terminate now
        return TRUE; 

	}
	return FALSE;
}

BOOL StealthyMemInstaller::GetTargetProcessPID() {
	vector<DWORD> pidsTargetProcess = GetPIDsOfProcess(targetProcessName);
	if (pidsTargetProcess.empty()) {
        cout << "Install() failed: no PIDs found." << endl;
        return FALSE;
    }
    // in case the target process has multiple PIDs, take the first one
    sort(pidsTargetProcess.begin(), pidsTargetProcess.end());
    targetProcessPID = pidsTargetProcess[0];
	if (!targetProcessPID) {
        cout << "Install() failed: Invalid PID." << endl;
        return FALSE;
	}
	return TRUE;
}

BOOL StealthyMemInstaller::AlreadyInstalled() {
	hSharedMemHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, sharedMemName.c_str());
	if (hSharedMemHandle) {
        return TRUE;
	}
	return FALSE;
}

BOOL StealthyMemInstaller::GetTargetProcessHandle() {
	hTargetProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
        FALSE,
        targetProcessPID
    );
	if (!hTargetProcess) {
        cout << "Install() failed: Cannot open process. " << GetLastError() << endl;
        return FALSE;
	}
	return TRUE;
}

BOOL StealthyMemInstaller::GetRemoteExecutableMemory() {
	vector<UNUSED_EXECUTABLE_MEM> availableExecutableMem = FindExecutableMemory(hTargetProcess, TRUE);
    if (availableExecutableMem.empty() || 
        availableExecutableMem[0].start == nullptr || 
        availableExecutableMem[0].size == NULL ||
        availableExecutableMem[0].size <= PADDING_IN_EXECUTABLE_MEM
    ) {
        cout << "Install() failed: No avaiable mem." << endl;
        return FALSE;
    }
    remoteExecutableMem = (void*)((DWORD64)availableExecutableMem[0].start + PADDING_IN_EXECUTABLE_MEM);
	remoteExecutableMemSize = availableExecutableMem[0].size - PADDING_IN_EXECUTABLE_MEM;
	return TRUE;
}


BOOL StealthyMemInstaller::CreateSharedFileMapping() {
    hLocalSharedMem = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE | SEC_COMMIT | SEC_NOCACHE,
        0,
        sharedMemSize,
        sharedMemName.c_str()
    );
	if (!hLocalSharedMem) {
        return FALSE;
    }
		
	ptrLocalSharedMem = MapViewOfFile(hLocalSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, sharedMemSize);
	if (!ptrLocalSharedMem) {
        return FALSE;
	}
	usableSharedMemSize = sharedMemSize - sizeof(SHARED_MEM_INFO);
    return TRUE;
}

vector <UNUSED_EXECUTABLE_MEM> StealthyMemInstaller::FindExecutableMemory(const HANDLE hProcess, BOOL onlyInBase) {
	MEMORY_BASIC_INFORMATION memInfo;
	vector<MEMORY_BASIC_INFORMATION> memInfos;
	vector<MEMORY_BASIC_INFORMATION> execMemInfos;
	vector<UNUSED_EXECUTABLE_MEM> freeExecutableMems;
	void* baseAddr = nullptr;
 
	if (onlyInBase) {
        baseAddr = GetProcessBaseAddress(hProcess);
        if (baseAddr == NULL) {
            return freeExecutableMems;
        }
    }
 
	// Getting all MEMORY_BASIC_INFORMATION of the target process
	unsigned char* addr = NULL;
	for (addr = NULL; VirtualQueryEx(hProcess, addr, &memInfo, sizeof(memInfo)) == sizeof(memInfo); addr += memInfo.RegionSize)
		if (!onlyInBase || (onlyInBase && memInfo.AllocationBase == baseAddr)) {
            memInfos.push_back(memInfo);
        }
	if (memInfos.empty()) {
        return freeExecutableMems;
    }
 
	// Filtering only executable memory regions
	for (int i = 0; i < memInfos.size(); ++i) {
		DWORD prot = memInfos[i].Protect;
		if (prot == PAGE_EXECUTE || prot == PAGE_EXECUTE_READ || prot == PAGE_EXECUTE_READWRITE || prot == PAGE_EXECUTE_WRITECOPY)
			execMemInfos.push_back(memInfos[i]);
	}
	if (execMemInfos.empty()) {
        return freeExecutableMems;
    }
 
	// Duplicating memory locally for analysis, finding unused memory at the end of executable regions
	for (int i = 0; i < execMemInfos.size(); ++i) {
		void* localMemCopy = VirtualAlloc(NULL, execMemInfos[i].RegionSize, MEM_COMMIT, PAGE_READWRITE);
		if (localMemCopy == NULL) {
            continue;
        }
 
		// Copying executable memory content locally
		SIZE_T bytesRead = 0;
		if (!ReadProcessMemory(hProcess, execMemInfos[i].BaseAddress, localMemCopy, execMemInfos[i].RegionSize, &bytesRead)) {
			VirtualFree(localMemCopy, execMemInfos[i].RegionSize, MEM_RELEASE);
			continue;
		}
 
		// Analyzing unused executable memory size and location locally
		BYTE currentByte = 0;
		SIZE_T unusedSize = 0;
		DWORD64 analysingByteAddr = (DWORD64)localMemCopy + execMemInfos[i].RegionSize - 1;
		while (analysingByteAddr >= (DWORD64)localMemCopy) {
			CopyMemory(&currentByte, (void*)analysingByteAddr, sizeof(BYTE));
			if (currentByte != 0) {
                break;
            }
			unusedSize++;
			analysingByteAddr--;
		}
		if (unusedSize == 0) {
			VirtualFree(localMemCopy, execMemInfos[i].RegionSize, MEM_RELEASE);
			continue;
		}
 
		UNUSED_EXECUTABLE_MEM unusedExecutableMem;
		unusedExecutableMem.regionInfo = execMemInfos[i];
		unusedExecutableMem.size = unusedSize;
		unusedExecutableMem.start = (void*)((DWORD64)execMemInfos[i].BaseAddress + execMemInfos[i].RegionSize - unusedSize);
		freeExecutableMems.push_back(unusedExecutableMem);
 
        VirtualFree(localMemCopy, execMemInfos[i].RegionSize, MEM_RELEASE);
	}
 
	return freeExecutableMems;
}

BOOL StealthyMemInstaller::FindUsableTID() {
	map<DWORD, wstring> tidsStartModules = GetTIDsModuleStartAddr(targetProcessPID);
	wstring modName = L"";
	for (int i = 0; i < preferedTIDsModules.size(); ++i) {
		for (auto const& thisTid : tidsStartModules) {
			DWORD tid = thisTid.first;
			modName = thisTid.second;
			if (modName == preferedTIDsModules[i]) {
				targetTID = tid;
				break;
			}
		}
		if (targetTID) {
			break;
		}
			
	}
	if (!targetTID) {
		return FALSE; // Could not find any of the threads starting in one of the target modules
	}
	wcout << "using: " << modName << endl;
	hTargetThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, targetTID);
	if (!hTargetThread) {
		cout << "Install()::FindUsableTID() failed: could not open thread." << endl;
		return FALSE;
	}

	return TRUE;
}


BOOL StealthyMemInstaller::CreateExternalGatekeeperHandleToFileMapping() {
	vector<DWORD> explorerPIDs = GetPIDsOfProcess(explExeName);
	if (explorerPIDs.empty()) {
		cout << "here" << endl;
		return FALSE;
	}
		
	hGateKeeperProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, explorerPIDs[0]);
	if (!hGateKeeperProcess) {
		return FALSE;
	}
	HANDLE hGateKeeper = NULL;
	if (!DuplicateHandle(
		GetCurrentProcess(), 
		hLocalSharedMem, 
		hGateKeeperProcess, 
		&hGateKeeper, 
		NULL, 
		FALSE, 
		DUPLICATE_SAME_ACCESS)
	) {
		return FALSE;
	}
	CloseHandle(hGateKeeperProcess);
	return TRUE;
}

BOOL StealthyMemInstaller::InjectFileMappingShellcodeIntoTargetThread() {
	// Getting function addresses
	FARPROC addrOpenFileMappingA = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "OpenFileMappingA");
	FARPROC addrMapViewOfFile = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "MapViewOfFile");
	FARPROC addrCloseHandle = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "CloseHandle");
	if (!addrOpenFileMappingA || !addrMapViewOfFile || !addrCloseHandle) {
		return FALSE;
	}
		
	// Get RW memory to assemble full shellcode from parts
	void* rwMemory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (rwMemory == nullptr) {
		return FALSE;
	}
		
	DWORD64 addrEndOfShellCode = (DWORD64)rwMemory;

	// break test
	// UCHAR x64Breakpoint[] = {0xCC};
	// CopyMemory((void*)addrEndOfShellCode, x64Breakpoint, sizeof(x64Breakpoint));
	// addrEndOfShellCode += sizeof(x64Breakpoint);
	// break test end
 
	UCHAR x64OpenFileMappingA[] = {
		0x48, 0xc7, 0xc1, 0x1f, 0, 0x0f, 0,	// mov rcx, dwDesiredAccess			+0 (FILE_MAP_ALL_ACCESS = 0xf001f @ +3)
		0x48, 0x31, 0xd2,					// xor rdx, rdx						+7 (bInheritHandle = FALSE)
		0x49, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0,	// mov r8, &lpName					+10 (&lpName +12)
		0x4d, 0x31, 0xc9,					// xor r9, r9						+20
		0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, // mov rax, addrOpenFileMappingA	+23 (addrOpenFileMappingA +25)
		0x48, 0x83, 0xec, 0x20,				// sub rsp, 0x20					+33
		0xff, 0xd0,							// call rax							+37
		0x48, 0x83, 0xc4, 0x20,				// add rsp, 0x20					+39
		0x49, 0x89, 0xc7					// mov r15, rax						+43
	};
	*(DWORD64*)((PUCHAR)x64OpenFileMappingA + 25) = (DWORD64)(ULONG_PTR)addrOpenFileMappingA;
	CopyMemory((void*)addrEndOfShellCode, x64OpenFileMappingA, sizeof(x64OpenFileMappingA));
	addrEndOfShellCode += sizeof(x64OpenFileMappingA);
 
	UCHAR x64MapViewOfFile[] = {
		0x48, 0x89, 0xc1,					// mov rcx, rax						+0
		0x48, 0xc7, 0xc2, 0x1f, 0, 0x0f, 0,	// mov rdx, dwDesiredAccess			+3 (FILE_MAP_ALL_ACCESS = 0xf001f @ +6)
		0x4d, 0x31, 0xc0,					// xor r8, r8						+10
		0x4d, 0x31, 0xc9,					// xor r9, r9						+13
		0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, // mov rax, dwNumberOfBytesToMap	+16 (dwNumberOfBytesToMap +18)
		0x50,								// push rax							+26
		0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0,	// mov rax, addrMapViewOfFile		+27 (addrMapViewOfFile +29)
		0x48, 0x83, 0xec, 0x20,				// sub rsp, 0x20					+37
		0xff, 0xd0,							// call rax							+41
		0x48, 0x83, 0xc4, 0x28,				// add rsp, 0x28					+43
		0x49, 0x89, 0xc6,					// mov r14, rax						+47
		// Writing to shared memory the virtual address in pivot process
		0x4d, 0x89, 0x36					// mov [r14], r14					+50
	};
	*(SIZE_T*)((PUCHAR)x64MapViewOfFile + 18) = (SIZE_T)(ULONG_PTR)sharedMemSize;
	*(DWORD64*)((PUCHAR)x64MapViewOfFile + 29) = (DWORD64)(ULONG_PTR)addrMapViewOfFile;
	CopyMemory((void*)addrEndOfShellCode, x64MapViewOfFile, sizeof(x64MapViewOfFile));
	addrEndOfShellCode += sizeof(x64MapViewOfFile);
 
	UCHAR x64CloseHandle[] = {
		0x4C, 0x89, 0xF9,					// mov rcx, r15						+0
		0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0,	// mov rax, addrCloseHandle			+3 (addrCloseHandle +5)
		0x48, 0x83, 0xec, 0x20,				// sub rsp, 0x20					+13
		0xff, 0xd0,							// call rax							+17
		0x48, 0x83, 0xc4, 0x20				// add rsp, 0x20					+19
	};
	*(DWORD64*)((PUCHAR)x64CloseHandle + 5) = (DWORD64)(ULONG_PTR)addrCloseHandle;
	CopyMemory((void*)addrEndOfShellCode, x64CloseHandle, sizeof(x64CloseHandle));
	addrEndOfShellCode += sizeof(x64CloseHandle);
 
	UCHAR x64InfiniteLoop[] = { 0xEB, 0xFE }; // nop + jmp rel8 -2
	CopyMemory((void*)addrEndOfShellCode, x64InfiniteLoop, sizeof(x64InfiniteLoop));
	addrEndOfShellCode += sizeof(x64InfiniteLoop);
 
	UCHAR lpNameBuffer[30];
	SecureZeroMemory(lpNameBuffer, sizeof(lpNameBuffer));
	CopyMemory(lpNameBuffer, sharedMemName.c_str(), sharedMemName.size());
	CopyMemory((void*)addrEndOfShellCode, lpNameBuffer, sizeof(lpNameBuffer));
	addrEndOfShellCode += sizeof(lpNameBuffer);
 
	SIZE_T fullShellcodeSize = addrEndOfShellCode - (DWORD64)rwMemory;
 
	DWORD64 lpNameInRemoteExecMemory = (DWORD64)remoteExecutableMem + fullShellcodeSize - sizeof(lpNameBuffer);
	CopyMemory((void*)((DWORD64)rwMemory + 12), &lpNameInRemoteExecMemory, sizeof(lpNameInRemoteExecMemory));
 
	// bool pushShellcodeStatus = PushShellcode(rwMemory, fullShellcodeSize);
	if (fullShellcodeSize > remoteExecutableMemSize) {
		return FALSE;
	}
	BOOL pushShellcodeStatus = WriteProcessMemoryAtPtrLocation(hTargetProcess, remoteExecutableMem, rwMemory, fullShellcodeSize);
	VirtualFree(rwMemory, 0, MEM_RELEASE);
	if (!pushShellcodeStatus) {
		return FALSE;
	}
	
	if (!ExecShellcodeWithHijackedThread(fullShellcodeSize - sizeof(lpNameBuffer), FALSE)) {
		return FALSE;
	} 
 
	CopyMemory(&ptrRemoteSharedMem, ptrLocalSharedMem, sizeof(void*));
	if (ptrRemoteSharedMem == nullptr) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

BOOL StealthyMemInstaller::ExecShellcodeWithHijackedThread(SIZE_T shellcodeSize = NULL, BOOL thenRestore = TRUE) {
	// Preparing for thread hijacking
	CONTEXT tcInitial;
	CONTEXT tcHiJack;
	CONTEXT tcCurrent;
	SecureZeroMemory(&tcInitial, sizeof(CONTEXT));
	tcInitial.ContextFlags = CONTEXT_ALL;
 
	// Suspend thread and send it executing our shellcode
	DWORD suspendCount = SuspendThread(hTargetThread);
	if (suspendCount > 0) {
		// The thread was already suspended
		for (int i = 0; i < suspendCount; ++i) {
			ResumeThread(hTargetThread);
		}
	}
	GetThreadContext(hTargetThread, &tcInitial);
	CopyMemory(&tcHiJack, &tcInitial, sizeof(CONTEXT)); // Faster than another call to GetThreadContext
	CopyMemory(&tcCurrent, &tcInitial, sizeof(CONTEXT));
	tcHiJack.Rip = (DWORD64)remoteExecutableMem;
	SetThreadContext(hTargetThread, &tcHiJack);
	ResumeThread(hTargetThread);
 
	if (shellcodeSize == NULL) {
		return TRUE; // Permanent thread hijack, do not wait for any execution completion
	}

	// Check the thread context to know when done executing (RIP should be at memory address + size of shellcode - 2 in the infinite loop jmp rel8 -2)
	DWORD64 addrEndOfExec = (DWORD64)remoteExecutableMem + shellcodeSize - 2;
	do {
		GetThreadContext(hTargetThread, &tcCurrent);
	} while (tcCurrent.Rip != addrEndOfExec);

	if (thenRestore) {
		// Execution finished, resuming previous operations
		SuspendThread(hTargetThread);
		SetThreadContext(hTargetThread, &tcInitial);
		ResumeThread(hTargetThread);
	}
 
	return TRUE;
}

BOOL StealthyMemInstaller::InjectCommunicationShellcodeIntoTargetThread() {
	// Pushing control structure into shared memory
	REMOTE_COMMAND_INFO controlStruct;
	void* controlLocalAddr = (void*)((DWORD64)ptrLocalSharedMem + sharedMemSize - sizeof(controlStruct));
	CopyMemory(controlLocalAddr, &controlStruct, sizeof(controlStruct));
	void* controlRemoteAddr = (void*)((DWORD64)ptrRemoteSharedMem + sharedMemSize - sizeof(controlStruct));
 
	// Getting function addresses
	// string e = "";
	// string ntrvmNoStr = e+'N'+'t'+'R'+'e'+'a'+'d'+'V'+'i'+'r'+'t'+'u'+'a'+'l'+'M'+'e'+'m'+'o'+'r'+'y';
	// string ntwvmNoStr = e+'N'+'t'+'W'+'r'+'i'+'t'+'e'+'V'+'i'+'r'+'t'+'u'+'a'+'l'+'M'+'e'+'m'+'o'+'r'+'y';
	string ntrvmNoStr = "NtReadVirtualMemory";
	string ntwvmNoStr = "NtWriteVirtualMemory";
	DWORD syscallIndexZwRVM = GetSyscallId("ntdll.dll", ntrvmNoStr);
	DWORD syscallIndexZwWVM = GetSyscallId("ntdll.dll", ntwvmNoStr);
	if (!syscallIndexZwRVM || !syscallIndexZwWVM) {
		return FALSE;
	}
 
	// Get RW memory to assemble full shellcode from parts
	void* rwMemory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (rwMemory == nullptr) {
		return FALSE;
	}
	DWORD64 addrEndOfShellCode = (DWORD64)rwMemory;

	// break test
	// UCHAR x64Breakpoint[] = {0xCC};
	// CopyMemory((void*)addrEndOfShellCode, x64Breakpoint, sizeof(x64Breakpoint));
	// addrEndOfShellCode += sizeof(x64Breakpoint);
	// break test end
 
	UCHAR x64Spinlock[] = {
		0xA0, 0, 0, 0, 0, 0, 0, 0, 0,	// mov al, [&exec]
		0x3c, 0,						// cmp al, 0
		0xF3, 0x90,						// pause (signals the CPU that we are in a spinlock)
		0x75, 0xF1						// jnz -14
	};
	*(DWORD64*)((PUCHAR)x64Spinlock + 1) = (DWORD64)(ULONG_PTR)controlRemoteAddr;
	CopyMemory((void*)addrEndOfShellCode, x64Spinlock, sizeof(x64Spinlock));
	addrEndOfShellCode += sizeof(x64Spinlock);
 
	// Do not retrieve nbr of bytes read/written (otherwise mov rax, ptr)
	UCHAR x64ZeroRax[] = { 0x48, 0x31, 0xC0 }; // xor rax, rax
	CopyMemory((void*)addrEndOfShellCode, x64ZeroRax, sizeof(x64ZeroRax));
	addrEndOfShellCode += sizeof(x64ZeroRax);
 
	UCHAR x64ZwRWVM[] = {
		// Preparing argument passing to NtRVM/NtWVM
		0x50,								// push rax						+0 (NumberOfBytesRead, optional)
		0x48, 0x83, 0xec, 0x28,				// sub rsp, 0x28				+1 (+8 normally the return address pushed by NtRVM call)
		0x48, 0xa1, 0, 0, 0, 0, 0, 0, 0, 0,	// mov rax, [&hProcess]			+5 (&hProcess +7)
		0x48, 0x89, 0xc1,					// mov rcx, rax					+15
		0x48, 0xa1, 0, 0, 0, 0, 0, 0, 0, 0,	// mov rax, [&lpBaseAddress]	+18 (&lpBaseAddress +20)
		0x48, 0x89, 0xc2,					// mov rdx, rax					+28
		0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0,	// mov rax, [&lpBuffer]			+31 (&lpBuffer +33)
		0x49, 0x89, 0xc0,					// mov r8, rax					+41
		0x48, 0xa1, 0, 0, 0, 0, 0, 0, 0, 0,	// mov rax, [&nSize]			+44 (&nSize +46)
		0x49, 0x89, 0xc1,					// mov r9, rax					+54
		// Loading function pointer accordingly to current order
		0xa0, 0, 0, 0, 0, 0, 0, 0, 0,		// mov al, [&order]				+57 (&order +58)
		0x3c, 0x0,							// cmp al, 0x0					+66
		0x49, 0x89, 0xCA,					// mov r10, rcx					+68
		0x75, 0x9,							// jne +9						+71
		0xb8, 0, 0, 0, 0,					// mov eax, WZWVM_SYSCALLID		+73 (WZWVM_SYSCALLID +74)
		0x0f, 0x05,							// syscall						+78
		0xeb, 0x7,							// jmp +7						+80
		0xb8, 0, 0, 0, 0,					// mov eax, WZRVM_SYSCALLID		+82 (WZRVM_SYSCALLID +83)
		0x0f, 0x05,							// syscall						+87
		0x48, 0x83, 0xC4, 0x30				// add rsp, 0x30				+89
	};
	*(DWORD64*)((PUCHAR)x64ZwRWVM + 7) = (DWORD64)(ULONG_PTR)((DWORD64)controlRemoteAddr + 16);
	*(DWORD64*)((PUCHAR)x64ZwRWVM + 20) = (DWORD64)(ULONG_PTR)((DWORD64)controlRemoteAddr + 24);
	*(DWORD64*)((PUCHAR)x64ZwRWVM + 33) = (DWORD64)(ULONG_PTR)ptrRemoteSharedMem;
	*(DWORD64*)((PUCHAR)x64ZwRWVM + 46) = (DWORD64)(ULONG_PTR)((DWORD64)controlRemoteAddr + 32);
	*(DWORD64*)((PUCHAR)x64ZwRWVM + 58) = (DWORD64)(ULONG_PTR)((DWORD64)controlRemoteAddr + 8);
	*(DWORD*)((PUCHAR)x64ZwRWVM + 74) = (DWORD)(ULONG_PTR)syscallIndexZwRVM;
	*(DWORD*)((PUCHAR)x64ZwRWVM + 83) = (DWORD)(ULONG_PTR)syscallIndexZwWVM;
	CopyMemory((void*)addrEndOfShellCode, x64ZwRWVM, sizeof(x64ZwRWVM));
	addrEndOfShellCode += sizeof(x64ZwRWVM);
 
	UCHAR x64ToggleSpinlock[] = {
		0xB0, 1,												// mov al, 1
		0xA2, 0, 0, 0, 0, 0, 0, 0, 0							// mov [&exec], al
	};
	*(DWORD64*)((PUCHAR)x64ToggleSpinlock + 3) = (DWORD64)(ULONG_PTR)controlRemoteAddr;
	CopyMemory((void*)addrEndOfShellCode, x64ToggleSpinlock, sizeof(x64ToggleSpinlock));
	addrEndOfShellCode += sizeof(x64ToggleSpinlock);
 
	// End of cycle, jump back to start
	UCHAR x64AbsoluteJump[] = {
		0x48, 0xb8,	0, 0, 0, 0, 0, 0, 0, 0,	// mov rax, m_remoteExecMem		+0 (m_remoteExecMem +2)
		0xff, 0xe0							// jmp rax								+10
	};
	*(DWORD64*)((PUCHAR)x64AbsoluteJump + 2) = (DWORD64)(ULONG_PTR)remoteExecutableMem;
	CopyMemory((void*)addrEndOfShellCode, x64AbsoluteJump, sizeof(x64AbsoluteJump));
	addrEndOfShellCode += sizeof(x64AbsoluteJump);
	
	SIZE_T fullShellcodeSize = addrEndOfShellCode - (DWORD64)rwMemory;
	// BOOL pushShellcodeStatus = PushShellcode(rwMemory, fullShellcodeSize);
	if (fullShellcodeSize > remoteExecutableMemSize) {
		return FALSE;
	}
	BOOL pushShellcodeStatus = WriteProcessMemoryAtPtrLocation(hTargetProcess, remoteExecutableMem, rwMemory, fullShellcodeSize);
	VirtualFree(rwMemory, 0, MEM_RELEASE);
	if (!pushShellcodeStatus) {
		return FALSE;
	}
	if (!ExecShellcodeWithHijackedThread()) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

DWORD StealthyMemInstaller::GetSyscallId(string strModule, string strProcName) {
	FARPROC pFunction = GetProcAddress(GetModuleHandleA(strModule.c_str()), strProcName.c_str());
	
	BYTE x64PreSyscallOpcodes[] = {
		0x4C, 0x8B, 0xD1,	// mov r10, rcx;
		0xB8				// mov eax, XXh ; Syscall ID
	};
 
	for (int i = 0; i < 4; ++i) {
		if (*(BYTE*)((DWORD64)pFunction + i) != x64PreSyscallOpcodes[i]) {
			return 0; // The function has been tampered with already...
		}
	}
 
	DWORD sysCallIndex = *(DWORD*)((DWORD64)pFunction + 4);
	return sysCallIndex;
}

void StealthyMemInstaller::WriteReconnectionInfoIntoSharedMemory() {
	// Pushes useful information into shared memory, in case the bypass has to reconnect
	CONTEXT contextEmpty;
	SecureZeroMemory(&contextEmpty, sizeof(contextEmpty));
	SHARED_MEM_INFO cfgBackup;
	cfgBackup.ptrRemoteSharedMem = ptrRemoteSharedMem;
	cfgBackup.sharedMemSize = sharedMemSize;
	cfgBackup.remoteExecMem = remoteExecutableMem;
	cfgBackup.remoteExecMemSize = remoteExecutableMemSize;
	void* endOfUsableLocalSharedMem = (void*)((DWORD64)ptrLocalSharedMem + sharedMemSize - sizeof(REMOTE_COMMAND_INFO));
	void* backupAddrInSharedMem = (void*)((DWORD64)endOfUsableLocalSharedMem - sizeof(SHARED_MEM_INFO));
	CopyMemory(backupAddrInSharedMem, &cfgBackup, sizeof(cfgBackup));
}

void StealthyMemInstaller::CleanUp() {
	if (hSharedMemHandle) {
		CloseHandle(hSharedMemHandle);
	}
	if (hTargetProcess) {
		CloseHandle(hTargetProcess);
	}
	if (hTargetThread) {
		CloseHandle(hTargetThread);
	}
	if (hLocalSharedMem) {
		CloseHandle(hLocalSharedMem);
	}
	if (ptrLocalSharedMem) {
		UnmapViewOfFile(ptrLocalSharedMem);
	}
	if (hGateKeeperProcess) {
		CloseHandle(hGateKeeperProcess);
	}
}