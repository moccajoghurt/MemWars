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
    BOOL Init(wstring targetProcess, wstring pivotProcess);
    BOOL ReadProcessMemory(HANDLE hProcess, void* address, void* readBuf, SIZE_T readSize, SIZE_T* bytesRead = NULL);
    BOOL WriteProcessMemory(HANDLE hProcess, void* address, void* writeBuf, SIZE_T writeSize, SIZE_T* bytesWritten = NULL);
    SIZE_T GetUsableSharedMemSize() {
        return smc.GetUsableSharedMemSize();
    }
protected:
    wstring pivotProcess;
    wstring targetProcess;
    StealthyMemClient smc;
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