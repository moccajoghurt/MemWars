#include <stdio.h>
#include <windows.h>
#include "MemWarsBase.h"

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

void shortToByteArrayTest() {
    short testVal = 137;
    BYTEARRAY bArr1;
    memcpy(bArr1.values, &testVal, sizeof(short));
    bArr1.size = sizeof(short);
    BYTEARRAY bArr;
    shortToByteArray(&bArr, testVal);
    if (valueIsMatching(&bArr, &bArr1)) {
        printf("shortToByteArrayTest() success\n");
    } else {
        printf("shortToByteArrayTest() failed\n");
    }
}

void byteToByteArrayTest() {
    char testVal = 255;
    BYTEARRAY bArr1;
    memcpy(bArr1.values, &testVal, sizeof(char));
    bArr1.size = sizeof(char);
    BYTEARRAY bArr;
    byteToByteArray(&bArr, testVal);
    if (valueIsMatching(&bArr, &bArr1)) {
        printf("byteToByteArrayTest() success\n");
    } else {
        printf("byteToByteArrayTest() failed\n");
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
    } else {
        printf("reallocMemoryMapTest() failed\n");
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

void memorySnapshotMemCountMatchesPtrCountTest() {
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
        // process = (HANDLE)getProcessByName("ac_client.exe");
    }
    memorySnapshotToDisc(process, "buf.txt");
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
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }
    memorySnapshotToDisc(process, "buf.txt");

    MEMPTRS matchingPtrs = {0};
    BYTEARRAY testVal = {0};
    intToByteArray(&testVal, 133337);
    findValueInProcess(&testVal, process, &matchingPtrs);
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

            if (valueIsMatching(&filebufVal, &memPtrVal)) {
                BYTEARRAY buf = {0};
                

                if (sizeof(void*) == sizeof(DWORD64)) {
                    readProcessMemoryAtPtrLocation((void*)*(((DWORD64*)fileBuf1) + i), 4, process, &buf);
                } else if (sizeof(void*) == sizeof(DWORD)) {
                    // suppress size warning since we made sure that the size is correct
                    #pragma warning(push)
                    #pragma warning(disable: 4312)
                    readProcessMemoryAtPtrLocation((void*)*(((DWORD*)fileBuf1) + i), 4, process, &buf);
                    #pragma warning(pop)
                }
                if (valueIsMatching(&testVal, &buf)) {
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
                if (valueIsMatching(&testVal, &buf1)) {
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


void writeProcessMemoryAtPtrLocationTest() {
    
    system("start /B memoryTestApp.exe");
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)getProcessByName("memoryTestApp.exe");
    }

    MEMPTRS matchingPtrs = {0};
    BYTEARRAY testVal = {0};
    uintToByteArray(&testVal, 3254963271);
    findValueInProcess(&testVal, process, &matchingPtrs);

    BYTEARRAY testVal1 = {0};
    uintToByteArray(&testVal1, 31111113);

    if (matchingPtrs.size == 0) {
        printf("writeProcessMemoryAtPtrLocationTest() failed! Testvalue not in memory!\n");
        goto Exit;
    }

    for (int i = 0; i < matchingPtrs.size; i++) {
        writeProcessMemoryAtPtrLocation(process, *(matchingPtrs.memPointerArray + i), testVal1.values, testVal1.size);
    }

    for (int i = 0; i < matchingPtrs.size; i++) {
        BYTEARRAY buf = {0};
        readProcessMemoryAtPtrLocation(*(matchingPtrs.memPointerArray + i), sizeof(unsigned int), process, &buf);
        if (!valueIsMatching(&buf, &testVal1)) {
            printf("writeProcessMemoryAtPtrLocationTest() failed\n");
            goto Exit;
        }
    }
    printf("writeProcessMemoryAtPtrLocationTest() success\n");

    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

void injectx64ShellcodeTest() {
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


int main() {
    // printProcessMemory("test.txt - Editor");
    // printProcessMemory("Buddy Liste");

    // printf("%zu\n", sizeof(DWORD64*));
    
    // valueIsMatchingTest();
    // concatMemPtrTest();
    // reallocMemPtrsTest();
    // intToByteArrayTest();
    // shortToByteArrayTest();
    // byteToByteArrayTest();
    // strToByteArrayTest();
    // floatToByteArrayTest();
    // doubleToByteArrayTest();
    // findValueInProcessTest();
    // readProcessMemoryAtPtrLocationTest();
    // getProcessBaseAddressTest();
    // reallocMemoryMapTest();
    // concatMemoryMapTest();
    // memorySnapshotMemCountMatchesPtrCountTest();
    // memorySnapshotSavesCorrectValueAndPointerTest();
    // writeProcessMemoryAtPtrLocationTest();
    injectx64ShellcodeTest();

    return 0;
}