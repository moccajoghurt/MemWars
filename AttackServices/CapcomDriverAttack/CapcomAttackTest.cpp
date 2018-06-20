#include <iostream>
#include "CapcomAttack.h"

using namespace std;

void CapcomAttackTest() {
    if (!InitDriver()) {
        cout << "CapcomAttackTest() failed, could not initialize driver" << endl;
        return;
    }
    StartAttack();
    for(;;) {}
}

int main() {
    CapcomAttackTest();
}