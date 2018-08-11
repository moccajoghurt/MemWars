#include <iostream>
extern "C" {
#include "lua-5.1.5_Win64/include/lua.h"
#include "lua-5.1.5_Win64/include/lauxlib.h"
#include "lua-5.1.5_Win64/include/lualib.h"
}

#include "../libs/LuaBridge/LuaBridge.h"

#include "../PenetrationRoutines/DLLInjectionProvider/DLLInjectionProvider.h"
#include "../PenetrationRoutines/ThreadHijackProvider/ThreadHijackProvider.h"

using namespace luabridge;
using namespace std;

// class TestClass {
// public:
//     TestClass(){cout << "hi123" << endl;}
//     string TestFunc(const std::string& s) {
//         return "hallo";
//     }
// };

int main() {
    
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
        .addFunction ("InjectDLL", &DLLInjectionProvider::InjectDLL)
    .endClass()
    .deriveClass <ThreadHijackProvider, AttackProvider>("ThreadHijacker")
        .addConstructor<void(*) (void)>()
        .addFunction ("SetTargetProcessByName", &ThreadHijackProvider::SetTargetProcessByName)
        .addFunction ("HijackThread", &ThreadHijackProvider::HijackThread)
    .endClass();





    if (luaL_dofile(L, "script.lua") != 0) {
        cout << "execution failure occured!" << endl;
    }

    lua_close(L);

    // getGlobalNamespace(L)
    // .beginClass<TestClass>("TestClass")
    //     .addConstructor<void(*) (void)>()
    //     .addFunction ("TestFunc", &TestClass::TestFunc)
    // .endClass();
}