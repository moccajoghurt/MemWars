#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include "../../libs/PolyHook/PolyHook.hpp"

using namespace std;

void CreateConfirmationFile();
HANDLE hMutex;
shared_ptr<PLH::Detour> Detour_Send(new PLH::Detour);
shared_ptr<PLH::Detour> Detour_SendTo(new PLH::Detour);
shared_ptr<PLH::Detour> Detour_WSASend(new PLH::Detour);
shared_ptr<PLH::Detour> Detour_WSASendTo(new PLH::Detour);
shared_ptr<PLH::Detour> Detour_WSASendMsg(new PLH::Detour);

typedef int (__stdcall* tSend)(SOCKET s, const char *buf, int len, int flags);
tSend oSend = send;

typedef int (__stdcall* tSendto)(SOCKET s, const char *buf, int len, int flags, const sockaddr *to, int tolen);
tSendto oSendto = sendto;

typedef int (__stdcall* tWSASend)(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesSent,
    DWORD                              dwFlags,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);
tWSASend oWSASend = WSASend;

typedef int (__stdcall* tWSASendTo)(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesSent,
    DWORD                              dwFlags,
    const sockaddr                     *lpTo,
    int                                iTolen,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);
tWSASendTo oWSASendTo = WSASendTo;

typedef int (__stdcall* tWSASendMsg)(
    SOCKET                             Handle,
    LPWSAMSG                           lpMsg,
    DWORD                              dwFlags,
    LPDWORD                            lpNumberOfBytesSent,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);
tWSASendMsg oWSASendMsg = WSASendMsg;

int __stdcall hSend(SOCKET s, const char *buf, int len, int flags) {
    // MessageBoxA(NULL, "send called!\n", "MemWars Framework", MB_OK | MB_TOPMOST);
    CreateConfirmationFile();
    WSASetLastError(WSAEINPROGRESS);
    return SOCKET_ERROR;
    // return oSend(s, buf, len, flags);
}

int __stdcall hSendto(SOCKET s, const char *buf, int len, int flags, const sockaddr *to, int tolen) {
    // MessageBoxA(NULL, "sendto called!\n", "MemWars Framework", MB_OK | MB_TOPMOST);
    CreateConfirmationFile();
    WSASetLastError(WSAEINPROGRESS);
    return SOCKET_ERROR;
    // return oSendto(s, buf, len, flags, to, tolen);
}

int __stdcall hWSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    // MessageBoxA(NULL, "WSASend called!\n", "MemWars Framework", MB_OK | MB_TOPMOST);
    CreateConfirmationFile();
    WSASetLastError(WSAEINPROGRESS);
    return SOCKET_ERROR;
    // return oWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

int __stdcall hWSASendTo(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const sockaddr *lpTo, int iTolen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    // MessageBoxA(NULL, "WSASendTo called!\n", "MemWars Framework", MB_OK | MB_TOPMOST);
    CreateConfirmationFile();
    WSASetLastError(WSAEINPROGRESS);
    return SOCKET_ERROR;
    // return oWSASendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine);
}

int __stdcall hWSASendMsg(SOCKET Handle, LPWSAMSG lpMsg, DWORD dwFlags, LPDWORD lpNumberOfBytesSent, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    // MessageBoxA(NULL, "WSASendMsg called!\n", "MemWars Framework", MB_OK | MB_TOPMOST);
    CreateConfirmationFile();
    WSASetLastError(WSAEINPROGRESS);
    return SOCKET_ERROR;
    // return oWSASendMsg(Handle, lpMsg, dwFlags, lpNumberOfBytesSent, lpOverlapped, lpCompletionRoutine);
}

BOOL unhook = FALSE;
void CreateConfirmationFile() {
    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);
    lstrcatA(tempPath, "dllInjectionConfirmationFile");
    HANDLE h = CreateFileA(tempPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(h);
    unhook = TRUE;
}

DWORD WINAPI InitializeHook(LPVOID lpParam) {

    Detour_Send->SetupHook((BYTE*)&send,(BYTE*) &hSend);
    Detour_Send->Hook();
    oSend = Detour_Send->GetOriginal<tSend>();

    Detour_SendTo->SetupHook((BYTE*)&sendto,(BYTE*) &hSendto);
    Detour_SendTo->Hook();
    oSendto = Detour_SendTo->GetOriginal<tSendto>();

    Detour_WSASend->SetupHook((BYTE*)&WSASend,(BYTE*) &hWSASend);
    Detour_WSASend->Hook();
    oWSASend = Detour_WSASend->GetOriginal<tWSASend>();

    Detour_WSASendTo->SetupHook((BYTE*)&WSASendTo,(BYTE*) &hWSASendTo);
    Detour_WSASendTo->Hook();
    oWSASendTo = Detour_WSASendTo->GetOriginal<tWSASendTo>();

    Detour_WSASendMsg->SetupHook((BYTE*)&WSASendMsg,(BYTE*) &hWSASendMsg);
    Detour_WSASendMsg->Hook();
    oWSASendMsg = Detour_WSASendMsg->GetOriginal<tWSASendMsg>();

    while (!unhook) {
        Sleep(1000); // wait for a socket function to be called
    }
    Sleep(1000);
    Detour_Send->UnHook();
    Detour_SendTo->UnHook();
    Detour_WSASend->UnHook();
    Detour_WSASendTo->UnHook();
    Detour_WSASendMsg->UnHook();
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