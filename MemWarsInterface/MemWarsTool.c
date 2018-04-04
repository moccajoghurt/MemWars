#include <stdio.h>
#include <windows.h>
#include <Psapi.h> // GetModuleFileNameEx
#include "../MemWarsBase/MemWarsBase.h"
#include "MemWarsTool.h"

char getCharacter() {
    char c = getchar();
    while(c == EOF || c == '\n') {
        c = getchar();
    }
    // do {
    // c = getchar();
    // } while (c != '\n' && c != EOF);
    return c;
}

size_t scanfByDatatype(char c, BYTEARRAY* bArr) {
    switch (c) {
        case 'd':
            scanf(" %d", (int*)bArr->values);
            bArr->size = sizeof(int);
            return bArr->size;
        case 'u':
            scanf(" %u", (unsigned int*)bArr->values);
            bArr->size = sizeof(unsigned int);
            return bArr->size;
        case 'h':
            scanf(" %h", (short*)bArr->values);
            bArr->size = sizeof(short);
            return bArr->size;
        case 'o':
            scanf(" %hu", (unsigned short*)bArr->values);
            bArr->size = sizeof(unsigned short);
            return bArr->size;
        case 'l':
            scanf(" %lf", (double*)bArr->values);
            bArr->size = sizeof(double);
            return bArr->size;
        case 'f':
            scanf(" %f", (float*)bArr->values);
            bArr->size = sizeof(float);
            return bArr->size;
        case 'b':
            scanf(" %c", (char*)bArr->values);
            bArr->size = sizeof(char);
            return bArr->size;
        case 's':
            char buf[MAX_VAL_SIZE];
            scanf(" %s", buf);
            bArr->size = strlen(buf);
            strcpy_s(bArr->values, MAX_VAL_SIZE, buf);
            return bArr->size;
        default:
            fprintf(stderr, "invalid datatype\n");
            exit(0);
    }
}

void valueSearchRoutine(HANDLE hProcess, HMODULE baseAddress, TCHAR* processName) {
    BYTEARRAY bArr = {0};
    fprintf(stderr, "Enter datatype to search for.\n1. int (d)\n2. uint (u)\n3. short (h)\n4. ushort (o)\n5. double (l)\n6. float (f)\n7. byte/char (b) (enter decimal value) \n8. string (s)\n");
    char c = getCharacter();
    
    printf("Enter the value:\n");
    scanfByDatatype(c, &bArr);

    MEMPTRS matchingMemPtrs = {0};
    findValueInProcess(&bArr, hProcess, &matchingMemPtrs);
    printf("Matching pointers:\n");
    for (int i = 0; i < matchingMemPtrs.size; i++) {
        printf("%s+%x\n", processName, ((unsigned int)*(matchingMemPtrs.memPointerArray + i) - (unsigned int)baseAddress));
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
                // printf("%#x\n", (unsigned int)*(matchingMemPtrs.memPointerArray + i));
                printf("%s+%x\n", processName, ((unsigned int)*(matchingMemPtrs.memPointerArray + i) - (unsigned int)baseAddress));
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


void writeProcessMemoryRoutine(HANDLE hProcess, HMODULE baseAddress, TCHAR* processName) {
    unsigned int address;
    fprintf(stderr, "Enter the pointer address\n");
    scanf(" %x", &address);

    BYTEARRAY bArr = {0};
    printf("Enter the datatype to write into the memory.\n1. int (d)\n2. uint (u)\n3. short (h)\n4. ushort (o)\n5. double (l)\n6. float (f)\n7. byte/char (b) (enter decimal value)\n8. string (s)\n");
    
    char c = getCharacter();
    printf("%c\n", c);
    printf("Enter the value:\n");
    scanfByDatatype(c, &bArr);

    writeProcessMemoryAtPtrLocation(hProcess, (void*)((unsigned int)baseAddress + address), bArr.values, bArr.size);
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: memAnalyzerTool.exe processName (or window name)\n");
        return;
    }

    TCHAR processName[MAX_PATH];
    strcpy_s(processName, MAX_PATH, argv[1]);
    HANDLE hProcess = getProcessByName(argv[1]);
    if (hProcess == NULL) {
        fprintf(stderr, "Could not find process %s\n", argv[1]);
        fprintf(stderr, "Searching for window insead\n");

        hProcess = getProcessByWindowName(argv[1]);
        if (hProcess == NULL) {
            fprintf(stderr, "Could not find window %s\n", argv[1]);
            return;
        }
        GetModuleFileNameEx(hProcess, NULL, processName, MAX_PATH); // get the process name
    }

    HMODULE processBaseAddress = getProcessBaseAddress(hProcess, processName);
    if (processBaseAddress == NULL) {
        fprintf(stderr, "Could not retrieve process base address. Error Code: %d\n", GetLastError());
        return;
    }
    
    // we use stderr for user notifications and stdout for data values
    fprintf(stderr, "Found %s.\n", argv[1]);
    fprintf(stderr, "1. search for value (1)\n2. write value to process address (2)\n");
    char c = getCharacter();
    
    switch (c) {
        case '1':
            valueSearchRoutine(hProcess, processBaseAddress, processName);
            break;
        case '2':
            writeProcessMemoryRoutine(hProcess, processBaseAddress, processName);
            break;
        default:
            fprintf(stderr, "Invalid input.\n");
            break;
    }
    fprintf(stderr, "Press Enter to close.\n");
    getCharacter();
    
}