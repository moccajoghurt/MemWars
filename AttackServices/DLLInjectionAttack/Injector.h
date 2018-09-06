#pragma once
#include <windows.h>

int LoadDll(HANDLE hProcess, const WCHAR* dllPath);
int LoadDllNoShellcode(HANDLE hProcess, const WCHAR* dllPath);