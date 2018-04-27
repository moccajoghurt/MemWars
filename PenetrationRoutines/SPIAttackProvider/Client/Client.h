#pragma once;
#include <string>
#include <map>
#include <vector>
#include "../../../Core/MemWarsCore.h"
#include "../../../AttackServices/SystemProcessInjectionAttack/StealthyMemManipulatorClient.h"
#include "../../../AttackServices/SystemProcessInjectionAttack/StealthyMemManipulatorGetHandleId.h"

#define MAX_VAL_SIZE 255

class Client {
public:
    void Init();
    vector<void*> FindValue(void* value, SIZE_T size, HANDLE hProcess);
    BOOL FindValueRoutine(HANDLE hProcess, int minByteSize = 0);
    map<uintptr_t, BYTE> GetMemoryMap(uintptr_t startAddress, uintptr_t endAddress);
    BOOL MemoryMapRoutine(uintptr_t startAddress, uintptr_t endAddress);
    StealthyMemClient& GetMemManipClient() {
        return smc;
    }
    wstring GetPivotExe() {
        return pivotExe;
    }
    wstring GetwTargetProcessExe() {
        return wTargetProcessExe;
    }
    string GetTargetProcessExe() {
        return targetProcessExe;
    }
    void SetBaseAddress(uintptr_t baseAddress) {
        this->baseAddress = baseAddress;
    }
protected:
    wstring pivotExe;
    wstring wTargetProcessExe;
    string targetProcessExe;
    uintptr_t baseAddress = 0x140000000;
    StealthyMemClient smc;
};

HANDLE GetProcessHandleByName(wstring name, DWORD access = PROCESS_ALL_ACCESS, BOOL inherit = FALSE);
vector<BYTE> HexStringToBytes(string hexString);

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