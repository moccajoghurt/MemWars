#include <stdio.h>
#include <windows.h>
#include "memAnalyzer.h"

void findValueByProcessTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    BYTEARRAY testValue1;
    BYTEARRAY testValue2;
    BYTEARRAY testValue3;
    BYTEARRAY testValue4;
    intToByteArray(&testValue1, 133337);
    intToByteArray(&testValue2, 0xB00B);
    // intToByteArray(0xC0FE, &testValue3);
    // floatToByteArray(1.375, &testValue4);

    MEMPTRS matchingMemPtrs = {0};
    findValueByProcess(&testValue1, process, &matchingMemPtrs);
    findValueByProcess(&testValue2, process, &matchingMemPtrs);
    // findValueByProcess(&testValue3, process, &matchingMemPtrs);
    // findValueByProcess(&testValue4, process, &matchingMemPtrs);
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void intToByteArrayTest() {
    int testVal = 1337;
    BYTEARRAY bArr;
    intToByteArray(&bArr, testVal);
    if (valueIsMatching(&testVal, &bArr)) {
        printf("intToByteArrayTest() success\n");
    } else {
        printf("intToByteArrayTest() failed\n");
    }
}

void floatToByteArrayTest() {
    float testVal = 2.859;
    BYTEARRAY bArr = {0};
    floatToByteArray(&bArr, testVal);
    if (valueIsMatching(&testVal, &bArr)) {
        printf("floatToByteArrayTest() success\n");
    } else {
        printf("floatToByteArrayTest() failed\n");
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

void concatMemPtrTest() {
    MEMPTRS memPtrs = {0};
    int testVal1 = 4;
    float testVal2 = 5;
    DWORD testVal3 = 7;
    double testVal4 = 1;
    BYTE testVal5 = 16;
    concatMemPtr(&testVal1, &memPtrs);
    concatMemPtr(&testVal2, &memPtrs);
    concatMemPtr(&testVal3, &memPtrs);
    concatMemPtr(&testVal4, &memPtrs);
    concatMemPtr((int*)&testVal5, &memPtrs);
    if (memPtrs.memPointerArray[0] == (BYTE*)&testVal1 &&
        memPtrs.memPointerArray[1] == (BYTE*)&testVal2 &&
        memPtrs.memPointerArray[2] == (BYTE*)&testVal3 &&
        memPtrs.memPointerArray[3] == (BYTE*)&testVal4 &&
        memPtrs.memPointerArray[4] == (BYTE*)&testVal5 &&
        memPtrs.size == 5) {
        printf("concatMemPtrTest() success\n");
    } else {
        printf("concatMemPtrTest() failed\n");
    }
}

void reallocMemPtrsTest() {
    int testVal = 16;
    MEMPTRS memPtrs = {0};
    for (int i = 0; i < 101; i++) {
        concatMemPtr(&testVal, &memPtrs);
    }
    for (int i = 0; i < memPtrs.size; i++) {
        if (*memPtrs.memPointerArray[i] != 16) {
            printf("reallocMemPtrsTest() failed\n");
            return;
        }
    }
    printf("reallocMemPtrsTest() success\n");
}

int main() {
    // printProcessMemory("test.txt - Editor");
    // printProcessMemory("Warcraft III");
    
    valueIsMatchingTest();
    intToByteArrayTest();
    concatMemPtrTest();
    reallocMemPtrsTest();
    // floatToByteArrayTest();
    findValueByProcessTest();
}