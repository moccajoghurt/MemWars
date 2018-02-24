#include "memAnalyzer.h"
#include <stdio.h>


BOOL valueIsMatching(BYTE* memPtr, BYTEARRAY* value) {
    for (int i = 0; i < value->size; i++) {
        if ((BYTE)*(memPtr + i) != value->values[i]) {
            return FALSE;
        }
    }
    return TRUE;
}


void intToByteArray(int val, BYTEARRAY* bArr) {
    // windows uses little-endian
    bArr->values[3] = val >> 24;
    bArr->values[2] = val >> 16;
    bArr->values[1] = val >> 8;
    bArr->values[0] = val;
    bArr->size = 4;
}

void reallocMemPtrs(MEMPTRS* matchingMemPtrs) {
    matchingMemPtrs->memPointerArray[matchingMemPtrs->size/20] = malloc(sizeof(int*) * 20);
}

void findValue(BYTEARRAY* value, HANDLE process, MEMPTRS* matchingMemPtrs) {
    BYTE* p = NULL;
    MEMORY_BASIC_INFORMATION info;
    size_t edgeByteCount = 0;
    void* edgeBytePtr = NULL;
    
    for (p = NULL; VirtualQueryEx(process, p, &info, sizeof(info)) != 0; p += info.RegionSize) {
        if (info.State == MEM_COMMIT && (info.Type == MEM_MAPPED || info.Type == MEM_PRIVATE)) {
            
            BYTE* buf = malloc(info.RegionSize);
            size_t bytesRead;
            BOOL status = ReadProcessMemory(process, p, buf, info.RegionSize, &bytesRead);
            if (!status) {
                if (GetLastError() != 299) {
                    printf("ReadProcessMemory() failed: %d\n", GetLastError());
                    return;
                }
            }
            for (int i = 0; i < info.RegionSize; i += value->size) {
                if (i + value->size > info.RegionSize && i != info.RegionSize) {
                    // end of memory reached
                    edgeByteCount = info.RegionSize - i;
                    edgeBytePtr = p + i;
                    printf("edge reached\n");
                    continue;
                } else {

                    //edge byte handling
                    if (i == 0 && edgeByteCount != 0) {
                        for (int n = 0; n < edgeByteCount; n++) {

                        }
                    }
                    
                    if (valueIsMatching((buf + i), value)) {
                        printf("found match\n");
                        if (matchingMemPtrs->size % 20 == 0 && matchingMemPtrs->size != 0) {
                            reallocMemPtrs(matchingMemPtrs);
                        } else {
                            matchingMemPtrs->memPointerArray[matchingMemPtrs->size] = (int*)(p + i);
                        }
                        matchingMemPtrs->size++;
                    }

                    edgeByteCount = 0;
                    edgeBytePtr = NULL;
                }
                
            }
            free(buf);
        }
    }
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
    HWND windowHwnd = FindWindow(0, windowName);
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
    UCHAR *p = NULL;
    MEMORY_BASIC_INFORMATION info;
    for (p = NULL; VirtualQueryEx(process, p, &info, sizeof(info)) != 0; p += info.RegionSize) {
        printProcessMemoryInformation(&info);
        if (info.State == MEM_COMMIT && (info.Type == MEM_MAPPED || info.Type == MEM_PRIVATE /*||info.Type == MEM_IMAGE*/)) {
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

