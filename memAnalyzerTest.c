#include <stdio.h>
#include <windows.h>
#include "memAnalyzer.h"

void findValueTest() {
    system("start /B memoryTestApp.exe");
    Sleep(1000);
    HWND windowHwnd = FindWindow(0, "manipulateMe");
    if (windowHwnd == NULL) {
        printf("FindWindow() returned NULL: %d\n", GetLastError());
        return;
    }
    DWORD processId;
    DWORD thread = GetWindowThreadProcessId(windowHwnd, &processId);
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (process == NULL) {
        printf("OpenProcess() returned NULL: %d\n", GetLastError());
        return;
    }
    BYTEARRAY bArr;
    intToByteArray(13337, &bArr);

    MEMPTRS matchingMemPtrs;
    matchingMemPtrs.size = 0;
    findValue(&bArr, process, &matchingMemPtrs);
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void intToByteArrayTest() {
    int testVal = 1337;
    BYTEARRAY bArr;
    intToByteArray(testVal, &bArr);
    if (valueIsMatching(&testVal, &bArr)) {
        printf("intToByteArrayTest() success\n");
    } else {
        printf("intToByteArrayTest() failed\n");
    }
}

void valueIsMatchingTest() {
    CHAR testMemory1[] = {0xA, 0xB, 0x0, 0x4};
    CHAR testMemory2[] = {0xB, 0x0, 0x0, 0xB};
    BYTEARRAY bArr;
    bArr.size = 4;
    bArr.values[0] = 0xA;
    bArr.values[1] = 0xB;
    bArr.values[2] = 0x0;
    bArr.values[3] = 0x4;

    if (valueIsMatching(testMemory1, &bArr) == TRUE && valueIsMatching(testMemory2, &bArr) == FALSE) {
        printf("valueIsMatchingTest() success\n");
    } else {
        printf("valueIsMatchingTest() failed\n");
    }
}

int main() {
    // printProcessMemory("test.txt - Editor");
    // printProcessMemory("Warcraft III");
    
    valueIsMatchingTest();
    intToByteArrayTest();
    findValueTest();
}