#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "InjectedDLLHelperFuncs.h"

using namespace std;

template<typename T>
T ReadMemory(void* address) {
	return *((T*)address);
}

template<typename T>
void WriteMemory(void* address, T value) {
	*((T*)address) = value;
}

void* GetVF(void* classInst, DWORD funcIndex) {
	void* VFTable = ReadMemory<void*>(classInst);
	void* hookAddress = (void*)((uintptr_t)VFTable + funcIndex * sizeof(void*));
	return ReadMemory<void*>(hookAddress);
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

// Locate the EndScene()-address inside the VF-table of our own direct3d-device.
void* LocateEndSceneAddress() {
	WNDCLASSEXA wc = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		DefWindowProc,
		0L,0L,
		GetModuleHandleA(NULL),
		NULL, NULL, NULL, NULL,
		"DX", NULL
	};

    RegisterClassExA(&wc);
    HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 600, 600, GetDesktopWindow(), NULL, wc.hInstance, NULL);
   
	LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D) {
		return 0;
	}

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	
    LPDIRECT3DDEVICE9 pd3dDevice;
	HRESULT res = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pd3dDevice);
	if (FAILED(res)) {
		return 0;
	}

	void* endSceneAddress = GetVF((void*)pd3dDevice, 42); //42 is the offset for the EndScene()-address

	pD3D->Release();
	pd3dDevice->Release();
    DestroyWindow(hWnd);

	return endSceneAddress;
}

void* CreateTrampolineFunc() {

	void* trampolineAddr = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	BYTE codeCave[] = {
		0x67, 0x48, 0x8B, 0x44, 0x24, 0x04,				// mov rax [esp + 0x4]
	};

}