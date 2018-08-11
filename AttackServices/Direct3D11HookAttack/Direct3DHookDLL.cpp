#include <windows.h>
#pragma warning (disable : 4005)
#include <d3d11.h>
#include <d3dx11.h>
#include "../../libs/PolyHook/PolyHook.hpp"

using namespace std;

shared_ptr<PLH::Detour> DetourD3D11Present(new PLH::Detour);

DWORD_PTR* pSwapChainVtable = NULL;
DWORD_PTR* pContextVTable = NULL;
DWORD_PTR* pDeviceVTable = NULL;

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;

BOOL unhooked = FALSE;


typedef HRESULT(__stdcall *D3D11Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
D3D11Present pD3D11Present = NULL;

HRESULT __stdcall HookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    // CreateFileA("direct3DConfirmationFile", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    lstrcatA(tempPath, "dllInjectionConfirmationFile");
    HANDLE h = CreateFileA(tempPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(h);
    DetourD3D11Present->UnHook();
    unhooked = TRUE;
    return pD3D11Present(pSwapChain, SyncInterval, Flags);
}

LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){ return DefWindowProc(hwnd, uMsg, wParam, lParam); }
DWORD WINAPI InitializeHook(LPVOID lpParam) {

    WNDCLASSEXA wc = {sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "WX", NULL};
	RegisterClassExA(&wc);
    HWND hWnd = CreateWindowA("WX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

    D3D_FEATURE_LEVEL requestedLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1};
    D3D_FEATURE_LEVEL obtainedLevel;

    IDXGISwapChain* pSwapChain;
    DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;
    UINT createFlags = 0;
    // create device to retrieve pointer to VTable
    D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		requestedLevels,
		sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&pSwapChain,
		&pDevice,
		&obtainedLevel,
        &pContext
    );

    pSwapChainVtable = (DWORD_PTR*)pSwapChain;
    pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

    pContextVTable = (DWORD_PTR*)pContext;
    pContextVTable = (DWORD_PTR*)pContextVTable[0];

	pDeviceVTable = (DWORD_PTR*)pDevice;
    pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];
    
    pD3D11Present = (D3D11Present)pSwapChainVtable[8];
    DetourD3D11Present->SetupHook((PBYTE)pSwapChainVtable[8], (PBYTE)HookD3D11Present);
    DetourD3D11Present->Hook();
    // pD3D11Present = DetourD3D11Present->GetOriginal<pD3D11Present>();
    while (!unhooked) {
        Sleep(1000); // wait for D3D11Present to be called
    }
    Sleep(1000);
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);

    return 1;
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved) {
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, &InitializeHook, hModule, 0, NULL); 
            break;
        }
    return TRUE;
}