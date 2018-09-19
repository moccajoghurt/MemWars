#include <string>
#include <Windows.h>
#include "../AttackProvider/AttackProvider.h"
#include "../../AttackServices/DLLInjectionAttack/Injector.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;

struct INJECTION_DATA {
    wstring dllPath;
    HANDLE hProcess;
    BOOL useShellcode;
};

DWORD StartThreadedInjection(LPVOID param);
bool DeleteConfirmationFile();

class DLLInjectionProvider : public AttackProvider {
public:
    DLLInjectionProvider(){}
    bool SetTargetProcessByName(const string);
    bool SetTargetDLL(const string);
    void RequireConfirmationFile();
    void SetTimeout(int milliSeconds);
    bool InjectDLL();
    bool AssertCompatible(); // possible addition
    
protected:
    int timeout = 0;
    wstring dllPath = L"";
    HANDLE hProcess = NULL;
    string processName = "";
    bool requireConfirmationFile = FALSE;
};
