#include <windows.h>
#include <string>
#include <vector>
#include "../SPIAttackProvider/Client/Client.h"

class ValueFinder {
public:
    BOOL Init(string attackMethod, wstring targetProcess, wstring pivotProcess = L"");
    vector<void*> FindValues();

protected:
    string attackMethod;
    wstring targetProcess;
    wstring pivotProcess;
};