
#ifndef _MEM_ANALYZER_H
#define _MEM_ANALYZER_H

// #include <stdlib.h> // size_t
#include <windows.h>

#define MAX_VAL_SIZE 200

typedef struct _MEM_PTRS {
    // 4 bytes
    BYTE** memPointerArray;
    size_t size;
} MEMPTRS;

typedef struct _BYTE_ARRAY {
    BYTE values[MAX_VAL_SIZE];
    size_t size;
} BYTEARRAY;

#endif