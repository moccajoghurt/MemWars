#pragma once
#include <Windows.h>

#define SeLoadDriverPrivilege 10ull
#define SystemModuleInformation 0xBull
#define AdjustCurrentProcess 0ull

using fnFreeCall = uint64_t(__fastcall*)(...);
template<typename ...Params>
static NTSTATUS __NtRoutine(const char* Name, Params &&... params) {
	auto fn = (fnFreeCall) GetProcAddress(GetModuleHandleA("ntdll.dll"), Name);
	return fn(std::forward<Params>(params)...);
}

#define RtlAdjustPrivilege(...) __NtRoutine("RtlAdjustPrivilege", __VA_ARGS__)
#define NtLoadDriver(...) __NtRoutine("NtLoadDriver", __VA_ARGS__)
#define NtUnloadDriver(...) __NtRoutine("NtUnloadDriver", __VA_ARGS__)