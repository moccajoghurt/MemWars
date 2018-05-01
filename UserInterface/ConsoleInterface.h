#pragma once

#include <string>
#include <vector>

using namespace std;

void ManualProcessManipulationRoutine();
void FindValueRoutine(string attackMethod, wstring targetProcess, wstring pivotProcess = L"");
BOOL RequestUserValueInput(void* value, SIZE_T& valSize);
vector<BYTE> HexStringToBytes(string hexString);