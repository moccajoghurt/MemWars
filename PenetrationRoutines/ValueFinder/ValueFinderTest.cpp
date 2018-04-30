#include <iostream>
#include "ValueFinder.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;

void FindValueUsingVirtualAllocTest() {
    HANDLE process = NULL;
    while (process == NULL) {
        process = (HANDLE)GetProcessHandleByName(L"SkypeApp.exe");
        if (process == NULL) {
            cout << "Open Skype to start testing..." << endl;
            Sleep(5000);
        }
    }
    ValueFinder vf;
    if (!vf.Init("SPI", L"SkypeApp.exe", L"lsass.exe")) {
        cout << "FindValueUsingVirtualAllocTest() failed. Init failed" << endl;
        return;
    }

    BYTEARRAY bArr;
    IntToByteArray(&bArr, 1337);
    MEMPTRS testPtrs = {0};
    FindValueInProcess(&bArr, process, &testPtrs);

    if (testPtrs.size <= 0) {
        cout << "ReadWriteValueTest() failed. Test Value not found" << endl;
        return;
    }

    int valBuf = 1337;
    vector<void*> ptrs = vf.FindValueUsingVirtualQuery(&valBuf, sizeof(int), process);

    if (ptrs.size() == testPtrs.size) {
        cout << "ReadWriteValueTest() success" << endl;
    } else {
        cout << "ReadWriteValueTest() failed. Found value amount differs" << endl;
    }
}

int main() {
    FindValueUsingVirtualAllocTest();
}