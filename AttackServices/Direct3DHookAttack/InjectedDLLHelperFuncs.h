#pragma once

#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>

using namespace std;


extern LPDIRECT3DDEVICE9 hookedDevice;
extern void* endSceneAddress;
extern UCHAR* overwrittenBytes;



template<typename T>
T ReadMemory(void* address);

template<typename T>
void WriteMemory(void* address, T value);

template<typename T>
DWORD ProtectMemory(void* address, DWORD prot);

UCHAR* HookWithJump(void* hookAt, void* newFunc);
void UnhookWithJump(void* hookAt, UCHAR* originals);
void* GetVF(void* classInst, DWORD funcIndex);
void* LocateEndSceneAddress();
void* CreateTrampolineFunc();
void* CaptureDevice(LPDIRECT3DDEVICE9);
