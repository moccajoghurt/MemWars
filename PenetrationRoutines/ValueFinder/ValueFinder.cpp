#include <iostream>
#include <algorithm>
#include "ValueFinder.h"

using namespace std;

BOOL ValueFinder::Init(string attackMethod, wstring targetProcess, wstring pivotProcess) {
    this->attackMethod = attackMethod;
    this->targetProcess = targetProcess;
    this->pivotProcess = pivotProcess;

    hProcess = GetProcessHandleByName(targetProcess, PROCESS_QUERY_INFORMATION);
    if (!hProcess) {
        return FALSE;
    }

    if (attackMethod == "SPI") {
        SPIAttackProvider* spi = new SPIAttackProvider;
        if (!spi->Init(targetProcess, pivotProcess)) {
            return FALSE;
        }
        attackProvider = spi;
    } else {
        return FALSE;
    }

    return TRUE;
}


vector<void*> ValueFinder::FindValueUsingVirtualQuery(void* value, const SIZE_T size, HANDLE hProcess) {
    vector<void*> ptrs;
    if (size > MAX_VAL_SIZE) {
        cout << "Val too big" << endl;
        return ptrs;
    }
    if (hProcess != NULL) {
        this->hProcess = hProcess;
    }
    MEMORY_BASIC_INFORMATION info;
    for (PBYTE p = NULL; VirtualQueryEx(this->hProcess, p, &info, sizeof(info)) != 0; p += info.RegionSize) {
        if (info.State == MEM_COMMIT) {
            PBYTE buf = (PBYTE)malloc(info.RegionSize);
            SIZE_T bytesRead;
            attackProvider->ReadProcessMemory(this->hProcess, p, buf, info.RegionSize, &bytesRead);
            for (int i = 0; i < bytesRead; i++) {
                if (i + size >= bytesRead) {
                    break;
                }
                if (memcmp(buf + i, value, size) == 0) {
                    ptrs.push_back((void*)((DWORD64)p + i));
                }
            }
            free(buf);
        }
    }
    return ptrs;
}

void ValueFinder::RemoveNotMatchingValues(vector<void*>& memPtrs, void* value, SIZE_T size) {

    vector<void*>::iterator it = memPtrs.begin();
    for (; it != memPtrs.end();) {
        PBYTE buf = (PBYTE)malloc(size);
        SIZE_T bytesRead;
        attackProvider->ReadProcessMemory(this->hProcess, *it, buf, size, &bytesRead);
        if (memcmp(buf, value, size) != 0) {
            it = memPtrs.erase(it);
        } else {
            ++it;
        }
        free(buf);
    }
}

/*

map<uintptr_t, BYTE> Client::GetMemoryMap(uintptr_t startAddress = 0, uintptr_t endAddress = 0) {
    map<uintptr_t, BYTE> memoryMap;
    for (uintptr_t i = startAddress; i < endAddress; i++) {
        SIZE_T sizeBuf;
        int buf;
        smc.ReadVirtualMemory((void*)(i), &buf, sizeof(BYTE), &sizeBuf);
        memoryMap[i] = buf;
    }
    return memoryMap;
}

BOOL Client::MemoryMapRoutine(uintptr_t startAddress = 0, uintptr_t endAddress = 0) {
    cout << "Creating Map..." << endl;
    map<uintptr_t, BYTE> memoryMap = GetMemoryMap(startAddress, endAddress);

    while (TRUE) {
        int choice;
        cout << "1: keep vals that changed" << endl << "2: keep vals that didn't change" << endl
        << "3: keep vals that got smaller" << endl << "4: keep vals that got bigger" << endl;
        cin >> choice;
        if (choice < 1 || choice > 4) {
            cout << "invalid choice" << endl;
            return FALSE;
        }
        for(map<uintptr_t, BYTE>::iterator i = memoryMap.begin(); i != memoryMap.end();) {
            SIZE_T sizeBuf;
            int buf;
            smc.ReadVirtualMemory((void*)(i->first), &buf, sizeof(BYTE), &sizeBuf);

            if (choice == 1 && i->second == buf) {
                memoryMap.erase(i++);
            } else if (choice == 2 && i->second != buf) {
                memoryMap.erase(i++);
            } else if (choice == 3 && i->second <= buf) {
                memoryMap.erase(i++);
            } else if (choice == 4 && i->second >= buf) {
                memoryMap.erase(i++);
            }  else {
                ++i;
            }
        }
        choice = 1;
        while (choice == 1) {
            cout << "size: " << memoryMap.size() << endl << "1: show" << endl << "other val: no" << endl;
            cin >> choice;
            if (choice == 1) {
                for(map<uintptr_t, BYTE>::iterator i = memoryMap.begin(); i != memoryMap.end();) {
                    cout << hex << i->first << "\t" << dec << i->second << endl;
                    i++;
                }
            }
        }
    }

    return TRUE;
}

*/