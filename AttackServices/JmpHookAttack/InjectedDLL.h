
using namespace std;

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

void FunctionCaller();
void* GetAddressForCallHook(void* functionStart);
UCHAR* HookWithJump(void* hookAt, void* newFunc);
void UnhookJump(void* hookAt, PBYTE originals);
void* CreateTrampolineFunc();
DWORD FunctionToBeHooked(DWORD id);
void* HookingFunc();