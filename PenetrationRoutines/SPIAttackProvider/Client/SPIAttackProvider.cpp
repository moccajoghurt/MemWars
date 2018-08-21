#include <iostream>
#include <map>
#include <string>
#include <windows.h>
#include <sstream>

#include "SPIAttackProvider.h"
#include "../../../Core/MemWarsCore.h"
#include "../../../Core/MemWarsServicesCore.h"


using namespace std;

vector<BYTE> SPIAttackProvider::HexStringToBytes(string hexString) {
    vector<BYTE> bytes;
    for (unsigned int i = 0; i < hexString.length(); i += 2) {
        string byteString = hexString.substr(i, 2);
        char byte = (char) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

string SPIAttackProvider::ReadProcessMemory(string s_address, int readSize) {
    uint64_t address = _strtoui64(s_address.c_str(), NULL, 16);
    void* readBuf = calloc(MAX_VAL_SIZE, 1);
    SIZE_T bytesRead;
    NTSTATUS status = smc.ReadVirtualMemory((void*)address, readBuf, readSize, &bytesRead);
    if (status != 0xFFFFFFFF) {
        results += "[+] lsass.exe called NtReadVirtualMemory() on ";
        results += this->targetProcess;
        results += ".\n";
        stringstream sstream;
        for (int i = 0; i < bytesRead; i++) {
            sstream << hex << (uint64_t)*((uint64_t*)readBuf + i);
        }
        return sstream.str();
    }
    results += "[-] ReadProcessMemory() call on ";
    results += this->targetProcess;
    results += " failed.\n";
    return "";
}
unsigned int SPIAttackProvider::WriteProcessMemory(string s_address, string hexSequence) {
    uint64_t address = _strtoui64(s_address.c_str(), NULL, 16);
    vector<BYTE> bytes = HexStringToBytes(hexSequence);
    SIZE_T bytesWritten = 0;
    // uint64_t val = *(uint64_t*)&bytes[0];
    // cout << "test " << hex << val << endl;
    NTSTATUS status = smc.WriteVirtualMemory((void*)address, (void*)&bytes[0], bytes.size(), &bytesWritten);
    if (status != 0xFFFFFFFF) {
        results += "[+] lsass.exe called NtWriteVirtualMemory() on ";
        results += this->targetProcess;
        results += ".\n";
        bytesWritten = bytesWritten == bytes.size() ? bytesWritten : 0;
        return bytesWritten;
    }
    results += "[-] WriteProcessMemory() call on ";
    results += this->targetProcess;
    results += " failed.\n";
    return 0;
}

bool SPIAttackProvider::Init(string _targetProcess/*, string _pivotProcess*/) {
    this->targetProcess = _targetProcess;
    // this->pivotProcess = pivotProcess;

    wstring targetProcess(_targetProcess.begin(), _targetProcess.end());
    wstring pivotProcess = L"lsass.exe";

    if (!smc.Init(pivotProcess)) {
        results += "[-] Init() failed. Could either not get PID of lsass.exe or connect to communication file mapping.\n";
        return FALSE;
    }
    if (!smc.SetTargetProcessHandle(targetProcess)) {
        results += "[-] Init() failed. HANDLE ID of ";
        results += _targetProcess;
        results += " inside lsass.exe not found.\n";
        results += "[-] Note that lsass.exe only has HANDLE IDs to processes that use socket functions.\n";
        results += "[-] You cannot use this attack method on all processes.\n";
        return FALSE;
    }
    return TRUE;
}


// int main() {

    
    
//     size_t size = 0;
//     SPIAttackProvider spi;
//     if (!spi.Init("SkypeApp.exe")) {
//         cout << spi.GetAttackResults() << endl;
//         return 1;
//     }
//     size = spi.WriteProcessMemory("0x1c921120000", "BEEF");
//     cout << size << endl;
//     string a = spi.ReadProcessMemory("0x1c921120000", sizeof(int));
//     cout << a << " # " << hex << atoi(a.c_str()) << endl;
//     cout << spi.GetAttackResults() << endl;


//     HANDLE process = NULL;
//     while (process == NULL) {
//         process = (HANDLE)GetProcessByName("SkypeApp.exe");
//         if (process == NULL) {
//             cout << "Open Skype to start testing..." << endl;
//             Sleep(5000);
//         }
//     }

//     BYTEARRAY bArr = {0};
//     ReadProcessMemoryAtPtrLocation((void*)0x000000D0F2EFC553, sizeof(int), process, &bArr);

//     cout << hex << (int)bArr.values[0] << endl;
// }