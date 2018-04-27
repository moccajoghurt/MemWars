#pragma once;

#include <windows>

class AttackProvider {
public:
    virtual void ReadProcessMemory() = 0;
    virtual void WriteProcessMemory() = 0;
};