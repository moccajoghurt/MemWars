#pragma once;
#include <string>
#include <map>
#include <vector>
#include "../../../Core/MemWarsCore.h"
#include "../../../AttackServices/SystemProcessInjectionAttack/StealthyMemManipulatorClient.h"
#include "../../../AttackServices/SystemProcessInjectionAttack/StealthyMemManipulatorGetHandleId.h"
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
protected:
    // wstring pivotProcess;
    string targetProcess;
    StealthyMemClient smc;
    vector<BYTE> HexStringToBytes(string hexString);
};


/*
Strings of interest:
lua_*
get_*
get_unit_data
mTarget*
unit_position
unit_indices
unit_rotation
local_rotation
local_position
set_local_position
world_pose
world_rotation
world_position
set_local_pose
update_position_lookup
*/