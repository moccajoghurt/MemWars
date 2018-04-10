
#ifndef _MEM_ANALYZER_H
#define _MEM_ANALYZER_H

#include <windows.h>

#define MAX_VAL_SIZE 200

typedef struct _BYTE_ARRAY {
    BYTE values[MAX_VAL_SIZE];
    size_t size;
} BYTEARRAY;

typedef struct _MEM_PTRS {
    void** memPointerArray;
    size_t size;
} MEMPTRS;

typedef struct _MEMORY_MAP {
    MEMPTRS* memPtrs;
    BYTEARRAY** byteArrays;
    size_t size;
} MEMMAP;

BOOL valueIsMatching(BYTEARRAY* memPtr, BYTEARRAY* memPtr1);
void intToByteArray(BYTEARRAY* bArr, int val);
void uintToByteArray(BYTEARRAY* bArr, unsigned int val);
void floatToByteArray(BYTEARRAY* bArr, float f);
void doubleToByteArray(BYTEARRAY* bArr, double d);
void shortToByteArray(BYTEARRAY* bArr, short s);
void byteToByteArray(BYTEARRAY* bArr, char c);
void strToByteArray(BYTEARRAY* bArr, const char* str);
void reallocMemPtrs(MEMPTRS* memPtrs);                  // todo: x64
void concatMemPtr(void* ptr, MEMPTRS* memPtrs);         // todo: x64
void findValueInProcess(BYTEARRAY* bArrValue, HANDLE process, MEMPTRS* matchingMemPtrs);    // todo: x64
BOOL readProcessMemoryAtPtrLocation(void* ptr, size_t byteLen, HANDLE process, BYTEARRAY* readValueByteArray);  // todo: x64
BOOL writeProcessMemoryAtPtrLocation(HANDLE process, void* baseAdress, void* value, size_t valSize); // todo: x64
HANDLE getProcessByWindowName(const char* windowName);
HANDLE getProcessByName(const TCHAR* szProcessName);
HMODULE getProcessBaseAddress(HANDLE hProcess, TCHAR* szProcessName);
// void printProcessMemoryInformation(MEMORY_BASIC_INFORMATION* info);
// void printProcessMemory(const char* windowName);
void reallocMemoryMap(MEMMAP* memMap);  // todo: x64
void concatMemoryMap(MEMMAP* memMap, void* memPtr, BYTEARRAY* bArrVal); // todo: x64
void memorySnapshotToDisc(HANDLE process, const char* fileName);    // todo: x64
void freeMemMap(MEMMAP* memMap);    // todo: x64


#endif