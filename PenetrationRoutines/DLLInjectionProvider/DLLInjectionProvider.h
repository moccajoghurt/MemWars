#include <string>
#include <Windows.h>
#include "../AttackProvider/AttackProvider.h"
#include "../../AttackServices/DLLInjectionAttack/Injector.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;

class DLLInjectionProvider : public AttackProvider {
public:
    DLLInjectionProvider(){}
    bool SetTargetProcessByName(const string);
    bool InjectDLL();
    bool SetTargetDLL(const string);
    bool AssertCompatible(); // possible addition
    
protected:
    wstring dllPath = L"";
    HANDLE hProcess = NULL;
    string processName = "";

};
