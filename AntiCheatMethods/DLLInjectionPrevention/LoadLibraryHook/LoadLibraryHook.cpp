#include <windows.h>
#include <iostream>
#include "../../../libs/PolyHook/PolyHook.hpp"

using namespace std;

typedef HMODULE(__stdcall* tLoadLibraryA)(LPCTSTR lpFileName);
tLoadLibraryA oLoadLibraryA;

typedef HMODULE(__stdcall* tLoadLibraryW)(LPWSTR lpFileName);
tLoadLibraryW oLoadLibraryW;

HMODULE __stdcall hLoadLibraryA(LPCTSTR lpFileName) {
    wcout << lpFileName << L" tried to be injected" << endl;
    if (strcmp(lpFileName, "allowed.dll") == 0) {
        return oLoadLibraryA(lpFileName);
    } else {
        cout << "invalid DLL detected" << endl;
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }
}

HMODULE __stdcall hLoadLibraryW(LPWSTR lpFileName) {
    wcout << lpFileName << L" tried to be injected" << endl;
    if (wcscmp(lpFileName, L"allowed.dll") == 0) {
        return oLoadLibraryW(lpFileName);
    } else {
        cout << "invalid DLL detected" << endl;
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }
}


int main() {
    shared_ptr<PLH::Detour> Detour_Ex(new PLH::Detour);
    Detour_Ex->SetupHook((BYTE*)&LoadLibraryA,(BYTE*) &hLoadLibraryA);
    Detour_Ex->Hook();
    oLoadLibraryA = Detour_Ex->GetOriginal<tLoadLibraryA>(); 

    Detour_Ex->SetupHook((BYTE*)&LoadLibraryW,(BYTE*) &hLoadLibraryW);
    Detour_Ex->Hook();
    oLoadLibraryW = Detour_Ex->GetOriginal<tLoadLibraryW>(); 

    // LoadLibraryA("InjectedDLL.dll");
    // LoadLibraryW(L"InjectedDLL.dll");
    for (;;) {
        // try to inject me
        TlsGetValue(0);
        Sleep(10);
    }
}