#include <stdio.h>
#include <windows.h>
#include "memAnalyzer.h"

void findValueInProcessTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    BYTEARRAY testValue1;
    BYTEARRAY testValue2;
    BYTEARRAY testValue3;
    BYTEARRAY testValue4;
    BYTEARRAY testValue5;
    BYTEARRAY testValue6;
    BYTEARRAY testValue7;
    BYTEARRAY testValue8;
    intToByteArray(&testValue1, 133337);
    intToByteArray(&testValue2, 0xB00B);
    intToByteArray(&testValue3, 0xCFFE);
    strToByteArray(&testValue4, "smallStr");
    strToByteArray(&testValue5, "Can you find me too?");
    strToByteArray(&testValue6, "And me??");
    floatToByteArray(&testValue7, 1.375);
    doubleToByteArray(&testValue8, 312.76493);

    MEMPTRS matchingMemPtrs = {0};
    int lastSize = matchingMemPtrs.size;
    findValueInProcess(&testValue1, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("findValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    findValueInProcess(&testValue2, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("findValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    findValueInProcess(&testValue3, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("findValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    findValueInProcess(&testValue4, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("findValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    findValueInProcess(&testValue5, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("findValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    findValueInProcess(&testValue6, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("findValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    findValueInProcess(&testValue7, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("findValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    findValueInProcess(&testValue8, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("findValueInProcessTest() failed\n");
        goto Exit;
    }
    printf("findValueInProcessTest() success\n");
    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void readProcessMemoryAtPtrLocationTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    BYTEARRAY testValue;
    strToByteArray(&testValue, "smallStr"); // make sure that this value actually is inside memoryTestApp.exe
    MEMPTRS matchingMemPtrs = {0};
    int lastSize = matchingMemPtrs.size;
    findValueInProcess(&testValue, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("readProcessMemoryAtPtrLocationTest()::findValueInProcess() failed\n");
        goto Exit;
    }
    BYTEARRAY foundValue;
    BOOL success = readProcessMemoryAtPtrLocation(matchingMemPtrs.memPointerArray[0], testValue.size, process, &foundValue);
    if (!success) {
        printf("readProcessMemoryAtPtrLocationTest()::readProcessMemoryAtPtrLocation() failed\n");
        goto Exit;
    }
    // printf("%d, %d\n", foundValue.size, testValue.size);
    if (valueIsMatching(&testValue, &foundValue)) {
        printf("readProcessMemoryAtPtrLocationTest() success\n");
    } else {
        printf("readProcessMemoryAtPtrLocationTest() failed\n");
    }
    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void intToByteArrayTest() {
    int testVal = 1337;
    BYTEARRAY bArr1;
    memcpy(bArr1.values, &testVal, sizeof(int));
    bArr1.size = sizeof(int);
    BYTEARRAY bArr;
    intToByteArray(&bArr, testVal);
    if (valueIsMatching(&bArr, &bArr1)) {
        printf("intToByteArrayTest() success\n");
    } else {
        printf("intToByteArrayTest() failed\n");
    }
}

void floatToByteArrayTest() {
    float testVal = 2.859;
    BYTEARRAY bArr1 = {0};
    bArr1.size = sizeof(testVal);
    memcpy(bArr1.values, &testVal, sizeof(testVal));
    BYTEARRAY bArr = {0};
    floatToByteArray(&bArr, testVal);
    if (valueIsMatching(&bArr, &bArr1)) {
        printf("floatToByteArrayTest() success\n");
    } else {
        printf("floatToByteArrayTest() failed\n");
    }
}

void doubleToByteArrayTest() {
    double testVal = 23.8591;
    BYTEARRAY bArr1 = {0};
    bArr1.size = sizeof(testVal);
    memcpy(bArr1.values, &testVal, sizeof(testVal));
    BYTEARRAY bArr = {0};
    doubleToByteArray(&bArr, testVal);
    if (valueIsMatching(&bArr, &bArr1)) {
        printf("doubleToByteArrayTest() success\n");
    } else {
        printf("doubleToByteArrayTest() failed\n");
    }
}


void strToByteArrayTest() {
    const char* testString = "Test123";
    BYTEARRAY bArr1;
    memcpy(bArr1.values, testString, strlen("Test123"));
    bArr1.size = strlen("Test123");
    BYTEARRAY bArr;
    strToByteArray(&bArr, testString);
    if (valueIsMatching(&bArr, &bArr1) && bArr.size == strlen("Test123")) {
        printf("strToByteArrayTest() success\n");
    } else {
        printf("strToByteArrayTest() failed\n");
    }
}

void valueIsMatchingTest() {
    
    BYTEARRAY bArr;
    bArr.size = 4;
    bArr.values[0] = 0xA;
    bArr.values[1] = 0xB;
    bArr.values[2] = 0x0;
    bArr.values[3] = 0x4;

    BYTEARRAY bArr1;
    bArr1.size = 4;
    bArr1.values[0] = 0xA;
    bArr1.values[1] = 0xB;
    bArr1.values[2] = 0x0;
    bArr1.values[3] = 0x4;

    BYTEARRAY bArr2;
    bArr2.size = 4;
    bArr2.values[0] = 0x5;
    bArr2.values[1] = 0x1;
    bArr2.values[2] = 0x3;
    bArr2.values[3] = 0x4;

    if (valueIsMatching(&bArr, &bArr1) == TRUE && valueIsMatching(&bArr, &bArr2) == FALSE) {
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
        if (*(int*)*(memPtrs.memPointerArray + i) != 16) {
            printf("reallocMemPtrsTest() failed\n");
            return;
        }
    }
    if (memPtrs.size == 101) {
        printf("reallocMemPtrsTest() success\n");
    } else {
        printf("reallocMemPtrsTest() failed\n");
    }
}

void reallocMemoryMapTest() {
    MEMMAP memMap = {0};
    BYTEARRAY bArr;
    bArr.size = 4;
    bArr.values[0] = 0xA;
    bArr.values[1] = 0xB;
    bArr.values[2] = 0x0;
    bArr.values[3] = 0x4;
    int testVal = 16;
    for (int i = 0; i < 101; i++) {
        concatMemoryMap(&memMap, &testVal, &bArr);
    }
    for (int i = 0; i < memMap.size; i++) {
        if (!valueIsMatching(memMap.byteArrays[i], &bArr) || 
            memMap.memPtrs->memPointerArray[i] != &testVal) {
            printf("reallocMemoryMapTest() failed\n");
            return;
        }
    }
    
    if (memMap.size == 101 && memMap.memPtrs->size == 101) {
        printf("reallocMemoryMapTest() success\n");
    }
    freeMemMap(&memMap);
}
void concatMemoryMapTest() {

    MEMMAP memMap = {0};

    BYTEARRAY bArr;
    bArr.size = 4;
    bArr.values[0] = 0xA;
    bArr.values[1] = 0xB;
    bArr.values[2] = 0x0;
    bArr.values[3] = 0x4;

    BYTEARRAY bArr1;
    bArr1.size = 4;
    bArr1.values[0] = 0xA;
    bArr1.values[1] = 0xB;
    bArr1.values[2] = 0x0;
    bArr1.values[3] = 0x4;

    BYTEARRAY bArr2;
    bArr2.size = 4;
    bArr2.values[0] = 0x5;
    bArr2.values[1] = 0x1;
    bArr2.values[2] = 0x3;
    bArr2.values[3] = 0x4;

    int testVal1 = 1;
    int testVal2 = 5;
    int testVal3 = 50;

    concatMemoryMap(&memMap, &testVal1, &bArr);
    concatMemoryMap(&memMap, &testVal2, &bArr1);
    concatMemoryMap(&memMap, &testVal3, &bArr2);

    memMap.byteArrays[2]->values[0] = 0x1;

    if (valueIsMatching(memMap.byteArrays[0], &bArr) &&
        valueIsMatching(memMap.byteArrays[1], &bArr1) &&
        !valueIsMatching(memMap.byteArrays[2], &bArr2) &&
        memMap.memPtrs->memPointerArray[0] == &testVal1 &&
        memMap.memPtrs->memPointerArray[1] == &testVal2 &&
        memMap.memPtrs->memPointerArray[2] == &testVal3 &&
        memMap.memPtrs->size == 3 &&
        memMap.size == 3) {
        
        printf("concatMemoryMapTest() success\n");
    } else {
        printf("concatMemoryMapTest() failed\n");
    }
    freeMemMap(&memMap);
}

void getProcessBaseAddressTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    HMODULE hMod = getProcessBaseAddress(process, "memoryTestApp.exe");

    if (hMod == NULL) {
        printf("getProcessBaseAddressTest() failed\n");
    } else {
        printf("getProcessBaseAddressTest() success\n");
    }
    system("taskkill /IM memoryTestApp.exe /F >nul");
}


int main() {
    // printProcessMemory("test.txt - Editor");
    // printProcessMemory("Buddy Liste");
    
    valueIsMatchingTest();
    concatMemPtrTest();
    reallocMemPtrsTest();
    intToByteArrayTest();
    strToByteArrayTest();
    floatToByteArrayTest();
    doubleToByteArrayTest();
    findValueInProcessTest();
    readProcessMemoryAtPtrLocationTest();
    getProcessBaseAddressTest();
    reallocMemoryMapTest();
    concatMemoryMapTest();
}