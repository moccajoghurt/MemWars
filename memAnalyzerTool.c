#include <stdio.h>
#include "memAnalyzer.h"
#include "memAnalyzerTool.h"

size_t scanfByDatatype(char c, BYTEARRAY* bArr) {
    switch (c) {
        case 'd':
            scanf("%d", (int*)bArr->values);
            bArr->size = sizeof(int);
            return bArr->size;
        case 'u':
            scanf("%u", (unsigned int*)bArr->values);
            bArr->size = sizeof(unsigned int);
            return bArr->size;
        case 'h':
            scanf("%h", (short*)bArr->values);
            bArr->size = sizeof(short);
            return bArr->size;
        case 'o':
            scanf("%hu", (unsigned short*)bArr->values);
            bArr->size = sizeof(unsigned short);
            return bArr->size;
        case 'l':
            scanf("%lf", (double*)bArr->values);
            bArr->size = sizeof(double);
            return bArr->size;
        case 'f':
            scanf("%f", (float*)bArr->values);
            bArr->size = sizeof(float);
            return bArr->size;
        case 'b':
            scanf("%c", (char*)bArr->values);
            bArr->size = sizeof(char);
            return bArr->size;
        case 's':
            char buf[MAX_VAL_SIZE];
            scanf("%s", buf);
            bArr->size = strlen(buf);
            strcpy_s(bArr->values, MAX_VAL_SIZE, buf);
            return bArr->size;
        default:
            fprintf(stderr, "invalid datatype\n");
            exit(0);
    }
}

void valueSearchRoutine(HANDLE hProcess) {
    BYTEARRAY bArr = {0};
    fprintf(stderr, "Enter datatype to search for.\n1. int (d)\n2. uint (u)\n3. short (h)\n4. ushort (o)\n5. double (l)\n6. float (f)\n7. byte/char (b) (enter decimal value) \n8. string (s)\n");
    char c = getchar();getchar();
    printf("Enter the value:\n");
    scanfByDatatype(c, &bArr);

    MEMPTRS matchingMemPtrs = {0};
    findValueInProcess(&bArr, hProcess, &matchingMemPtrs);
    printf("Matching pointers:\n");
    for (int i = 0; i < matchingMemPtrs.size; i++) {
        printf("%#x\n", (unsigned int)*(matchingMemPtrs.memPointerArray + i));
    }
    if (matchingMemPtrs.size == 0) {
        fprintf(stderr, "No matching pointers found\n");
        return;
    }
    // now we reiterate over the matching pointers as long as we want
    for (;;) {
        MEMPTRS matchingMemPtrsBuf = {0};
        fprintf(stderr, "Enter new (or same) value (of the same datatype) to search among the matching pointers:\n");
        size_t dataSize = scanfByDatatype(c, &bArr);
        printf("Matching pointers:\n");
        for (int i = 0; i < matchingMemPtrs.size; i++) {
            BYTEARRAY bArrBuf = {0};
            readProcessMemoryAtPtrLocation(*(matchingMemPtrs.memPointerArray + i), dataSize, hProcess, &bArrBuf);
            if (valueIsMatching(&bArr, &bArrBuf)) {
                printf("%#x\n", (unsigned int)*(matchingMemPtrs.memPointerArray + i));
                concatMemPtr(*(matchingMemPtrs.memPointerArray + i), &matchingMemPtrsBuf);
            }
        }
        free(matchingMemPtrs.memPointerArray);
        matchingMemPtrs = matchingMemPtrsBuf;
        if (matchingMemPtrs.size == 0) {
            fprintf(stderr, "No more matching pointers exist.\n");
            break;
        }
    }
}

int main(int argc, char* argv[]) {

    HANDLE hProcess = getProcessByName(argv[1]);
    if (hProcess == NULL) {
        fprintf(stderr, "Could not find process %s\n", argv[1]);
        return;
    }
    // we use stderr for user notifications and stdout for data values
    fprintf(stderr, "Found %s.\n", argv[1]);
    fprintf(stderr, "1. search for value (1)\n2. compare memory snapshots (2)\n");
    char c = getchar();getchar();
    switch (c) {
        case '1':
            valueSearchRoutine(hProcess);
            break;
        case '2':

            break;

        default:
            fprintf(stderr, "Invalid input.\n");
            break;
    }
    fflush(stdout);
    for (;;) {

    }
}