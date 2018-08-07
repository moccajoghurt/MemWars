#include <string>
#include "../AttackProvider/AttackProvider.h"
#include "../../AttackServices/DLLInjectionAttack/Injector.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;

class DLLInjectionProvider : public AttackProvider {
public:
    DLLInjectionProvider(){}
    bool SetTargetProcessByName(const string);
    bool ExecuteAttack();
    bool SetTargetDLL(const string);
    
protected:
    wstring dllPath = L"";
    HANDLE hProcess = NULL;
};