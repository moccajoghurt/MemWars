#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "InjectedDLLHelperFuncs.h"

using namespace std;

LPDIRECT3DDEVICE9 hookedDevice;
UCHAR* originalCodeBuf;

void* GetVF(void* classInst, DWORD funcIndex) {
	void* VFTable = ReadMemory<void*>(classInst);
	void* hookAddress = (void*)((uintptr_t)VFTable + funcIndex * sizeof(void*));
	return ReadMemory<void*>(hookAddress);
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

void* __stdcall CaptureDevice(LPDIRECT3DDEVICE9 device) {
	hookedDevice = device;
	UnhookJump(endSceneAddress, originalCodeBuf);
	return endSceneAddress;
}


void* CreateTrampolineFunc() {
	void* trampolineAddr = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	BYTE codeCave[] = {
		0x51,											// push rcx
		0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,				// mov rax, [CaptureDevice]
		0xFF, 0xD0,                         			// call rax
		0x59,											// pop rcx
		0xFF, 0xE0										// jmp rax
	};
	*(DWORD64*)((PUCHAR)codeCave + 3) = (DWORD64)(ULONG_PTR)CaptureDevice;
	CopyMemory(trampolineAddr, codeCave, sizeof(codeCave));

	return trampolineAddr;
}
