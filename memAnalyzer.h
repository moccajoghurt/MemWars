
#ifndef _MEM_ANALYZER_H
#define _MEM_ANALYZER_H

// #include <stdlib.h> // size_t
#include <windows.h>

#define MAX_VAL_SIZE 200

typedef struct _BYTE_ARRAY {
    size_t size;
    BYTE values[MAX_VAL_SIZE];
} BYTEARRAY;

typedef struct _MEM_PTRS {
    // 4 bytes
    void** memPointerArray;
    size_t datatypeSize;
    size_t size;
} MEMPTRS;

#endif