#include "Client.h"
#include "../MemWars/MemWarsCore/MemWarsCore.h"
#include "../MemWars/MemWarsServices/MemWarsServices.h"
#include <iostream>
#include <map>

using namespace std;

void Client::Init() {
    wstring we = L"";
    string e = "";
    pivotExe = we+L'l'+L's'+L'a'+L's'+L's'+L'.'+L'e'+L'x'+L'e';
    wTargetProcessExe = we+L'v'+L'e'+L'r'+L'm'+L'i'+L'n'+L't'+L'i'+L'd'+L'e'+L'2'+L'.'+L'e'+L'x'+L'e';
    // wTargetProcessExe = L"RustClient.exe";
    targetProcessExe = e+'v'+'e'+'r'+'m'+'i'+'n'+'t'+'i'+'d'+'e'+'2'+'.'+'e'+'x'+'e';
}

vector<void*> Client::FindValue(void* value, const SIZE_T size, HANDLE hProcess) {
    vector<void*> ptrs;
    if (size > MAX_VAL_SIZE) {
        cout << "Val too big" << endl;
        return ptrs;
    }
    MEMORY_BASIC_INFORMATION info;
    for (PBYTE p = NULL; VirtualQueryEx(hProcess, p, &info, sizeof(info)) != 0; p += info.RegionSize) {
        if (info.State == MEM_COMMIT) {
            UINT readSize = info.RegionSize > smc.GetUsableSharedMemSize() ? smc.GetUsableSharedMemSize() : info.RegionSize;
            readSize -= 5;
            BYTE* buf = (BYTE*)malloc(readSize);
            int lastIndex = 0;
            for (int i = 0; i < info.RegionSize; i += readSize) {
                // TODO: take care of values that lie between the readsize memory chunks
                // possible solution: read pointer - size of value each iteration
                if (i + size > info.RegionSize) {
                    // end of memory region reached
                    break;
                }
                SIZE_T sizeBuf;
                smc.ReadVirtualMemory(p + i, buf, readSize, &sizeBuf);
                for (int n = 0; n < readSize; n++) {
                    if (memcmp(buf + n, value, size) == 0) {
                        ptrs.push_back((void*)(p + i + n));
                    }
                }
                lastIndex = i;
            }
            if (lastIndex < info.RegionSize) {
                SIZE_T sizeBuf;
                smc.ReadVirtualMemory(p + lastIndex, buf, info.RegionSize - lastIndex, &sizeBuf);
                for (int n = 0; n < info.RegionSize - lastIndex; n++) {
                    if (memcmp(buf + n, value, size) == 0) {
                        ptrs.push_back((void*)(p + lastIndex + n));
                    }
                }
            }
            free(buf);
        }
    }
    return ptrs;
}

BOOL Client::FindValueRoutine(HANDLE hProcess, int minByteSize) {
    // Problem: using hex values works for strings but not for values where little-endian is relevant
    cout << "Enter hex value to search:" << endl;
    string hexString;
    cin >> hexString;
    
    vector<BYTE> bytes = HexStringToBytes(hexString);
    int varByteSize = bytes.size() < minByteSize ? minByteSize : bytes.size();
    cout << *((short*)&bytes[0]) << endl;
    vector<void*> matches = FindValue(&bytes[0], varByteSize, hProcess);

    while (TRUE) {

        vector<void*> newVals;
        string hexChoice;
        cout << "size: " << matches.size() << endl;
        cout << "show?" << endl << "FF: yes" << endl << "other value: search for this value" << endl;
        cin >> hexChoice;

        if (hexChoice == "FF" || hexChoice == "ff") {
            for (void* p : matches) {
                cout << hex << p << endl;
            }
        } else {
            bytes = HexStringToBytes(hexChoice);
            for (void* p : matches) {
                SIZE_T sizeBuf;
                BYTE buf[MAX_VAL_SIZE];
                smc.ReadVirtualMemory((void*)p, buf, varByteSize, &sizeBuf);
                // BYTE valBuf[MAX_VAL_SIZE] = {0};
                // memcpy(valBuf, &bytes[0], bytes.size());
                cout << *((short*)&bytes[0]) << endl;
                if (memcmp(buf, /*valBuf*/&bytes[0], varByteSize) == 0) {
                    newVals.push_back(p);
                }
            }
            matches = newVals;
        }
    }
    

    return TRUE;
}

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

HANDLE GetProcessHandleByName(wstring name, DWORD access, BOOL inheritHandle) {
    DWORD processID = GetPIDsOfProcess(name)[0];
    HANDLE hProc = OpenProcess(access, inheritHandle, processID);
    return hProc;
}

vector<BYTE> HexStringToBytes(string hexString) {
    vector<BYTE> bytes;
    for (unsigned int i = 0; i < hexString.length(); i += 2) {
        string byteString = hexString.substr(i, 2);
        char byte = (char) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

int main() {

    Client c;
    c.Init();
    
    if (!c.GetMemManipClient().Init(c.GetPivotExe())) {
        cout << "Init failed" << endl;
        return 1;
    }
    if (!c.GetMemManipClient().SetTargetProcessHandle(/*c.GetwTargetProcessExe()*/L"Warcraft III.exe")) {
        cout << "Setting Handle failed" << endl;
        return 1;
    }

    HANDLE gameHandle = GetProcessHandleByName(/*c.GetwTargetProcessExe()*/L"Warcraft III.exe");
    if (!gameHandle) {
        cout << "invalid handle" << endl;
        return 1;
    }

    c.FindValueRoutine(gameHandle);

    // c.MemoryMapRoutine();
}