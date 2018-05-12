#include "ThreadHijack.h"

void ThreadHijack(HANDLE process, LPVOID func, int times, const char* string) {
	BYTE codeCave[31] = {
		0x60, //PUSHAD
		0x9C, //PUSHFD
		0x68, 0x00, 0x00, 0x00, 0x00, // PUSH 0
		0x68, 0x00, 0x00, 0x00, 0x00, // PUSH 0
		0xB8, 0x00, 0x00, 0x00, 0x00, // MOV EAX, 0x0
		0xFF, 0xD0, // CALL EAX
		0x83, 0xC4, 0x08, // ADD ESP, 0x08
		0x9D, //POPFD
		0x61, //POPAD
		0x68, 0x00, 0x00, 0x00, 0x00, // PUSH 0
		0xC3 // RETN
	};

	// allocate memory for the code cave
	int stringlen = strlen(string) + 1;
	int fulllen = stringlen + sizeof(codeCave);
	LPVOID remoteMemory = VirtualAllocEx(process, NULL, fulllen, MEM_COMMIT, PAGE_EXECUTE);
	LPVOID remoteCave = (LPVOID)((DWORD)remoteMemory + stringlen);

	// suspend the thread and query its control context
	DWORD threadID = GetProcessThreadID(process);
	HANDLE thread = OpenThread((THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_SET_CONTEXT), false, threadID);
	SuspendThread(thread);

	CONTEXT threadContext;
	threadContext.ContextFlags = CONTEXT_CONTROL;
	GetThreadContext(thread, &threadContext);

	// copy values to the shellcode (happens late because we need values from allocation)
	memcpy(&codeCave[3], &remoteMemory, 4);
	memcpy(&codeCave[8], &times, 4);
	memcpy(&codeCave[13], &func, 4);
	memcpy(&codeCave[25], &threadContext.Eip, 4);


	// write the code cave
	WriteProcessMemory(process, remoteMemory, string, stringlen, NULL);
	WriteProcessMemory(process, remoteCave, codeCave, sizeof(codeCave), NULL);


	//hijack the thread
	threadContext.Eip = (DWORD)remoteCave;
	threadContext.ContextFlags = CONTEXT_CONTROL;
	SetThreadContext(thread, &threadContext);
	ResumeThread(thread);

	//clean
	CloseHandle(thread);
}