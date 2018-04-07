#include <windows.h>
int main() {
    CreateFile("hack.txt",                // name of the write
        GENERIC_WRITE,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        CREATE_NEW,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);  
}