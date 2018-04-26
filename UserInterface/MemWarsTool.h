#ifndef _MEM_ANALYZER_TOOL_H
#define _MEM_ANALYZER_TOOL_H

#include <stdio.h>

size_t scanfByDatatype(char c, BYTEARRAY* bArr);
void valueSearchRoutine(HANDLE hProcess, HMODULE processBaseAddress, TCHAR* processName);
void memorySnapshotRoutine(HANDLE hProcess, HMODULE baseAddress, TCHAR* processName);


#endif