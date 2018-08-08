#pragma once;
#include <windows.h>
#include <string>

using namespace std;

class AttackProvider {
public:
    AttackProvider(){}
    string GetAttackResults() {
        return results;
    };
protected:
    string results = "";
};