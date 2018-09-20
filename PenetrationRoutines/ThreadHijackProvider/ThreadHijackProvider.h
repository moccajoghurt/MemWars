#include <string>
#include "../AttackProvider/AttackProvider.h"
#include "../../AttackServices/ThreadHijackAttack/ThreadHijack.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;

struct HIJACK_DATA {
    HANDLE hProcess;
    DWORD tid;
};

bool DeleteHijackConfirmationFile();
DWORD StartThreadedHijack(LPVOID param);

class ThreadHijackProvider : public AttackProvider {
public:
    ThreadHijackProvider(){}
    bool SetTargetProcessByName(const string);
    void SetTimeout(int milliSeconds);
    bool HijackThread();
    
protected:
    int timeout = 0;
    HANDLE hProcess = NULL;
    wstring processNameW = L"";
    string processName = "";
};