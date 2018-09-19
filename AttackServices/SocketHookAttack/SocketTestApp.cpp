#include <winsock2.h>
#include <windows.h>

using namespace std;

int main() {

    while (true) {
        SOCKET s;
        send(s, NULL, 0, 0);
        sendto(s, NULL, 0, 0, NULL, 0);
        WSASend(s, NULL, 0, NULL, 0, NULL, NULL);
        WSASendTo(s, NULL, 0, NULL, 0, NULL, 0, NULL, NULL);
        WSASendMsg(s, NULL, 0, NULL, NULL, NULL);

        Sleep(1000);
    }
}