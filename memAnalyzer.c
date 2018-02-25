#include <stdio.h>
#include <windows.h>
#include <Psapi.h> // enumprocesses
#include "memAnalyzer.h"


BOOL valueIsMatching(BYTE* memPtr, BYTEARRAY* value) {
    // for (int i = 0; i < value->size; i++) {
    //     printf("%c, %c\n", *(memPtr + i), value->values[i]);
    // }
    for (int i = 0; i < value->size; i++) {
        if ((BYTE)*(memPtr + i) != value->values[i]) {
            return FALSE;
        }
    }
    return TRUE;
}


void intToByteArray(BYTEARRAY* bArr, int val) {
    // windows uses little-endian
    bArr->values[3] = val >> 24;
    bArr->values[2] = val >> 16;
    bArr->values[1] = val >> 8;
    bArr->values[0] = val;
    bArr->size = 4;
}

void floatToByteArray(BYTEARRAY* bArr, float val){
    // memcpy(bArr->values, &val, sizeof(float));
    bArr->values[3] = *((BYTE*)&val);
    bArr->values[2] = *(((BYTE*)&val) + 1);
    bArr->values[1] = *(((BYTE*)&val) + 2);
    bArr->values[0] = *(((BYTE*)&val) + 3);
    bArr->size = sizeof(float);

    for (int i = 0; i < bArr->size; i++) {
        printf("%c\n", *(((BYTE*)&val) + i));
    }
}

void reallocMemPtrs(MEMPTRS* memPtrs) {
    if (memPtrs->size == 0) {
        memPtrs->memPointerArray = malloc(sizeof(BYTE*) * 20);
    } else {
        memPtrs->memPointerArray = realloc(memPtrs->memPointerArray, memPtrs->size * sizeof(BYTE*) + sizeof(BYTE*) * 20);
    }
}

void concatMemPtr(BYTE* ptr, MEMPTRS* memPtrs) {
    if (memPtrs->size % 20 == 0) {
        reallocMemPtrs(memPtrs);
    }
    memPtrs->memPointerArray[memPtrs->size] = ptr;
    memPtrs->size++;
}

void findValueByProcess(BYTEARRAY* value, HANDLE process, MEMPTRS* matchingMemPtrs) {
    BYTE* p = NULL;
    MEMORY_BASIC_INFORMATION info;
    
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
            for (int i = 0; i < bytesRead; i += value->size) {
                if (i + value->size > bytesRead) {
                    // end of memory reached
                    printf("edge reached\n");
                    break;
                }
                if (valueIsMatching((buf + i), value)) {
                    printf("found match\n");
                    concatMemPtr((p + i), matchingMemPtrs);
                }
            }
            free(buf);
        }
    }
}

HANDLE getProcessByWindowName(const char* windowName) {
    HWND windowHwnd = FindWindow(0, windowName);
    if (windowHwnd == NULL) {
        printf("getProcessByWindowName::FindWindow() returned NULL: %d\n", GetLastError());
        return NULL;
    }
    DWORD processId;
    DWORD thread = GetWindowThreadProcessId(windowHwnd, &processId);
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
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
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID);

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

