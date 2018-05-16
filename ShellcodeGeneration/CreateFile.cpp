#include <windows.h>
#include <iostream>

using namespace std;

int main() {
    
    CreateFileA("Test.txt",         // lpFileName
    GENERIC_READ | GENERIC_WRITE,   // dwDesiredAccess
    0,                              // dwShareMode
    NULL,                           // lpSecurityAttributes
    CREATE_ALWAYS,                  // dwCreationDisposition
    FILE_ATTRIBUTE_NORMAL,          // dwFlagsAndAttributes
    NULL                            // hTemplateFile 
    );
}