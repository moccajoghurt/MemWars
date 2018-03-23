#include <stdio.h>
#include <windows.h>
#include <Psapi.h> // enumprocesses
#include "memAnalyzer.h"


BOOL valueIsMatching(BYTEARRAY* memPtr, BYTEARRAY* memPtr1) {
    if (memPtr->size != memPtr1->size) {
        printf("valueIsMatching() failed. memPtr->size != memPtr1->size\n");
        return FALSE;
    }
    int status = memcmp(memPtr->values, memPtr1->values, memPtr->size);
    if (status == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}


void intToByteArray(BYTEARRAY* bArr, int val) {
    // windows uses little-endian
    bArr->values[3] = val >> 24;
    bArr->values[2] = val >> 16;
    bArr->values[1] = val >> 8;
    bArr->values[0] = val;
    bArr->size = 4;
}

void byteToByteArray(BYTEARRAY* bArr, char c) {
    bArr->size = sizeof(c);
    memcpy(bArr->values, &c, sizeof(c));
}

void shortToByteArray(BYTEARRAY* bArr, short s) {
    bArr->size = sizeof(s);
    memcpy(bArr->values, &s, sizeof(s));
}

void floatToByteArray(BYTEARRAY* bArr, float f) {
    bArr->size = sizeof(f);
    memcpy(bArr->values, &f, sizeof(f));
}

void doubleToByteArray(BYTEARRAY* bArr, double d) {
    bArr->size = sizeof(d);
    memcpy(bArr->values, &d, sizeof(d));
}

void strToByteArray(BYTEARRAY* bArr, const char* str) {
    if (strlen(str) > MAX_VAL_SIZE) {
        printf("strToByteArray()::String too long! Increase MAX_VAL_SIZE\n");
        return;
    }
    strcpy(bArr->values, str);
    bArr->size = strlen(str);
}

void reallocMemPtrs(MEMPTRS* memPtrs) {
    if (memPtrs->size == 0) {
        memPtrs->memPointerArray = malloc(sizeof(void*) * 20);
    } else {
        memPtrs->memPointerArray = realloc(memPtrs->memPointerArray, memPtrs->size * sizeof(void*) + sizeof(void*) * 20);
    }
}

void concatMemPtr(void* ptr, MEMPTRS* memPtrs) {
    if (memPtrs->size % 20 == 0) {
        reallocMemPtrs(memPtrs);
    }
    memPtrs->memPointerArray[memPtrs->size] = ptr;
    memPtrs->size++;
}

void findValueInProcess(BYTEARRAY* bArrValue, HANDLE process, MEMPTRS* matchingMemPtrs) {
    BYTE* p = NULL;
    MEMORY_BASIC_INFORMATION info;
    
    for (p = NULL; VirtualQueryEx(process, p, &info, sizeof(info)) != 0; p += info.RegionSize) {
        if (info.State == MEM_COMMIT/* && (info.Type == MEM_MAPPED || info.Type == MEM_PRIVATE || MEM_IMAGE)*/) {
            BYTE* buf = malloc(info.RegionSize);
            size_t bytesRead;
            BOOL status = ReadProcessMemory(process, p, buf, info.RegionSize, &bytesRead);
            if (!status) {
                if (GetLastError() != 299) {
                    printf("findValueInProcess()::ReadProcessMemory() failed: %d\n", GetLastError());
                    return;
                }
            }
            for (int i = 0; i < bytesRead; i++) {
                if (i + bArrValue->size > bytesRead) {
                    // end of memory reached
                    // printf("edge reached\n");
                    break;
                }
                BYTEARRAY memVal;
                memcpy(memVal.values, buf + i, bArrValue->size);
                memVal.size = bArrValue->size;
                if (valueIsMatching(&memVal, bArrValue)) {
                    concatMemPtr((p + i), matchingMemPtrs);
                }
            }
            free(buf);
        }
    }
}


void reallocMemoryMap(MEMMAP* memMap) {
    if (memMap->size == 0) {
        memMap->byteArrays = malloc(sizeof(BYTEARRAY*) * 20);
        memMap->memPtrs = calloc(sizeof(MEMPTRS), 0);
        memMap->memPtrs->size = 0;
    } else {
        memMap->byteArrays = realloc(memMap->byteArrays, memMap->size * sizeof(BYTEARRAY*) + sizeof(BYTEARRAY*) * 20);
    }
}

void concatMemoryMap(MEMMAP* memMap, void* memPtr, BYTEARRAY* bArrVal) {
    if (memMap->size % 20 == 0) {
        reallocMemoryMap(memMap);
    }
    memMap->byteArrays[memMap->size] = malloc(sizeof(BYTEARRAY));
    memcpy(memMap->byteArrays[memMap->size], bArrVal, sizeof(BYTEARRAY));
    concatMemPtr(memPtr, memMap->memPtrs);
    memMap->size++;
}

void freeMemMap(MEMMAP* memMap) {
    for (int i = 0; i < memMap->size; i++) {
        free(memMap->byteArrays[i]);
    }
    free(memMap->byteArrays);
    free(memMap->memPtrs->memPointerArray);
    free(memMap->memPtrs);
}


BOOL readProcessMemoryAtPtrLocation(void* ptr, size_t byteLen, HANDLE process, BYTEARRAY* readValueByteArray) {
    if (byteLen > MAX_VAL_SIZE) {
        printf("readProcessMemoryAtPtrLocation() failed, byteLen too big, increase MAX_VAL_SIZE\n");
        return FALSE;
    }
    MEMORY_BASIC_INFORMATION info;
    BOOL status = VirtualQueryEx(process, ptr, &info, sizeof(info));
    if (status == 0 || info.RegionSize < byteLen || info.State != MEM_COMMIT) {
        printf("readProcessMemoryAtPtrLocation()::VirtualQueryEx() failed\n");
        return FALSE;
    }
    BYTE* buf = malloc(info.RegionSize);
    size_t bytesRead;
    status = ReadProcessMemory(process, ptr, buf, byteLen, &bytesRead);
    if (!status) {
        if (GetLastError() != 299) {
            printf("readProcessMemoryAtPtrLocation()::ReadProcessMemory() failed: %d\n", GetLastError());
            return FALSE;
        }
    }
    if (bytesRead < byteLen) {
        return FALSE;
    }
    memcpy(readValueByteArray->values, buf, byteLen);
    readValueByteArray->size = byteLen;

    free(buf);
    return TRUE;
}

HANDLE getProcessByWindowName(const char* windowName) {
    HWND windowHwnd = FindWindow(0, windowName);
    if (windowHwnd == NULL) {
        printf("getProcessByWindowName::FindWindow() returned NULL: %d\n", GetLastError());
        return NULL;
    }
    DWORD processId;
    DWORD thread = GetWindowThreadProcessId(windowHwnd, &processId);
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_ALL_ACCESS, FALSE, processId);
    if (process == NULL) {
        printf("getProcessByWindowName::OpenProcess() returned NULL: %d\n", GetLastError());
    }
    return process;
}

