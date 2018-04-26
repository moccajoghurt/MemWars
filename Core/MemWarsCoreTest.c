#include <stdio.h>
#include <windows.h>
#include "MemWarsCore.h"

void FindValueInProcessTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    BYTEARRAY testValue1;
    BYTEARRAY testValue2;
    BYTEARRAY testValue3;
    BYTEARRAY testValue4;
    BYTEARRAY testValue5;
    BYTEARRAY testValue6;
    BYTEARRAY testValue7;
    BYTEARRAY testValue8;
    IntToByteArray(&testValue1, 133337);
    IntToByteArray(&testValue2, 0xB00B);
    IntToByteArray(&testValue3, 0xCFFE);
    StrToByteArray(&testValue4, "smallStr");
    StrToByteArray(&testValue5, "Can you find me too?");
    StrToByteArray(&testValue6, "And me??");
    FloatToByteArray(&testValue7, 1.375);
    DoubleToByteArray(&testValue8, 312.76493);

    MEMPTRS matchingMemPtrs = {0};
    int lastSize = matchingMemPtrs.size;
    FindValueInProcess(&testValue1, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("FindValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    FindValueInProcess(&testValue2, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("FindValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    FindValueInProcess(&testValue3, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("FindValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    FindValueInProcess(&testValue4, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("FindValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    FindValueInProcess(&testValue5, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("FindValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    FindValueInProcess(&testValue6, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("FindValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    FindValueInProcess(&testValue7, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("FindValueInProcessTest() failed\n");
        goto Exit;
    }
    lastSize = matchingMemPtrs.size;
    FindValueInProcess(&testValue8, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("FindValueInProcessTest() failed\n");
        goto Exit;
    }
    printf("FindValueInProcessTest() success\n");
    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void ReadProcessMemoryAtPtrLocationTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    BYTEARRAY testValue;
    StrToByteArray(&testValue, "smallStr"); // make sure that this value actually is inside memoryTestApp.exe
    MEMPTRS matchingMemPtrs = {0};
    int lastSize = matchingMemPtrs.size;
    FindValueInProcess(&testValue, process, &matchingMemPtrs);
    if (lastSize >= matchingMemPtrs.size) {
        printf("ReadProcessMemoryAtPtrLocationTest()::FindValueInProcess() failed\n");
        goto Exit;
    }
    BYTEARRAY foundValue;
    BOOL success = ReadProcessMemoryAtPtrLocation(matchingMemPtrs.memPointerArray[0], testValue.size, process, &foundValue);
    if (!success) {
        printf("ReadProcessMemoryAtPtrLocationTest()::ReadProcessMemoryAtPtrLocation() failed\n");
        goto Exit;
    }
    // printf("%d, %d\n", foundValue.size, testValue.size);
    if (ValueIsMatching(&testValue, &foundValue)) {
        printf("ReadProcessMemoryAtPtrLocationTest() success\n");
    } else {
        printf("ReadProcessMemoryAtPtrLocationTest() failed\n");
    }
    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void IntToByteArrayTest() {
    int testVal = 1337;
    BYTEARRAY bArr1;
    memcpy(bArr1.values, &testVal, sizeof(int));
    bArr1.size = sizeof(int);
    BYTEARRAY bArr;
    IntToByteArray(&bArr, testVal);
    if (ValueIsMatching(&bArr, &bArr1)) {
        printf("IntToByteArrayTest() success\n");
    } else {
        printf("IntToByteArrayTest() failed\n");
    }
}

void ShortToByteArrayTest() {
    short testVal = 137;
    BYTEARRAY bArr1;
    memcpy(bArr1.values, &testVal, sizeof(short));
    bArr1.size = sizeof(short);
    BYTEARRAY bArr;
    ShortToByteArray(&bArr, testVal);
    if (ValueIsMatching(&bArr, &bArr1)) {
        printf("ShortToByteArrayTest() success\n");
    } else {
        printf("ShortToByteArrayTest() failed\n");
    }
}

void ByteToByteArrayTest() {
    char testVal = 255;
    BYTEARRAY bArr1;
    memcpy(bArr1.values, &testVal, sizeof(char));
    bArr1.size = sizeof(char);
    BYTEARRAY bArr;
    ByteToByteArray(&bArr, testVal);
    if (ValueIsMatching(&bArr, &bArr1)) {
        printf("ByteToByteArrayTest() success\n");
    } else {
        printf("ByteToByteArrayTest() failed\n");
    }
}

void FloatToByteArrayTest() {
    float testVal = 2.859;
    BYTEARRAY bArr1 = {0};
    bArr1.size = sizeof(testVal);
    memcpy(bArr1.values, &testVal, sizeof(testVal));
    BYTEARRAY bArr = {0};
    FloatToByteArray(&bArr, testVal);
    if (ValueIsMatching(&bArr, &bArr1)) {
        printf("FloatToByteArrayTest() success\n");
    } else {
        printf("FloatToByteArrayTest() failed\n");
    }
}

void DoubleToByteArrayTest() {
    double testVal = 23.8591;
    BYTEARRAY bArr1 = {0};
    bArr1.size = sizeof(testVal);
    memcpy(bArr1.values, &testVal, sizeof(testVal));
    BYTEARRAY bArr = {0};
    DoubleToByteArray(&bArr, testVal);
    if (ValueIsMatching(&bArr, &bArr1)) {
        printf("DoubleToByteArrayTest() success\n");
    } else {
        printf("DoubleToByteArrayTest() failed\n");
    }
}


void StrToByteArrayTest() {
    const char* testString = "Test123";
    BYTEARRAY bArr1;
    memcpy(bArr1.values, testString, strlen("Test123"));
    bArr1.size = strlen("Test123");
    BYTEARRAY bArr;
    StrToByteArray(&bArr, testString);
    if (ValueIsMatching(&bArr, &bArr1) && bArr.size == strlen("Test123")) {
        printf("StrToByteArrayTest() success\n");
    } else {
        printf("StrToByteArrayTest() failed\n");
    }
}

void BytesToByteArrayTest() {
    BYTE arr[] = {0x0D, 0xE0, 0x0A, 0xD0, 0x0B, 0xE0, 0x0E, 0xF0};
    SIZE_T arrLen = 8;
    BYTEARRAY bArr1;
    memcpy(bArr1.values, arr, arrLen);
    bArr1.size = arrLen;
    BYTEARRAY bArr;
    BytesToByteArray(&bArr, arr, arrLen);
    if (ValueIsMatching(&bArr, &bArr1) && bArr.size == arrLen) {
        printf("BytesToByteArrayTest() success\n");
    } else {
        printf("BytesToByteArrayTest() failed\n");
    }
}



void ValueIsMatchingTest() {
    
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

    if (ValueIsMatching(&bArr, &bArr1) == TRUE && ValueIsMatching(&bArr, &bArr2) == FALSE) {
        printf("ValueIsMatchingTest() success\n");
    } else {
        printf("ValueIsMatchingTest() failed\n");
    }
}

void ConcatMemPtrTest() {
    MEMPTRS memPtrs = {0};
    int testVal1 = 4;
    float testVal2 = 5;
    DWORD testVal3 = 7;
    double testVal4 = 1;
    BYTE testVal5 = 16;
    ConcatMemPtr(&testVal1, &memPtrs);
    ConcatMemPtr(&testVal2, &memPtrs);
    ConcatMemPtr(&testVal3, &memPtrs);
    ConcatMemPtr(&testVal4, &memPtrs);
    ConcatMemPtr((int*)&testVal5, &memPtrs);
    if (memPtrs.memPointerArray[0] == (BYTE*)&testVal1 &&
        memPtrs.memPointerArray[1] == (BYTE*)&testVal2 &&
        memPtrs.memPointerArray[2] == (BYTE*)&testVal3 &&
        memPtrs.memPointerArray[3] == (BYTE*)&testVal4 &&
        memPtrs.memPointerArray[4] == (BYTE*)&testVal5 &&
        memPtrs.size == 5) {
        printf("ConcatMemPtrTest() success\n");
    } else {
        printf("ConcatMemPtrTest() failed\n");
    }
}

void ReallocMemPtrsTest() {
    int testVal = 16;
    MEMPTRS memPtrs = {0};
    for (int i = 0; i < 101; i++) {
        ConcatMemPtr(&testVal, &memPtrs);
    }
    for (int i = 0; i < memPtrs.size; i++) {
        if (*(int*)*(memPtrs.memPointerArray + i) != 16) {
            printf("ReallocMemPtrsTest() failed\n");
            return;
        }
    }
    if (memPtrs.size == 101) {
        printf("ReallocMemPtrsTest() success\n");
    } else {
        printf("ReallocMemPtrsTest() failed\n");
    }
}

void ReallocMemoryMapTest() {
    MEMMAP memMap = {0};
    BYTEARRAY bArr;
    bArr.size = 4;
    bArr.values[0] = 0xA;
    bArr.values[1] = 0xB;
    bArr.values[2] = 0x0;
    bArr.values[3] = 0x4;
    int testVal = 16;
    for (int i = 0; i < 101; i++) {
        ConcatMemoryMap(&memMap, &testVal, &bArr);
    }
    for (int i = 0; i < memMap.size; i++) {
        if (!ValueIsMatching(memMap.byteArrays[i], &bArr) || 
            memMap.memPtrs->memPointerArray[i] != &testVal) {
            printf("ReallocMemoryMapTest() failed\n");
            return;
        }
    }
    
    if (memMap.size == 101 && memMap.memPtrs->size == 101) {
        printf("ReallocMemoryMapTest() success\n");
    } else {
        printf("ReallocMemoryMapTest() failed\n");
    }
    FreeMemMap(&memMap);
}

void ConcatMemoryMapTest() {
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
    ConcatMemoryMap(&memMap, &testVal1, &bArr);
    ConcatMemoryMap(&memMap, &testVal2, &bArr1);
    ConcatMemoryMap(&memMap, &testVal3, &bArr2);
    memMap.byteArrays[2]->values[0] = 0x1;
    if (ValueIsMatching(memMap.byteArrays[0], &bArr) &&
        ValueIsMatching(memMap.byteArrays[1], &bArr1) &&
        !ValueIsMatching(memMap.byteArrays[2], &bArr2) &&
        memMap.memPtrs->memPointerArray[0] == &testVal1 &&
        memMap.memPtrs->memPointerArray[1] == &testVal2 &&
        memMap.memPtrs->memPointerArray[2] == &testVal3 &&
        memMap.memPtrs->size == 3 &&
        memMap.size == 3) {
        
        printf("ConcatMemoryMapTest() success\n");
    } else {
        printf("ConcatMemoryMapTest() failed\n");
    }
    FreeMemMap(&memMap);
}

void getProcessBaseAddressTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    HMODULE hMod = GetProcessBaseAddress(process);

    if (hMod == NULL) {
        printf("getProcessBaseAddressTest() failed\n");
    } else {
        printf("getProcessBaseAddressTest() success\n");
    }
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void memorySnapshotMemCountMatchesPtrCountTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
        // process = (HANDLE)GetProcessByName("ac_client.exe");
    }
    MemorySnapshotToDisc(process, "buf.txt");
    FILE* file1 = fopen("buf.txt", "rb");
    if (file1 == NULL) {
        printf("Could not open buf.txt\n");
        printf("memorySnapshotMemCountMatchesPtrCountTest() failed\n");
        goto Exit;
    }
    

    FILE* file2 = fopen("buf.txt - ptrs", "rb");
    if (file2 == NULL) {
        printf("Could not open buf.txt - ptrs\n");
        printf("memorySnapshotMemCountMatchesPtrCountTest() failed\n");
        goto Exit;
    }
    
    int fileSize1;
    fseek(file1, 0 , SEEK_END);
    fileSize1 = ftell(file1);
    fseek(file1, 0, SEEK_SET);

    void* fileBuf1 = malloc(fileSize1 * 4);
    fread(fileBuf1, fileSize1, 1, file1);

    int fileSize2;
    fseek(file2, 0 , SEEK_END);
    fileSize2 = ftell(file2);
    fseek(file2, 0, SEEK_SET);

    void* fileBuf2 = malloc(fileSize2 * 4);
    fread(fileBuf2, fileSize2, 1, file2);

    if (fileSize1 == fileSize2/sizeof(void*)) {
        printf("memorySnapshotMemCountMatchesPtrCountTest() success\n");
    } else {
        printf("memorySnapshotMemCountMatchesPtrCountTest() failed\n");
    }

    fclose(file1);
    fclose(file2);
    Exit:
    system("del buf.txt");
    system("del \"buf.txt - ptrs\"");
    system("taskkill /IM memoryTestApp.exe /F >nul");
}


void memorySnapshotSavesCorrectValueAndPointerTest() {
    // this test makes a memory snapshot and tests if the saved pointers and
    // values correspond to the current pointers and values of the running program
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }
    MemorySnapshotToDisc(process, "buf.txt");

    MEMPTRS matchingPtrs = {0};
    BYTEARRAY testVal = {0};
    IntToByteArray(&testVal, 133337);
    FindValueInProcess(&testVal, process, &matchingPtrs);
    if (matchingPtrs.size == 0) {
        printf("memorySnapshotSavesCorrectValueAndPointerTest() failed. testVal not found\n");
        goto Exit;
    }

    FILE* file1 = fopen("buf.txt - ptrs", "rb");
    if (file1 == NULL) {
        printf("Could not open buf.txt - ptrs\n");
        printf("memorySnapshotSavesCorrectValueTest() failed\n");
        goto Exit;
    }

    FILE* file2 = fopen("buf.txt", "rb");
    if (file2 == NULL) {
        printf("Could not open buf.txt\n");
        printf("memorySnapshotSavesCorrectValueTest() failed\n");
        goto Exit;
    }

    int fileSize;
    fseek(file1, 0 , SEEK_END);
    fileSize = ftell(file1);
    fseek(file1, 0, SEEK_SET);

    void* fileBuf1 = malloc(fileSize * sizeof(void*));
    fread(fileBuf1, fileSize, 1, file1);

    int fileSize2;
    fseek(file2, 0 , SEEK_END);
    fileSize2 = ftell(file2);
    fseek(file2, 0, SEEK_SET);

    void* fileBuf2 = malloc(fileSize2 * sizeof(void*));
    fread(fileBuf2, fileSize2, 1, file2);

    BOOL foundValInMemoryBySnapshotPtr = FALSE;
    BOOL foundValOnDisc = FALSE;
    for (unsigned int i = 0; i < fileSize; i += sizeof(void*)) {
        if (i + sizeof(void*) >= fileSize) {
            break;
        }
        
        for (int n = 0; n < matchingPtrs.size; n++) {
            BYTEARRAY filebufVal = {0};
            BYTEARRAY memPtrVal = {0};
            if (sizeof(void*) == sizeof(DWORD64)) {
                memcpy(filebufVal.values, (DWORD64*)fileBuf1 + i, sizeof(void*));
                memcpy(memPtrVal.values, (DWORD64*)matchingPtrs.memPointerArray + n, sizeof(void*));
            } else if (sizeof(void*) == sizeof(DWORD)) {
                memcpy(filebufVal.values, (DWORD*)fileBuf1 + i, sizeof(void*));
                memcpy(memPtrVal.values, (DWORD*)matchingPtrs.memPointerArray + n, sizeof(void*));
            } else {
                printf("memorySnapshotSavesCorrectValueAndPointerTest() failed. unsupported platform\n");
                goto Exit;
            }
            filebufVal.size = sizeof(void*);
            memPtrVal.size = sizeof(void*);

            if (ValueIsMatching(&filebufVal, &memPtrVal)) {
                BYTEARRAY buf = {0};
                

                if (sizeof(void*) == sizeof(DWORD64)) {
                    ReadProcessMemoryAtPtrLocation((void*)*(((DWORD64*)fileBuf1) + i), 4, process, &buf);
                } else if (sizeof(void*) == sizeof(DWORD)) {
                    // suppress size warning since we made sure that the size is correct
                    #pragma warning(push)
                    #pragma warning(disable: 4312)
                    ReadProcessMemoryAtPtrLocation((void*)*(((DWORD*)fileBuf1) + i), 4, process, &buf);
                    #pragma warning(pop)
                }
                if (ValueIsMatching(&testVal, &buf)) {
                    foundValInMemoryBySnapshotPtr = TRUE;
                }

                BYTEARRAY buf1 = {0};
                if (sizeof(void*) == sizeof(DWORD64)) {
                    memcpy(&buf1.values, ((DWORD64*)fileBuf2 + i/sizeof(void*)), sizeof(int));
                    buf1.size = sizeof(int);
                } else if (sizeof(void*) == sizeof(DWORD)) {
                    memcpy(&buf1.values, ((DWORD*)fileBuf2 + i/sizeof(void*)), sizeof(int));
                    buf1.size = sizeof(int);
                }
                if (ValueIsMatching(&testVal, &buf1)) {
                    foundValOnDisc = TRUE;
                }
            }
        }
    }
    if (foundValInMemoryBySnapshotPtr && foundValOnDisc) {
        printf("memorySnapshotSavesCorrectValueAndPointerTest() success\n");
    } else {
        printf("memorySnapshotSavesCorrectValueAndPointerTest() failed\n");
    }

    fclose(file1);
    fclose(file2);
    Exit:
    system("del buf.txt");
    system("del \"buf.txt - ptrs\"");
    system("taskkill /IM memoryTestApp.exe /F >nul");
}


void WriteProcessMemoryAtPtrLocationTest() {
    
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }

    MEMPTRS matchingPtrs = {0};
    BYTEARRAY testVal = {0};
    UintToByteArray(&testVal, 3254963271);
    FindValueInProcess(&testVal, process, &matchingPtrs);

    BYTEARRAY testVal1 = {0};
    UintToByteArray(&testVal1, 31111113);

    if (matchingPtrs.size == 0) {
        printf("WriteProcessMemoryAtPtrLocationTest() failed! Testvalue not in memory!\n");
        goto Exit;
    }

    for (int i = 0; i < matchingPtrs.size; i++) {
        WriteProcessMemoryAtPtrLocation(process, *(matchingPtrs.memPointerArray + i), testVal1.values, testVal1.size);
    }

    for (int i = 0; i < matchingPtrs.size; i++) {
        BYTEARRAY buf = {0};
        ReadProcessMemoryAtPtrLocation(*(matchingPtrs.memPointerArray + i), sizeof(unsigned int), process, &buf);
        if (!ValueIsMatching(&buf, &testVal1)) {
            printf("WriteProcessMemoryAtPtrLocationTest() failed\n");
            goto Exit;
        }
    }
    printf("WriteProcessMemoryAtPtrLocationTest() success\n");

    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}



int main() {
    // printProcessMemory("test.txt - Editor");
    // printProcessMemory("Buddy Liste");

    // printf("%zu\n", sizeof(DWORD64*));
    
    ValueIsMatchingTest();
    ConcatMemPtrTest();
    ReallocMemPtrsTest();
    IntToByteArrayTest();
    ShortToByteArrayTest();
    ByteToByteArrayTest();
    StrToByteArrayTest();
    BytesToByteArrayTest();
    FloatToByteArrayTest();
    DoubleToByteArrayTest();
    FindValueInProcessTest();
    ReadProcessMemoryAtPtrLocationTest();
    getProcessBaseAddressTest();
    ReallocMemoryMapTest();
    ConcatMemoryMapTest();
    memorySnapshotMemCountMatchesPtrCountTest();
    memorySnapshotSavesCorrectValueAndPointerTest();
    WriteProcessMemoryAtPtrLocationTest();

    return 0;
}