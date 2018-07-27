#include <windows.h>

using namespace std;

int main() {

    TlsAlloc();

    while (true) {

        TlsGetValue(0);
        Sleep(50);
    }

}