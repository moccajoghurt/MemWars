#include <windows.h>
#include <stdio.h>

int main() {
    int value = 13337;
    for (;;) {
        char msg[] = "The value is: xxxxx";
        sprintf(msg + strlen("The value is: "), "%d", value);
        MessageBox(0, msg, "manipulateMe", MB_OK);
    }
}