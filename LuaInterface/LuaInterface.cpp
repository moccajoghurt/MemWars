
extern "C" {
#include "lua-5.1.5_Win64/include/lua.h"
#include "lua-5.1.5_Win64/include/lauxlib.h"
#include "lua-5.1.5_Win64/include/lualib.h"
}

#include "../libs/LuaBridge/LuaBridge.h"

#include "../PenetrationRoutines/DLLInjectionProvider/DLLInjectionProvider.h"

#include <iostream>

using namespace luabridge;

void printMessage(const std::string& s) {
    std::cout << s << std::endl;
}
 
int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    getGlobalNamespace(L)
    // .addFunction("printMessage", printMessage);
    .beginNamespace ("test")
        .beginClass<DLLInjectionProvider>("AttackProvider")
            .addFunction ("GetAttackResults", &AttackProvider::GetAttackResults)
        .endClass()
        .deriveClass <DLLInjectionProvider, AttackProvider>("DLLInjector")
            .addFunction ("SetTargetDLL", &DLLInjectionProvider::SetTargetDLL)
            .addFunction ("SetTargetProcessByName", &DLLInjectionProvider::SetTargetProcessByName)
            .addFunction ("ExecuteAttack", &DLLInjectionProvider::ExecuteAttack)
        .endClass()
    .endNamespace();
    luaL_dofile(L, "script.lua");
    lua_pcall(L, 0, 0, 0);
    // LuaRef sumNumbers = getGlobal(L, "sumNumbers");
    // int result = sumNumbers(5, 4);
    // std::cout << "Result:" << result << std::endl;



    system("pause");
}