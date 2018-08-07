
extern "C" {
#include "lua-5.1.5_Win64/include/lua.h"
#include "lua-5.1.5_Win64/include/lauxlib.h"
#include "lua-5.1.5_Win64/include/lualib.h"
}

#include "../libs/LuaBridge/LuaBridge.h"

#include "../PenetrationRoutines/DLLInjectionProvider/DLLInjectionProvider.h"

#include <iostream>

using namespace luabridge;
using namespace std;

void printMessage(const std::string& s) {
    std::cout << s << std::endl;
}

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
        // .addData ("results", &AttackProvider::results)
    .endClass()
    .deriveClass <DLLInjectionProvider, AttackProvider>("DLLInjector")
        .addConstructor<void(*) (void)>()
        .addFunction ("SetTargetDLL", &DLLInjectionProvider::SetTargetDLL)
        .addFunction ("SetTargetProcessByName", &DLLInjectionProvider::SetTargetProcessByName)
        .addFunction ("ExecuteAttack", &DLLInjectionProvider::ExecuteAttack)
    .endClass();

    // getGlobalNamespace(L)
    // .beginClass<TestClass>("TestClass")
    //     .addConstructor<void(*) (void)>()
    //     .addFunction ("TestFunc", &TestClass::TestFunc)
    // .endClass();


    if (luaL_dofile(L, "script.lua") != 0) {
        cout << "execution failure occured!" << endl;
    }
    // lua_pcall(L, 0, 0, 0);
    // LuaRef sumNumbers = getGlobal(L, "sumNumbers");
    // int result = sumNumbers(5, 4);
    // std::cout << "Result:" << result << std::endl;

    lua_close(L);

    // system("pause");
}