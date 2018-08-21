#include <string>
#include "KernelDLLInjectionProvider.h"
#include "../AttackProvider/AttackProvider.h"
#include "../../AttackServices/HiddenKernelDLLInjectionAttack/Injector.h"
#include "../../Core/MemWarsServicesCore.h"

using namespace std;


int main() {
    KernelDLLInjectionProvider ki;
    ki.SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/DLLInjectionAttack/InjectedDLL.dll");
    ki.LoadDLLIntoKernel();
    cout << ki.GetAttackResults();
    // start the target program
    ki.InjectDLLIntoTargetProcess("TestApp.exe");
    // ki.InjectDLL();
    cout << ki.GetAttackResults();
}