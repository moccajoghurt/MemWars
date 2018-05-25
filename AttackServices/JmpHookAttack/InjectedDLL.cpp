#include <windows.h>
#include "InjectedDLL.h"

using namespace std;

PBYTE orgCode;
void* hookFunc;


DWORD FunctionToBeHooked(DWORD id) {
	if (id != 0xBEEF) {
		DeleteFile("jmpHookConfirmationFile");
	}
	return 0;
}

void FunctionCaller() {
	FunctionToBeHooked(0xBEEF);
}

void* GetAddressForCallHook(void* functionStart) {
	DWORD oldProtection = ProtectMemory<BYTE[1000]>(functionStart, PAGE_EXECUTE_READ);
	PBYTE mem = PointMemory<BYTE>(functionStart);

	void* ret = 0;
	for (int i = 0; i < 1000; i++) {
		if (mem[i] == 0xE8) {
			ret = (void*)((uintptr_t)functionStart + i);
			break;
		}
	}
	ProtectMemory<BYTE[1000]>(functionStart, oldProtection);
	return ret;
}

UCHAR* HookWithJump(void* hookAt, void* newFunc) {
	DWORD oldProtection = ProtectMemory<BYTE[12]>(hookAt, PAGE_EXECUTE_READWRITE);
	PBYTE originalCodeBuf = new BYTE[12];
	CopyMemory(originalCodeBuf, hookAt, sizeof(BYTE) * 12);

	BYTE codeCave[] = {
		0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,		// mov rax
		0xFF, 0xE0								// jmp rax
	};
	*(DWORD64*)((PUCHAR)codeCave + 2) = (DWORD64)(ULONG_PTR)newFunc;

	CopyMemory(hookAt, codeCave, sizeof(codeCave));
	ProtectMemory<BYTE[12]>(hookAt, oldProtection);
	
	return originalCodeBuf;
}

void UnhookJump(void* hookAt, PBYTE originalCodeBuf) {
	DWORD oldProtection = ProtectMemory<BYTE[12]>(hookAt, PAGE_EXECUTE_READWRITE);
	CopyMemory(hookAt, originalCodeBuf, sizeof(BYTE) * 12);
	ProtectMemory<BYTE[12]>(hookAt, oldProtection);
	delete [] originalCodeBuf;
}

void* HookingFunc() {
	MessageBoxA(NULL, "Hooking Func", "MemWars Framework", MB_OK | MB_TOPMOST);
	CreateFileA("jmpHookConfirmationFile", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	UnhookJump(hookFunc, orgCode);
	return hookFunc;
}

void* CreateTrampolineFunc() {
	void* trampolineAddr = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	BYTE codeCave[] = {
		0x51,											// push rcx
		0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,				// mov rax, [HookingFunc]
		0xFF, 0xD0,                         			// call rax
		0x59,											// pop rcx
		0xFF, 0xE0										// jmp rax
	};
	*(DWORD64*)((PUCHAR)codeCave + 3) = (DWORD64)(ULONG_PTR)HookingFunc;
	CopyMemory(trampolineAddr, codeCave, sizeof(codeCave));

	return trampolineAddr;
}

DWORD WINAPI StartWork(LPVOID lpParam) {
	hookFunc = GetAddressForCallHook(FunctionCaller);
	void* trampoline = CreateTrampolineFunc();
	orgCode = HookWithJump(hookFunc, trampoline);
	FunctionCaller();
    return 1;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD  fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			CreateThread(NULL, 0, &StartWork, NULL, 0, NULL); 
			break;
	}
	return TRUE;
}