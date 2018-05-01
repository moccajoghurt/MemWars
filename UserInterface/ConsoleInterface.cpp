#include <iostream>
#include <string>
#include <windows.h>
#include <vector>
#include "ConsoleInterface.h"
#include "../PenetrationRoutines/ValueFinder/ValueFinder.h"

using namespace std;

vector<BYTE> HexStringToBytes(string hexString) {
    vector<BYTE> bytes;
    for (unsigned int i = 0; i < hexString.length(); i += 2) {
        string byteString = hexString.substr(i, 2);
        char byte = (char) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

BOOL RequestUserValueInput(void* value, SIZE_T& valSize) {
    cout << "Enter the value datatype:" << endl
    << "(1) Byte" << endl
    << "(2) 2 Bytes" << endl
    << "(3) 4 Bytes" << endl
    << "(4) 8 Bytes" << endl
    << "(5) Float" << endl
    << "(6) Double" << endl
    << "(7) String" << endl
    << "(8) Bytearray" << endl;
    string choice;
    cin >> choice;
    cout << "Enter value:" << endl;
    if (choice == "1") {
        BYTE b;
        cin >> b;
        memcpy(value, &b, sizeof(BYTE));
        valSize = sizeof(BYTE);
    } else if (choice == "2") {
        WORD w;
        cin >> w;
        memcpy(value, &w, sizeof(WORD));
        valSize = sizeof(WORD);

    } else if (choice == "3") {
        DWORD dw;
        cin >> dw;
        memcpy(value, &dw, sizeof(DWORD));
        valSize = sizeof(DWORD);

    } else if (choice == "4") {
        DWORD64 dw64;
        cin >> dw64;
        memcpy(value, &dw64, sizeof(DWORD64));
        valSize = sizeof(DWORD64);

    } else if (choice == "5") {
        float f;
        cin >> f;
        memcpy(value, &f, sizeof(float));
        valSize = sizeof(float);

    } else if (choice == "6") {
        double d;
        cin >> d;
        memcpy(value, &d, sizeof(double));
        valSize = sizeof(double);

    } else if (choice == "7") {
        string s;
        cin >> s;
        if (s.size() > MAX_VAL_SIZE) {
            cout << "MAX_VAL_SIZE exceeded: " << MAX_VAL_SIZE << " " << s.size() << endl;
            return FALSE;
        }
        memcpy(value, &s, s.size());
        valSize = s.size();

    } else if (choice == "8") {
        cout << "Enter the bytearray as hex values (no 0x required)" << endl;
        string s;
        cin >> s;
        vector<BYTE> bytes = HexStringToBytes(s);
        if (bytes.size() > MAX_VAL_SIZE) {
            cout << "MAX_VAL_SIZE exceeded: " << MAX_VAL_SIZE << " " << bytes.size() << endl;
            return FALSE;
        }
        memcpy(value, &bytes[0], bytes.size());
        valSize = bytes.size();
    }
    return TRUE;
}

void FindValueRoutine(string attackMethod, wstring targetProcess, wstring pivotProcess) {
    if (attackMethod == "SPI") {
        cout << "Enter pivot process name" << endl;
        wcin >> pivotProcess;
    }
    ValueFinder vf;
    if (!vf.Init(attackMethod, targetProcess, pivotProcess)) {
        cout << "Init failed" << endl;
        return;
    }
    void* valBuf = malloc(MAX_VAL_SIZE);
    SIZE_T valSize;
    if (!RequestUserValueInput(valBuf, valSize)) {
        return;
    }
    vector<void*> ptrs = vf.FindValueUsingVirtualQuery(valBuf, valSize);
    while (TRUE) {
        cout << "Found pointers: " << ptrs.size() << endl
        << "(1) Show pointers" << endl
        << "(2) Enter new value and remove pointers that don't match it" << endl;
        string choice;
        cin >> choice;
        if (choice == "1") {
            for (void* ptr : ptrs) {
                cout << ptr << endl;
            }
    
        } else if (choice == "2") {
            if (!RequestUserValueInput(valBuf, valSize)) {
                return;
            }
            vf.RemoveNotMatchingValues(ptrs, valBuf, valSize);
        }
    }
    
}

void ManualProcessManipulationRoutine() {
    cout << "Choose the attack method: " << endl
    << "(1) No bypass attack" << endl
    << "(2) System Process Injection attack" << endl;
    int attackMethod;
    cin >> attackMethod;
    
    cout << "Choose the operation:" << endl
    << "(1) Find a specific value in the process" << endl
    << "(2) Read a value at a target address" << endl
    << "(3) Write a value at a target address" << endl;
    int operation;
    cin >> operation;

    cout << "Enter the target process name:" << endl;
    wstring targetProcess;
    // wcin >> targetProcess;
    cin.ignore(); // ignore pending enter
    getline(wcin, targetProcess);

    if (attackMethod == 1) {
        cout << "not implemented yet" << endl;
        return;
    } else if (attackMethod == 2 && operation == 1) {
        FindValueRoutine("SPI", targetProcess);
    }

}

int main() {
    cout << sizeof(WORD) << endl;
    // TODO: ValueFinder implementieren -> SPIAttackProvider Client anpassen & NoBypass Client implementieren
    cout << "Welcome to the MemWars Game Penetration Framework!" << endl;
    cout << "Choose an operation: " << endl
    << "(1) Automated penetration test of an application" << endl
    << "(2) Manual process manipulation" << endl;
    int choice;
    cin >> choice;
    if (choice == 1) {
        cout << "not implemented yet" << endl;
        return 1;
    } else if (choice == 2) {
        ManualProcessManipulationRoutine();
    }
    return 0;
}