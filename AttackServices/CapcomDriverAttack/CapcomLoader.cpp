#include <windows.h>
#include <Shlwapi.h>
#include <Subauth.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "CapcomDriver.h"
#include "NtDefines.h"
#include "CapcomLoader.h"

using namespace std;

void DecryptDriver() {
	if (CAPCOM_DRIVER[0] != 0x4D) {
		for (BYTE& b : CAPCOM_DRIVER) {
            b ^= CAPCOM_DRIVER_XOR_KEY;
        }
	}
}

wstring CreateDriverName() {
    srand(__rdtsc());
    wstring driverName = L"";
    for (int i = 0; i < 12; i++) {
        driverName += wchar_t(L'A' + rand() % 20);
    }
    return driverName;
    // return L"capcom";
}

wstring GetDriverPath() {
	wchar_t systemDirectory[2048];
	GetSystemDirectoryW(systemDirectory, 2048);

	wstring driverPath = systemDirectory;
	driverPath += L"\\drivers\\";

	return driverPath;
}

BOOL CreateDriverFile(const wchar_t* capcomDriverName) {
    wstring driverPath = GetDriverPath() + capcomDriverName + L".sys";
    ofstream driverFile(driverPath, ios::binary);
	if (!driverFile.good()) {
		cout << "Failed to create driver file!" << endl;
		return FALSE;
    }
    driverFile.write((char*)CAPCOM_DRIVER, sizeof(CAPCOM_DRIVER));
    driverFile.close();
    return TRUE;
}

NTSTATUS AddServiceToRegistry(const wchar_t* driverName) {
    NTSTATUS status = STATUS_SUCCESS;

    wstring registryPath = wstring(L"System\\CurrentControlSet\\Services\\") + driverName;

    RemoveDriverFromRegistry(driverName);

    HKEY key;
    status = RegCreateKeyExW(HKEY_LOCAL_MACHINE, registryPath.c_str(), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, 0);

    if (status != ERROR_SUCCESS) {
        return status;
    }
    wstring driverPath = wstring(L"\\SystemRoot\\System32\\drivers\\") + driverName + L".sys";
    DWORD value = 1;
    status |= RegSetValueExW(key, L"ImagePath", 0, REG_EXPAND_SZ, (PBYTE)driverPath.c_str(), driverPath.size() * sizeof(wchar_t));
    status |= RegSetValueExW(key, L"Type", 0, REG_DWORD, (PBYTE)&value, sizeof(DWORD));
    status |= RegSetValueExW(key, L"ErrorControl", 0, REG_DWORD, (PBYTE)&value, sizeof(DWORD));
    value = 3;
    status |= RegSetValueExW(key, L"Start", 0, REG_DWORD, (PBYTE)&value, sizeof(DWORD));

    if (status != ERROR_SUCCESS) {
        RegCloseKey(key);
        RemoveDriverFromRegistry(driverName);
        return status;
    }

    RegCloseKey(key);
    return STATUS_SUCCESS;
}

NTSTATUS RemoveDriverFromRegistry(const wchar_t* driverName) {
	NTSTATUS status = STATUS_SUCCESS;

	wstring RegistryPath = wstring(L"System\\CurrentControlSet\\Services\\") + driverName;

	status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, RegistryPath.c_str());
	if (!status || status == ERROR_FILE_NOT_FOUND) {
        return STATUS_SUCCESS;
    }

	status = SHDeleteKeyW(HKEY_LOCAL_MACHINE, RegistryPath.c_str());
	if (!status || status == ERROR_FILE_NOT_FOUND) {
        return STATUS_SUCCESS;
    }

	status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, RegistryPath.c_str());
	if (!status || status == ERROR_FILE_NOT_FOUND) {
        return STATUS_SUCCESS;
    }

	return status;
}

NTSTATUS TryOpenServiceKey(const wchar_t* driverName) {
	wstring registryPath = wstring(L"System\\CurrentControlSet\\Services\\") + driverName;
	HKEY key;
	NTSTATUS result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, registryPath.c_str(), 0, KEY_ALL_ACCESS, &key);
	RegCloseKey(key);
	return result;
}

