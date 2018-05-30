#include <windows.h>
#include "Direct3DHook.h"
#include "../DLLInjectionAttack/Injector.h"

using namespace std;


BOOL LoadDirect3DDll(HANDLE hProcess, const WCHAR* dllPath) {
    return LoadDll(hProcess, dllPath);
}