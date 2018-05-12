#include <iostream>
#include "../../Core/MemWarsCore.h"
#include "ThreadHijack.h"

using namespace std;

void ThreadHijackTest() {
    system("start /B memoryTestApp.exe");
    HANDLE hProcess = NULL;
    while (hProcess == NULL) {
        hProcess = (HANDLE)GetProcessByName("memoryTestApp.exe");
    }

    

    Exit:
    system("taskkill /IM memoryTestApp.exe /F >nul");
}

int main() {
    ThreadHijackTest();
}