NTSTATUS RemoveSimilarDrivers(BYTE* driver) {
	namespace fs = experimental::filesystem;

	wstring driverPath = GetDriverPath();

	NTSTATUS status = STATUS_SUCCESS;

	for (auto& file : fs::directory_iterator(driverPath)) {
		wstring path = file.path();
		if (path.find(L".sys") != -1) {
			ifstream fileStr(path, ios::binary);
			char data[1024];
			fileStr.read(data, 1024);
			fileStr.close();

			if (!memcmp(driver, data, 1024)) {
				bool deleted = DeleteFileW(path.c_str());

				// printf("DeleteFile (%ls) : %x\n", path.c_str(), deleted);

				if (!deleted) {
					int strEnd = path.find(L".sys");
					int strStart = path.rfind(L"\\", strEnd);
					wstring driverName = path.substr(strStart + 1, strEnd - strStart - 1).c_str();
					UnloadDriver(driverName.c_str());

					deleted = DeleteFileW(path.c_str());
					// printf("DeleteFile2 (%ls) : %x\n", path.c_str(), deleted);
				}

				status |= !deleted;
			}
		}
	}

	return status;
}

NTSTATUS UnloadDriver(const wchar_t* driverName) {
	BOOLEAN alreadyEnabled = FALSE;
    if (RtlAdjustPrivilege(SeLoadDriverPrivilege, 1ull, AdjustCurrentProcess, &alreadyEnabled) != STATUS_SUCCESS && !alreadyEnabled) {
        return FALSE;
    }

	if (TryOpenServiceKey(driverName) == 2) {
        AddServiceToRegistry(driverName);
    }
		
	wstring sourceRegistry = wstring(L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\") + driverName;

	UNICODE_STRING sourceRegistryUnicode = {0};
	sourceRegistryUnicode.Buffer = (wchar_t*) sourceRegistry.c_str();
	sourceRegistryUnicode.Length = (sourceRegistry.size()) * 2;
	sourceRegistryUnicode.MaximumLength = (sourceRegistry.size() + 1) * 2;

	NTSTATUS status = NtUnloadDriver(&sourceRegistryUnicode);

	// printf("NtUnloadDriver(%ls) returned %08x\n", sourceRegistry.c_str(), status);

	RemoveDriverFromRegistry(driverName);

	return status;
}

BOOL LoadDriver() {

    DecryptDriver();

    if (RemoveSimilarDrivers(CAPCOM_DRIVER) != STATUS_SUCCESS) {
		// printf("Failed to remove similar drivers!\n");
		return FALSE;
	}
    
    wstring driverName = CreateDriverName();
    
    if (!CreateDriverFile(driverName.c_str())) {
        return FALSE;
    }

    BOOLEAN alreadyEnabled = FALSE;
    if (RtlAdjustPrivilege(SeLoadDriverPrivilege, 1ull, AdjustCurrentProcess, &alreadyEnabled) != STATUS_SUCCESS && !alreadyEnabled) {
        return FALSE;
    }
    
    if (AddServiceToRegistry(driverName.c_str()) != STATUS_SUCCESS) {
        return FALSE;
    }
    
    wstring sourceRegistry = wstring(L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\") + driverName;

    UNICODE_STRING sourceRegistryUnicode = {0};
    sourceRegistryUnicode.Buffer = (wchar_t*) sourceRegistry.c_str();
    sourceRegistryUnicode.Length = (sourceRegistry.size()) * 2;
    sourceRegistryUnicode.MaximumLength = (sourceRegistry.size() + 1) * 2;

    NTSTATUS status = NtLoadDriver(&sourceRegistryUnicode);

    // printf("NtLoadDriver(%ls) returned %08x\n", sourceRegistry.c_str(), status);

    if (status != STATUS_SUCCESS) {
        UnloadDriver(driverName.c_str());
        RemoveDriverFromRegistry(driverName.c_str());
        return FALSE;
    }
    return TRUE;
}

HANDLE OpenDevice(string driverName) {
	char completeDeviceName[128];
	sprintf_s(completeDeviceName, "\\\\.\\%s", driverName.data());

	HANDLE deviceHandle = CreateFileA(
		completeDeviceName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (deviceHandle == INVALID_HANDLE_VALUE) {
        deviceHandle = 0;
    }
	// printf("[+] CreateFileA(%s) returned %p\n", completeDeviceName, deviceHandle);
	return deviceHandle;
}