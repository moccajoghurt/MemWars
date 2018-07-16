#pragma once
#include <Windows.h>
#include <iostream>

#pragma warning(disable : 4330)
#pragma section(".LDATA", read, write)
#pragma section(".LTEXT", read, write, execute)

#pragma data_seg(".LDATA$1")
#pragma data_seg(".LDATA$2")
#pragma data_seg(".LDATA$3")
#pragma data_seg()

#pragma code_seg(".LTEXT$1")
#pragma code_seg(".LTEXT$2")
#pragma code_seg(".LTEXT$3")
#pragma code_seg()

__declspec(allocate(".LDATA$1")) static char nonPagedDataStart = 0x0;
__declspec(allocate(".LDATA$3")) static char nonPagedDataEnd = 0x0;

__declspec(allocate(".LTEXT$1")) static char nonPagedTextStart = 0x0;
__declspec(allocate(".LTEXT$3")) static char nonPagedTextEnd = 0x0;

#define NtCurrentProcess()(HANDLE(-1))


#define NON_PAGED_DATA  __declspec(allocate(".LDATA$2"))
#define NON_PAGED_CODE __declspec(code_seg(".LTEXT$2")) __declspec(noinline)
#define NON_PAGED_LAMBDA(...)  [](__VA_ARGS__) NON_PAGED_CODE

// Mini non-paged crt
#define NonPagedMemcpy(dst, src, size) __movsb((BYTE*) dst, (const BYTE*) src, size)
#define NonPagedMemset(dst, val, size) __stosb((BYTE*) dst, val, size)
#define NonPagedZeroMemory(dst, size) __stosb((BYTE*) dst, 0, size)

#pragma comment(linker,"/MERGE:.LDATA=.data")
#pragma comment(linker,"/MERGE:.LTEXT=.text")

// Routines to lock the pages
static BOOL TryIncreaseWorkingSetSize(SIZE_T Size) {
	SIZE_T Min, Max;
	if (!GetProcessWorkingSetSize(NtCurrentProcess(), &Min, &Max))
		return FALSE;
	if (!SetProcessWorkingSetSize(NtCurrentProcess(), Min + Size, Max + Size))
		return FALSE;
	// printf("[+] Increasing working set (%d KB, %d KB) -> (%d KB, %d KB)!\n", Min / 1024, Max / 1024, (Min + Size) / 1024, (Max + Size) / 1024);
	return TRUE;
}

static BOOL TryLockPage(PVOID Page) {
	if (!TryIncreaseWorkingSetSize(0x1000))
		return FALSE;
	if (VirtualLock(Page, 0x1000))
		return TRUE;
	if (!TryIncreaseWorkingSetSize(0x2000))
		return FALSE;
	return VirtualLock(Page, 0x1000);
}

static BOOL LockRange(PVOID From, PVOID To) {
	PBYTE FromPageAligned = (PBYTE) ((uintptr_t) (From) & (~0xFFF));
	PBYTE ToPageAligned = (PBYTE) ((uintptr_t) (To) & (~0xFFF));

	for (PBYTE Current = FromPageAligned; Current <= ToPageAligned; Current += 0x1000) {
		if (!TryLockPage(Current)) {
			// printf("[+] Failed locking %16llx!\n", Current);
			return FALSE;
		}
		else {
			// printf("[+] Locked %16llx successfully!\n", From);
		}
	}
	return TRUE;
}

static BOOL LockMemorySections() {
	// printf("[+] .LDATA: %16llx -> %16llx!\n", &nonPagedDataStart, &nonPagedDataEnd);
	// printf("[+] .LTEXT: %16llx -> %16llx!\n", &nonPagedTextStart, &nonPagedTextEnd);

	return LockRange(&nonPagedDataStart, &nonPagedDataEnd) && LockRange(&nonPagedTextStart, &nonPagedTextEnd);
}