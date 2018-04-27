#include "SPIAttackProvider.h"
#include "../../../Core/MemWarsCore.h"
#include "../../../Core/MemWarsServicesCore.h"
#include <iostream>
// #include <cstdlib>

using namespace std;

void ReadWriteValueTest() {
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessHandleByName(L"SkypeApp.exe");
        if (process == NULL) {
            cout << "Open Skype to start testing..." << endl;
            Sleep(5000);
        }
    }

    SPIAttackProvider ap;
    ap.Init(L"SkypeApp.exe", L"lsass.exe");

    BYTEARRAY bArr;
    IntToByteArray(&bArr, 1337);
    MEMPTRS testPtrs = {0};
    FindValueInProcess(&bArr, process, &testPtrs);

    if (testPtrs.size <= 0) {
        cout << "ReadWriteValueTest() failed. Test Value not found" << endl;
        return;
    }

    int readBuf;
    ap.ReadProcessMemory(testPtrs.memPointerArray[0], &readBuf, sizeof(int));

    if (readBuf != 1337) {
        cout << "ReadWriteValueTest() failed. ReadProcessMemory returns wrong value." << endl;
        return;
    }

    int writeBuf = 7331;
    ap.WriteProcessMemory(testPtrs.memPointerArray[0], &writeBuf, sizeof(int));
    ap.ReadProcessMemory(testPtrs.memPointerArray[0], &readBuf, sizeof(int));

    if (readBuf != 7331) {
        cout << "ReadWriteValueTest() failed. WriteProcessMemory did not write." << endl;
        return;
    }

    cout << "ReadWriteValueTest() success" << endl;
    
}

int main () {
    ReadWriteValueTest();
}