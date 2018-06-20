#pragma once
#include <windows.h>
#include <string>

using namespace std;

void DecryptDriver();
wstring GetDriverPath();
BOOL CreateDriverFile();
BOOL LoadDriver();
NTSTATUS UnloadDriver(const wchar_t* driverName);
NTSTATUS RemoveSimilarDrivers(BYTE* driver);
wstring CreateDriverName();
NTSTATUS AddServiceToRegistry(const wchar_t* driverName);
NTSTATUS RemoveDriverFromRegistry(const wchar_t* driverName);
NTSTATUS TryOpenServiceKey(const wchar_t* driverName);
HANDLE OpenDevice(string driverName);