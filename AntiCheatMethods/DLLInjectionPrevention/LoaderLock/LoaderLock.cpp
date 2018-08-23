/*
    This program uses LdrLockLoaderLock to prevent the loading of any libraries, which grants immunity to any malicious DLL injections.
    While this method is very effective, it also prevents any friendly DLL to be loaded, which is unpractical in game development.
*/

#include <windows.h>
#include <iostream>

#define LDR_LOCK_LOADER_LOCK_FLAG_DEFAULT 0x00000000
#define STATUS_SUCCESS 0

using namespace std;

using fnFreeCall = uint64_t(__fastcall*)(...);

template<typename ...Params>
static NTSTATUS __NtRoutine(const char* Name, Params &&... params) {
	auto fn = (fnFreeCall) GetProcAddress(GetModuleHandleA("ntdll.dll"), Name);
	return fn(std::forward<Params>(params)...);
}

#define LdrLockLoaderLock(...) __NtRoutine("LdrLockLoaderLock", __VA_ARGS__)

int main() {

    ULONG_PTR m_uCookie = NULL;
    ULONG uState = NULL;
    NTSTATUS ntStatus = LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_DEFAULT, &uState, &m_uCookie);
    if (ntStatus == STATUS_SUCCESS) {
        cout << "success, try to inject me" << endl;
    } else {
        cout << "failed" << endl;
        return 1;
    }

    for (;;) {
        // try to inject me
        TlsGetValue(0);
        Sleep(10);
    }
}