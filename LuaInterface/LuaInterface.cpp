#include <iostream>
extern "C" {
#include "lua-5.1.5_Win64/include/lua.h"
#include "lua-5.1.5_Win64/include/lauxlib.h"
#include "lua-5.1.5_Win64/include/lualib.h"
}

#include "../libs/LuaBridge/LuaBridge.h"

#include "../PenetrationRoutines/DLLInjectionProvider/DLLInjectionProvider.h"
#include "../PenetrationRoutines/ThreadHijackProvider/ThreadHijackProvider.h"
#include "../PenetrationRoutines/LsassAttackProvider/Installer/LsassInstallProvider.h"
#include "../PenetrationRoutines/LsassAttackProvider/Client/LsassAttackProvider.h"
#include "../PenetrationRoutines/KernelDLLInjectionProvider/KernelDLLInjectionProvider.h"

using namespace luabridge;
using namespace std;

// class TestClass {
// public:
//     TestClass(){cout << "hi123" << endl;}
//     string TestFunc(const std::string& s) {
//         return "hallo";
//     }
// };

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "usage: LuaInterface.exe MyScript.lua" << endl;
        return 1;
    }

    if (!PathFileExists(argv[1])) {
        cout << argv[1] << " does not exist." << endl;
        return 1;
    }
    
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);


    getGlobalNamespace(L)
    .beginClass<AttackProvider>("AttackProvider")
        .addConstructor<void(*) (void)>()
        .addFunction ("GetAttackResults", &AttackProvider::GetAttackResults)
    .endClass()
    .deriveClass <DLLInjectionProvider, AttackProvider>("DLLInjector")
        .addConstructor<void(*) (void)>()
        .addFunction ("SetTargetDLL", &DLLInjectionProvider::SetTargetDLL)
        .addFunction ("SetTargetProcessByName", &DLLInjectionProvider::SetTargetProcessByName)
        .addFunction ("RequireConfirmationFile", &DLLInjectionProvider::ForceConfirmationFile)
        .addFunction ("InjectDLL", &DLLInjectionProvider::InjectDLL)
    .endClass()
    .deriveClass <ThreadHijackProvider, AttackProvider>("ThreadHijacker")
        .addConstructor<void(*) (void)>()
        .addFunction ("SetTargetProcessByName", &ThreadHijackProvider::SetTargetProcessByName)
        .addFunction ("HijackThread", &ThreadHijackProvider::HijackThread)
    .endClass()
    .deriveClass <SPIInstallProvider, AttackProvider>("LsassAttackInstaller")
        .addConstructor<void(*) (void)>()
        .addFunction ("Install", &SPIInstallProvider::Install)
    .endClass()
    .deriveClass <SPIAttackProvider, AttackProvider>("LsassAttackClient")
        .addConstructor<void(*) (void)>()
        .addFunction ("SetTargetProcessByName", &SPIAttackProvider::Init)
        .addFunction ("ReadProcessMemory", &SPIAttackProvider::ReadProcessMemory)
        .addFunction ("WriteProcessMemory", &SPIAttackProvider::WriteProcessMemory)
        .addFunction ("GetUsableSharedMemSize", &SPIAttackProvider::GetUsableSharedMemSize)
        .addFunction ("StartAttack", &SPIAttackProvider::StartAttack)
    .endClass()
    .deriveClass <KernelDLLInjectionProvider, AttackProvider>("HiddenKernelDLLInjector")
        .addConstructor<void(*) (void)>()
        .addFunction ("SetTargetDLL", &KernelDLLInjectionProvider::SetTargetDLL)
        .addFunction ("LoadDLLIntoKernel", &KernelDLLInjectionProvider::LoadDLLIntoKernel)
        .addFunction ("InjectDLLIntoProcess", &KernelDLLInjectionProvider::InjectDLLIntoTargetProcess)
    .endClass()
    ;


    if (luaL_dofile(L, argv[1]) != 0) {
        // execution failure occured, print error
        cout << lua_tostring(L, -1) << endl;
    }

    lua_close(L);

    // getGlobalNamespace(L)
    // .beginClass<TestClass>("TestClass")
    //     .addConstructor<void(*) (void)>()
    //     .addFunction ("TestFunc", &TestClass::TestFunc)
    // .endClass();
}