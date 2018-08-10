#include <string>
#include "../AttackProvider/AttackProvider.h"
#include "../../AttackServices/ThreadHijackAttack/ThreadHijack.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;

class ThreadHijackProvider : public AttackProvider {
public:
    ThreadHijackProvider(){}
    bool SetTargetProcessByName(const string);
    bool HijackThread();
    
protected:
    HANDLE hProcess = NULL;
    wstring processNameW = L"";
    string processName = "";
};