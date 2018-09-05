#pragma once;
#include <string>
#include <map>
#include <vector>
#include "../../../Core/MemWarsCore.h"
#include "../../../AttackServices/LsassAttack/StealthyMemManipulatorClient.h"
#include "../../../AttackServices/LsassAttack/StealthyMemManipulatorGetHandleId.h"
#include "../../AttackProvider/AttackProvider.h"

#define MAX_VAL_SIZE 255

class SPIAttackProvider : public AttackProvider {
public:
    SPIAttackProvider() {}
    bool Init(string targetProcess/*, string pivotProcess*/);
    string ReadProcessMemory(string address, int readSize);
    unsigned int WriteProcessMemory(string address, string hexSequence);
    int GetUsableSharedMemSize() {
        return smc.GetUsableSharedMemSize();
    }
    bool StartAttack();
protected:
    // wstring pivotProcess;
    string targetProcess = "";
    StealthyMemClient smc;
    vector<BYTE> HexStringToBytes(string hexString);
};

