#include <windows.h>
#include <stdio.h>
#include <wchar.h>

int main(int argc, char* argv[]) {
    int value1 = 133337;
    int value2 = 0xB00B;
    long value3 = 0xC0FEFE;
    float value4 = 1.375;
    double value5 = 312.76493;
    char* str1 = "Hello, can you find me?";
    const char* str2 = "Can you find me too?";
    char str3[] = "And me??";
    wchar_t* str4 = L"Try finding wide chars...";
    
    if (argc > 1 && strstr(argv[1], "-window") != 0) {
        for (;;) {
            char msg[] = "The value is: xxxxx";
            sprintf(msg + strlen("The value is: "), "%d", value1);
            MessageBox(0, msg, "manipulateMe", MB_OK);
        }
    } else {
        for (;;) {
            // just wait to be manipulated
        }
    }
}