HANDLE getProcessByName(const TCHAR* szProcessName) {
    if(szProcessName == NULL) return NULL;
    const char* strProcessName = szProcessName;

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return NULL;
    }
    cProcesses = cbNeeded / sizeof(DWORD);
    for (unsigned int i = 0; i < cProcesses; i++) {
        DWORD dwProcessID = aProcesses[i];
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_ALL_ACCESS, FALSE, dwProcessID);

        TCHAR szEachProcessName[MAX_PATH];
        if (hProcess != NULL) {
            HMODULE hMod;
            DWORD cbNeeded;
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                GetModuleBaseName(hProcess, hMod, szEachProcessName, sizeof(szEachProcessName) / sizeof(TCHAR));
            }
        }
        if (strcmp(strProcessName, szEachProcessName) == 0) {
            return hProcess;
        }
        CloseHandle(hProcess);
    }
    return NULL;
}

HMODULE getProcessBaseAddress(HANDLE hProcess, TCHAR* szProcessName) {

    TCHAR szProcessNameBuf[MAX_PATH];
    HMODULE hMod;
    DWORD cbNeeded;
    if (EnumProcessModulesEx(hProcess, &hMod, sizeof(hMod), &cbNeeded, LIST_MODULES_32BIT | LIST_MODULES_64BIT)) {
        GetModuleBaseName(hProcess, hMod, szProcessNameBuf, sizeof(szProcessNameBuf) / sizeof(TCHAR));
        if (!_tcsicmp(szProcessName, szProcessNameBuf)) {
            return hMod;
        }
    }
    return NULL;
}

