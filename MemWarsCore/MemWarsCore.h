
#ifndef _MEM_ANALYZER_H
#define _MEM_ANALYZER_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_VAL_SIZE 200

typedef struct _BYTE_ARRAY {
    BYTE values[MAX_VAL_SIZE];
    SIZE_T size;
} BYTEARRAY;

typedef struct _MEM_PTRS {
    void** memPointerArray;
    SIZE_T size;
} MEMPTRS;

typedef struct _MEMORY_MAP {
    MEMPTRS* memPtrs;
    BYTEARRAY** byteArrays;
    SIZE_T size;
} MEMMAP;


BOOL ValueIsMatching(BYTEARRAY*, BYTEARRAY*);
void IntToByteArray(BYTEARRAY*, INT);
void UintToByteArray(BYTEARRAY*, UINT);
void FloatToByteArray(BYTEARRAY*, FLOAT);
void DoubleToByteArray(BYTEARRAY*, DOUBLE);
void ShortToByteArray(BYTEARRAY*, SHORT);
void ByteToByteArray(BYTEARRAY*, BYTE);
void StrToByteArray(BYTEARRAY*, const TCHAR*);
BOOL ReallocMemPtrs(MEMPTRS*);
BOOL ConcatMemPtr(void*, MEMPTRS*);
BOOL FindValueInProcess(BYTEARRAY*, HANDLE, MEMPTRS*);
BOOL ReadProcessMemoryAtPtrLocation(void*, SIZE_T, HANDLE, BYTEARRAY*);
BOOL WriteProcessMemoryAtPtrLocation(HANDLE, void*, void*, SIZE_T);
HANDLE GetProcessByWindowName(const TCHAR*);
HANDLE GetProcessByName(const TCHAR*);
void* GetProcessBaseAddress(HANDLE);
BOOL SetProcessPrivilege(LPCSTR, BOOL);
BOOL ReallocMemoryMap(MEMMAP*);
BOOL ConcatMemoryMap(MEMMAP*, void*, BYTEARRAY*);
BOOL MemorySnapshotToDisc(HANDLE, const TCHAR*);
void FreeMemMap(MEMMAP*);

#ifdef __cplusplus
}
#endif

#endif