#include <windows.h>
#include <string>

using namespace std;

void FunctionCaller();
void* GetAddressForCallHook(void* functionStart);
template<typename T>
DWORD ProtectMemory(void* address, DWORD prot);
template<typename T>
T* PointMemory(void* address);
UCHAR* HookWithJump(void* hookAt, void* newFunc);
void* CreateTrampolineFunc();

DWORD WINAPI StartWork(LPVOID lpParam) {
	
	void* hookFunc = GetAddressForCallHook(FunctionCaller);
	void* trampoline = CreateTrampolineFunc();
	// void* trampoline = CreateTrampolineFunc;
	// MessageBoxA(NULL, to_string((uintptr_t)hookFunc).c_str(), "MemWars Framework", MB_OK | MB_TOPMOST);
	HookWithJump(hookFunc, trampoline);
	FunctionCaller();
    return 1;
}

DWORD FunctionToBeHooked(DWORD id) {
	MessageBoxA(NULL, "Original Func", "MemWars Framework", MB_OK | MB_TOPMOST);
	return 0;
}

void FunctionCaller() {
	FunctionToBeHooked(0xBEEF);
}

void* GetAddressForCallHook(void* functionStart) {
	DWORD oldProtection = ProtectMemory<BYTE[1000]>(functionStart, PAGE_EXECUTE_READ); // make sure memory is readable, just incase
	PBYTE mem = PointMemory<BYTE>(functionStart);

	void* ret = 0;
	for (int i = 0; i < 1000; i++) {
		if (mem[i] == 0xE8) {
			ret = (void*)((uintptr_t)functionStart + i);
			break;
		}
	}
	ProtectMemory<BYTE[1000]>(functionStart, oldProtection); // restore old memory protection
	return ret;
}

template<typename T>
T* PointMemory(void* address) {
	return ((T*)address);
}

template<typename T>
T ReadMemory(void* address) {
	return *((T*)address);
}

template<typename T>
void WriteMemory(void* address, T value) {
	*((T*)address) = value;
}

template<typename T>
DWORD ProtectMemory(void* address, DWORD prot) {
	DWORD oldProt;
	VirtualProtect(address, sizeof(T), prot, &oldProt);
	return oldProt;
}

UCHAR* HookWithJump(void* hookAt, void* newFunc) {
	void* newOffset = (void*)((uintptr_t)newFunc - (uintptr_t)hookAt - 5);
	DWORD oldProtection = ProtectMemory<BYTE[5]>(hookAt, PAGE_EXECUTE_READWRITE);
	UCHAR* originals = new UCHAR[5];
	for (UINT i = 0; i < 5; i++) {
		originals[i] = ReadMemory<UCHAR>((void*)((uintptr_t)hookAt + i));
	}

	WriteMemory<BYTE>(hookAt, 0xE9);
	WriteMemory<void*>((void*)((uintptr_t)hookAt + 1), newOffset);
	ProtectMemory<BYTE[5]>((void*)((uintptr_t)hookAt + 1), oldProtection);
	
	return originals;
}

void /*__stdcall*/ HookingFunc(DWORD id) {
	MessageBoxA(NULL, "Hooking Func", "MemWars Framework", MB_OK | MB_TOPMOST);
	// return FunctionToBeHooked;
}

void* CreateTrampolineFunc() {
	void* trampolineAddr = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	BYTE codeCave[] = {
		// 0x67, 0x48, 0x8B, 0x44, 0x24, 0x04,				// mov rax, [esp + 0x4]
		// 0x50,											// push rax
		0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,				// mov rax, [HookingFunc]
		0xFF, 0xD0,                         			// call rax
		// 0xFF, 0xE0										// jmp rax
		0xC3											// ret
	};
	*(DWORD64*)((PUCHAR)codeCave + 3) = (DWORD64)(ULONG_PTR)HookingFunc;
	CopyMemory(trampolineAddr, codeCave, sizeof(codeCave));

	return trampolineAddr;
}




BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD  fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			CreateThread(NULL, 0, &StartWork, NULL, 0, NULL); 
			break;
	}
	return TRUE;
}