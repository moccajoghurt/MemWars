#include <iostream>
#include <string>
#include "ConsoleInterface.h"
#include "../PenetrationRoutines/ValueFinder/ValueFinder.h"

using namespace std;

void FindValueRoutine(string attackMethod, wstring targetProcess, wstring pivotProcess) {
    if (attackMethod == "SPI") {
        cout << "Enter pivot process name" << endl;
        wcin >> pivotProcess;
    }
    ValueFinder vf;
    vf.Init(attackMethod, targetProcess, pivotProcess);
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

    cout << "Enter the target process name" << endl;
    wstring targetProcess;
    wcin >> targetProcess;

    if (attackMethod == 1) {
        cout << "not implemented yet" << endl;
        return;
    } else if (attackMethod == 2 && operation == 1) {
        FindValueRoutine("SPI", targetProcess);
    }

}

int main() {
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