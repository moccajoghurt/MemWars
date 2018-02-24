#include <windows.h>
#include <stdio.h>

void printPlayerPos() {
    HWND windowHwnd = FindWindow(0, "World of Warcraft");
    if (windowHwnd == NULL) {
        printf("FindWindow() returned NULL: %d\n", GetLastError());
        return;
    }
    DWORD processId;
    DWORD thread = GetWindowThreadProcessId(windowHwnd, &processId);
    printf("process id: %d\n", processId);
    printf("thread id: %d\n", thread);
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (process == NULL) {
        printf("OpenProcess() returned NULL: %d\n", GetLastError());
        return;
    }
    MEMORY_BASIC_INFORMATION info;
    UCHAR* p = 0x00FF31FC;
    VirtualQueryEx(process, p, &info, sizeof(info));
    printf("BaseAddress:\t\t 0x%x\n", (UINT)info.BaseAddress);
    printf("AllocationBase:\t\t 0x%x\n", (UINT)info.AllocationBase);
    printf("RegionSize:\t\t %u\n", info.RegionSize);
    printf("State:\t\t\t %d\n", info.State);
    printf("Protect:\t\t %x\n", info.Protect);
    printf("Type:\t\t\t %d\n", info.Type);
}