void memorySnapshotToDisc(HANDLE process, const char* fileName) {
    char* memPtrsFileName;
    memPtrsFileName = malloc(strlen(fileName) + strlen(" - ptrs"));
    strcpy(memPtrsFileName, fileName);
    strcat(memPtrsFileName, " - ptrs");
    FILE* dataFile = fopen(fileName, "wb");
    FILE* memPtrFile = fopen(memPtrsFileName, "wb");
    if (dataFile == NULL) {
        printf("Could not open file %s\n", fileName);
        exit(1);
    }
    BYTE* p = NULL;
    MEMORY_BASIC_INFORMATION info;
    for (p = NULL; VirtualQueryEx(process, p, &info, sizeof(info)) != 0; p += info.RegionSize) {
        if (info.State == MEM_COMMIT) {
            BYTE* buf = malloc(info.RegionSize);
            size_t bytesRead;
            BOOL status = ReadProcessMemory(process, p, buf, info.RegionSize, &bytesRead);
            if (!status) {
                if (GetLastError() != 299) {
                    printf("findValueInProcess()::ReadProcessMemory() failed: %d\n", GetLastError());
                    return;
                }
            }
            for (int i = 0; i < bytesRead; i++) {
                unsigned int ptrBuf = (unsigned int)(p + i);
                fwrite(&ptrBuf, sizeof(unsigned int), 1, memPtrFile);
            }
            fwrite(buf, sizeof(char), bytesRead, dataFile);
            free(buf);
        }
    }
    fclose(dataFile);
    fclose(memPtrFile);
    free(memPtrsFileName);
}

// this function compares two memory snapshots and either saves values that changed or didn't change
void filterMemorySnapshots(const char* oldSnapshotFileName1, const char* recentSnapshotfileName2, const char* filteredSnapshotName, size_t valByteLen, BOOL valsChanged) {

    FILE* snapshot1;
    snapshot1 = fopen(oldSnapshotFileName1, "rb");
    if (snapshot1 == NULL) {
        printf("Could not open file %s\n", oldSnapshotFileName1);
        exit(1);
    }

    FILE* snapshot2;
    snapshot2 = fopen(recentSnapshotfileName2, "rb");
    if (snapshot2 == NULL) {
        printf("Could not open file %s\n", recentSnapshotfileName2);
        exit(1);
    }

    char* snapshotPtrsFileName1;
    snapshotPtrsFileName1 = malloc(strlen(oldSnapshotFileName1) + strlen(" - ptrs"));
    strcpy(snapshotPtrsFileName1, oldSnapshotFileName1);
    strcat(snapshotPtrsFileName1, " - ptrs");
    FILE* snapshotPtrs1;
    snapshotPtrs1 = fopen(snapshotPtrsFileName1, "rb");
    if (snapshotPtrs1 == NULL) {
        printf("Could not open file %s\n", snapshotPtrsFileName1);
        exit(1);
    }

    char* snapshotPtrsFileName2;
    snapshotPtrsFileName2 = malloc(strlen(recentSnapshotfileName2) + strlen(" - ptrs"));
    strcpy(snapshotPtrsFileName2, recentSnapshotfileName2);
    strcat(snapshotPtrsFileName2, " - ptrs");
    FILE* snapshotPtrs2;
    snapshotPtrs2 = fopen(snapshotPtrsFileName2, "rb");
    if (snapshotPtrs2 == NULL) {
        printf("Could not open file %s\n", snapshotPtrsFileName2);
        exit(1);
    }

    int ptrFileSize1;
    fseek(snapshotPtrs1, 0 , SEEK_END);
    ptrFileSize1 = ftell(snapshotPtrs1);
    fseek(snapshotPtrs1, 0, SEEK_SET);

    int ptrFileSize2;
    fseek(snapshotPtrs2, 0 , SEEK_END);
    ptrFileSize2 = ftell(snapshotPtrs2);
    fseek(snapshotPtrs2, 0, SEEK_SET);

    int snapshotSize1;
    fseek(snapshot1, 0 , SEEK_END);
    snapshotSize1 = ftell(snapshot1);
    fseek(snapshot1, 0, SEEK_SET);

    int snapshotSize2;
    fseek(snapshot2, 0 , SEEK_END);
    snapshotSize2 = ftell(snapshot2);
    fseek(snapshot2, 0, SEEK_SET);

    unsigned int* smallFilePtrsBuf = malloc(ptrFileSize1 * 4);
    fread(smallFilePtrsBuf, ptrFileSize1, 1, snapshotPtrs1);

    unsigned int* bigFilePtrsBuf = malloc(ptrFileSize2 * 4);
    fread(bigFilePtrsBuf, ptrFileSize2, 1, snapshotPtrs2);

    BYTE* smallSnapshotBuf = malloc(snapshotSize1 * 4);
    fread(smallSnapshotBuf, snapshotSize1, 1, snapshot1);

    BYTE* bigSnapshotBuf = malloc(snapshotSize2 * 4);
    fread(bigSnapshotBuf, snapshotSize2, 1, snapshot2);

    BYTE* recentSnapshot = bigSnapshotBuf;
    unsigned int* recentPtrs = bigFilePtrsBuf;

    int smallFileIterationLength = ptrFileSize1;

    if (ptrFileSize1 > ptrFileSize2) {
        void* ptrBuf = smallFilePtrsBuf;
        void* snapshotBuf = smallSnapshotBuf;
        smallFilePtrsBuf = bigFilePtrsBuf;
        bigFilePtrsBuf = ptrBuf;
        smallSnapshotBuf = bigSnapshotBuf;
        bigSnapshotBuf = snapshotBuf;
        smallFileIterationLength = ptrFileSize2;
    }

    FILE* filterFile = fopen(filteredSnapshotName, "wb");
    if (filterFile == NULL) {
        printf("Could not open file %s\n", filteredSnapshotName);
        exit(1);
    }

    char* filterPtrsFileName = malloc(strlen(filteredSnapshotName) + strlen(" - ptrs"));
    strcpy(filterPtrsFileName, filteredSnapshotName);
    strcat(filterPtrsFileName, " - ptrs");
    FILE* filterPtrs = fopen(filterPtrsFileName, "wb");
    if (filterPtrs == NULL) {
        printf("Could not open file %s\n", filterPtrsFileName);
        exit(1);
    }

    unsigned int bigFileIterator = 0;
    unsigned int i;
    unsigned int* recentFileIterator = recentSnapshot == bigSnapshotBuf ? &bigFileIterator : &i;
    unsigned int* recentPtrFileIterator = recentPtrs == bigFilePtrsBuf ? &bigFileIterator : &i;
    for (i = 0; i < smallFileIterationLength; i += sizeof(unsigned int)) {
        // make sure we are looking at the same pointers
        while (*(smallFilePtrsBuf + i) != *(bigFilePtrsBuf + bigFileIterator)) {
            bigFileIterator += sizeof(unsigned int);
        }

        BYTE* bigSnapshotPtr = bigSnapshotBuf + bigFileIterator/sizeof(unsigned int);
        BYTE* smallSnapshotPtr = smallSnapshotBuf + i/sizeof(unsigned int);
        if (!valsChanged && memcmp(bigSnapshotPtr, smallSnapshotPtr, valByteLen) == 0 ||
            valsChanged && memcmp(bigSnapshotPtr, smallSnapshotPtr, valByteLen) != 0) {
            fwrite(recentSnapshot + *recentFileIterator/sizeof(unsigned int), valByteLen, 1, filterFile);
            fwrite(recentPtrs + *recentPtrFileIterator, valByteLen * sizeof(unsigned int), 1, filterPtrs);
        }
        bigFileIterator += sizeof(unsigned int);
    }
    fclose(snapshotPtrs1);
    fclose(snapshotPtrs2);
    fclose(snapshot1);
    fclose(snapshot2);
    fclose(filterFile);
    fclose(filterPtrs);

}

