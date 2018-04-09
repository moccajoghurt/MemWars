#include <windows.h>
#include "../MemWarsCore/MemWarsCore.h"


int main() {

    FARPROC addrMessageBoxA = GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "MessageBoxA");
    FARPROC addrExitThread = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "ExitThread");
    if (addrMessageBoxA == NULL || addrExitThread == NULL) {
    }
    BYTE shellcode[] = {
        0x48, 0x31, 0xC9,                           // xor    rcx,rcx
        0x48, 0x31, 0xD2,                           // xor    rdx,rdx
        0x4D, 0x31, 0xC0,                           // xor    r8,r8 (8)
        0x45, 0x31, 0xC9,                           // xor    r9d,r9d (11)
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,         // mov    rax,addr
        0xFF, 0xD0,                                 // call   rax
        0xEB, 0xFE                                  // nop + jmp rel8 -2
    };
    *(DWORD64*)((PUCHAR)shellcode + 14) = (DWORD64)(ULONG_PTR)addrExitThread;

    int (*ret)() = (int(*)())shellcode;

    ret();
}