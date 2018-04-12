#include <stdio.h>
#include <windows.h>
#include <Psapi.h> // enumprocesses
#include <limits.h> // UINT_MAX
#include "MemWarsCore.h"


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
    bArr->size = sizeof(val);
    memcpy(bArr->values, &val, sizeof(val));
}

void uintToByteArray(BYTEARRAY* bArr, unsigned int val) {
    bArr->size = sizeof(val);
    memcpy(bArr->values, &val, sizeof(val));
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

BOOL writeProcessMemoryAtPtrLocation(HANDLE process, void* baseAdress, void* value, size_t valSize) {
    BOOL status =  WriteProcessMemory(process, baseAdress, value, valSize, NULL);
    if (status == 0) {
        printf("writeMemoryAtPtrLocation()::WriteProcessMemory() failed!\n");
        return FALSE;
    }
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


void* GetProcessBaseAddress(const HANDLE hProcess) {
	if (hProcess == NULL) {
        return NULL;
    }
	HMODULE lphModule[1024];
	DWORD lpcbNeeded;
	if (!EnumProcessModules(hProcess, lphModule, sizeof(lphModule), &lpcbNeeded)) {
        return NULL;
    }
	return lphModule[0]; // Module 0 is the EXE itself, returning its address
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
                void* ptrBuf = (p + i);
                fwrite(&ptrBuf, sizeof(void*), 1, memPtrFile);
            }
            fwrite(buf, sizeof(BYTE), bytesRead, dataFile);
            free(buf);
        }
    }
    fclose(dataFile);
    fclose(memPtrFile);
    free(memPtrsFileName);
}

BOOL SetProcessPrivilege(LPCSTR lpszPrivilege, BOOL bEnablePrivilege) {
    TOKEN_PRIVILEGES priv = {0, 0, 0, 0};
    HANDLE hToken = NULL;
    LUID luid = {0, 0};
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        if (hToken) {
            CloseHandle(hToken);
        }
        return FALSE;
    }
    if (!LookupPrivilegeValueA(0, lpszPrivilege, &luid)) {
        if (hToken){
            CloseHandle(hToken);
        }
        return FALSE;
    }
    priv.PrivilegeCount = 1;
    priv.Privileges[0].Luid = luid;
    priv.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : SE_PRIVILEGE_REMOVED;
    if (!AdjustTokenPrivileges(hToken, FALSE, &priv, 0, 0, 0)) {
        if (hToken) {
            CloseHandle(hToken);
        }
        return FALSE;
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return TRUE;
}