void printProcessMemoryInformation(MEMORY_BASIC_INFORMATION* info) {
    printf("\n______________________________________________________\n");
    printf("BaseAddress:\t\t 0x%x\n", (UINT)info->BaseAddress);
    printf("AllocationBase:\t\t 0x%x\n", (UINT)info->AllocationBase);
    printf("RegionSize:\t\t %u\n", info->RegionSize);
    printf("State:\t\t\t %d\n", info->State);
    printf("Protect:\t\t %x\n", info->Protect);
    printf("Type:\t\t\t %d\n", info->Type);
    printf("______________________________________________________\n");
}

void printProcessMemory(const char* windowName) {
    HANDLE process = (HANDLE)getProcessByWindowName(windowName);
    if (process == NULL) {
        return;
    }
    UCHAR *p = NULL;
    MEMORY_BASIC_INFORMATION info;
    for (p = NULL; VirtualQueryEx(process, p, &info, sizeof(info)) != 0; p += info.RegionSize) {
        printProcessMemoryInformation(&info);
        if (info.State == MEM_COMMIT && (info.Type == MEM_MAPPED || info.Type == MEM_PRIVATE ||info.Type == MEM_IMAGE)) {
            UCHAR* buf = malloc(info.RegionSize);
            SIZE_T bytesRead;
            ReadProcessMemory(process, p, buf, info.RegionSize, &bytesRead);
            for (int i = 0; i < info.RegionSize; i ++) {
                printf("%c", *(buf + i));
            }
            free(buf);
        }
    }
    printf("\n");
}

