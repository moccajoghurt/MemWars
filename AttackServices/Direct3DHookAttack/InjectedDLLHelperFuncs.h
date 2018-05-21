#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>

using namespace std;

template<typename T>
T ReadMemory(void* address);

template<typename T>
void WriteMemory(void* address, T value);

template<typename T>
DWORD ProtectMemory(void* address, DWORD prot);

UCHAR* HookWithJump(void* hookAt, void* newFunc);
void* GetVF(void* classInst, DWORD funcIndex);
void* LocateEndSceneAddress();
void* CreateTrampolineFunc();
