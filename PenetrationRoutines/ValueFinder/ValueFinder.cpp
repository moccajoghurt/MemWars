#include "ValueFinder.h"

using namespace std;

BOOL ValueFinder::Init(string attackMethod, wstring targetProcess, wstring pivotProcess) {
    this->attackMethod = attackMethod;
    this->targetProcess = targetProcess;
    this->pivotProcess = pivotProcess;
    return TRUE;
}

