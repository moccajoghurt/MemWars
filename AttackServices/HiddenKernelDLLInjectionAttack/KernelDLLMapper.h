#pragma once
#include <Windows.h>
#include <fstream>
#include <vector>
#include <functional>
#include <string>

using namespace std;

struct TlsLockedHookStatus {
	BYTE isFree;
	BYTE numThreadsWaiting;
	BYTE entryBytes;
};

static vector<BYTE> ReadFile(const string& path) {
	ifstream stream(path, ios::binary | ios::ate);
	ifstream::pos_type pos = stream.tellg();

	if (pos == (ifstream::pos_type) - 1) {
        return {};
    }

	vector<BYTE> data(pos);
	stream.seekg(0, ios::beg);
	stream.read((char*) &data[0], pos);

	return data;
}

static void* RvaToPointer(BYTE* image, DWORD va) {
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)image;
	PIMAGE_NT_HEADERS fileHeader = (PIMAGE_NT_HEADERS)((uint64_t)dosHeader + dosHeader->e_lfanew);

	PIMAGE_SECTION_HEADER sectionHeader = (PIMAGE_SECTION_HEADER)(((ULONG_PTR) &fileHeader->OptionalHeader) + fileHeader->FileHeader.SizeOfOptionalHeader);

	for (int i = 0; i < fileHeader->FileHeader.NumberOfSections; i++) {
		char* name = (char*) sectionHeader[i].Name;
		DWORD rawData = sectionHeader[i].PointerToRawData;
		DWORD virtualAddress = sectionHeader[i].VirtualAddress;
		DWORD rawSize = sectionHeader[i].SizeOfRawData;
		DWORD virtualSize = sectionHeader[i].Misc.VirtualSize;

		if (va >= virtualAddress && va < (virtualAddress + virtualSize)) {
			return image + va - virtualAddress + rawData;
		}
	}
	return image + va;
}

static void PushBytes(vector<BYTE>& target, const vector<BYTE>& bytes) {
	int i = target.size();
	target.resize(i + bytes.size());
	memcpy(&target[i], &bytes[0], bytes.size());
}

static vector<BYTE> CreateImportShell(BYTE* image, PVOID mappedAdr, bool loadLib) {

	vector<BYTE> out = { 
		0x48, 0x83, 0xEC, 0x38,                                       // sub    rsp,0x38
		0x4C, 0x8D, 0x3D, 0xDD, 0xCC, 0xBB, 0x00,                     // lea r15, [rip+0xBBCCDD]
		0x48, 0xB8, 0xAA, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x00, 0x00,   // mov rax, 0xAABBCCDDEEAA ; GetModuleHandleA // LoadLibraryA?
		0x49, 0x89, 0xC5,                                             // mov r13, rax
		0x48, 0xB8, 0xAA, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x00, 0x00,   // mov rax, 0xAABBCCDDEEAA ; GetProcAddress
		0x49, 0x89, 0xC4                                              // mov r11, rax
	};

	*(FARPROC*)&out[0xD] = loadLib ? GetProcAddress(GetModuleHandleA("KERNEL32"), "LoadLibraryA") : GetProcAddress(GetModuleHandleA("KERNEL32"), "GetModuleHandleA"); // avoding __imp's
	*(FARPROC*)&out[0x1A] = GetProcAddress(GetModuleHandleA("KERNEL32"), "GetProcAddress");   // avoding __imp's

	vector<BYTE> dataContainer = {};

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)image;
	PIMAGE_NT_HEADERS fileHeader = (PIMAGE_NT_HEADERS)((uint64_t)dosHeader + dosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER optionalHeader = &fileHeader->OptionalHeader;

    

	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)RvaToPointer(
		image,
		fileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
	);

    

	while (importDescriptor && importDescriptor->Name && fileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
		PCHAR moduleName = (PCHAR)RvaToPointer(image, importDescriptor->Name);

		IMAGE_THUNK_DATA* thunk = NULL;
		IMAGE_THUNK_DATA* func = NULL;

		uint32_t moduleNameOffset = dataContainer.size();

		do {
            dataContainer.push_back(*moduleName);
        }
		while (*moduleName++);

		vector<BYTE> modulePusher = { 
			0x49, 0x8D, 0x8F, 0xBB, 0xAA, 0x00, 0x00,  // lea    rcx,[r15+0xaabb]
			0x41, 0xFF, 0xD5,                          // call   r13
			0x48, 0x89, 0xC6                           // mov    rsi,rax
		};

		*(uint32_t*) (&modulePusher[3]) = moduleNameOffset;

		PushBytes(out, modulePusher);

		if (importDescriptor->OriginalFirstThunk) {
			thunk = (IMAGE_THUNK_DATA*)RvaToPointer(image, importDescriptor->OriginalFirstThunk);
			func = (IMAGE_THUNK_DATA*)((PUCHAR) mappedAdr + importDescriptor->FirstThunk);
		}
		else {
			thunk = (IMAGE_THUNK_DATA*)RvaToPointer(image, importDescriptor->FirstThunk);
			func = (IMAGE_THUNK_DATA*)((PUCHAR) mappedAdr + importDescriptor->FirstThunk);
		}

		for (; thunk->u1.AddressOfData; thunk++, func++) {
			// assert(!(thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64));
            if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64) {
                vector<BYTE> failed;
                return failed;
            }

			FARPROC functionAddress = NULL;
			IMAGE_IMPORT_BY_NAME* imageImportByName = (IMAGE_IMPORT_BY_NAME*)RvaToPointer(image, *(DWORD*) thunk);
			PCHAR importName = (PCHAR)imageImportByName->Name;
			ULONGLONG* target = &func->u1.Function;

			uint32_t importNameOffset = dataContainer.size();

			if (!strcmpi(importName, "AddVectoredExceptionHandler")){
                cout << "WARNING: Vectored Exception Handling IS NOT SUPPORTED!" << endl;
            }

			do {
                dataContainer.push_back(*importName);
            }
			while (*importName++);

			uint32_t offsetOffset = dataContainer.size();
			dataContainer.resize(dataContainer.size() + 8);
			*(uint64_t*)(&dataContainer[offsetOffset]) = (uint64_t)target;

			vector<BYTE> importFixer = { 
				0x48, 0x89, 0xF1,                          // mov    rcx,rsi
				0x49, 0x8D, 0x97, 0xBB, 0xAA, 0x00, 0x00,  // lea    rdx,[r9+0xaabb]
				0x41, 0xFF, 0xD4,                          // call   r12
				0x49, 0x8B, 0x9F, 0xBB, 0xAA, 0x00, 0x00,  // mov    rbx,QWORD PTR [r9+0xaabb]
				0x48, 0x89, 0x03                           // mov    QWORD PTR [rbx],rax
			};

			*(uint32_t*)(&importFixer[6]) = importNameOffset;
			*(uint32_t*)(&importFixer[16]) = offsetOffset;

			PushBytes(out, importFixer);
		}
		importDescriptor++;
	}

	PushBytes(out, { 0x48, 0x83, 0xC4, 0x38 }); // add rsp, 0x38
	uint32_t jmpSize = out.size();
	PushBytes(out, { 0xE9, 0x00, 0x00, 0x00, 0x00 }); // jmp 0xAABBCCDD
	*(uint32_t*) (&out[7]) = out.size() - 0xB;
	PushBytes(out, dataContainer);
	*(int32_t*) (&out[jmpSize + 1]) = dataContainer.size();
	return out;
}

static void RelocateImage(BYTE* image, BYTE* target) {
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)image;
	PIMAGE_NT_HEADERS fileHeader = (PIMAGE_NT_HEADERS) ((uint64_t)dosHeader + dosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER sectionHeader = (PIMAGE_SECTION_HEADER)(((ULONG_PTR) &fileHeader->OptionalHeader) + fileHeader->FileHeader.SizeOfOptionalHeader);

	// Copy sections
	memcpy(target, image, 0x1000); // Pe Header

	for (int i = 0; i < fileHeader->FileHeader.NumberOfSections; i++) {
		char* name = (char*)sectionHeader[i].Name;
		uint64_t rawData = sectionHeader[i].PointerToRawData;
		uint64_t virtualAddress = sectionHeader[i].VirtualAddress;
		uint64_t rawSize = sectionHeader[i].SizeOfRawData;
		uint64_t virtSize = sectionHeader[i].Misc.VirtualSize;
		ZeroMemory(target + virtualAddress, virtSize);
		memcpy(target + virtualAddress, image + rawData, rawSize);

		if (!strcmpi(name, ".pdata")) {
            // cout << "WARNING: Structured Exception Handling IS NOT SUPPORTED!" << endl;
        }
		if (!strcmpi(name, ".tls")) {
            // cout << "WARNING: Thread-local Storage IS NOT SUPPORTED!" << endl;
        }
	}

	// Reloc sections
	if (fileHeader->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_BASERELOC &&
		 fileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress != 0) {

		PIMAGE_BASE_RELOCATION reloc = (PIMAGE_BASE_RELOCATION) (target + fileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		DWORD relocSize = fileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
		uint64_t delta = (uint64_t)target - fileHeader->OptionalHeader.ImageBase;
		int c = 0;
		while (c < relocSize) {
			size_t p = sizeof(IMAGE_BASE_RELOCATION);
			LPWORD chains = (LPWORD) ((PUCHAR) reloc + p);
			while (p < reloc->SizeOfBlock) {
				uint64_t base = (uint64_t) (target + reloc->VirtualAddress);
				switch (*chains >> 12) {
					case IMAGE_REL_BASED_HIGHLOW:
						*(uint32_t*) (base + (*chains & 0xFFF)) += (uint32_t)delta;
						break;
	 				case IMAGE_REL_BASED_DIR64:
						*(uint64_t*) (base + (*chains & 0xFFF)) += delta;
						break;
				}
				chains++;
				p += sizeof(WORD);
			}
			c += reloc->SizeOfBlock;
			reloc = (PIMAGE_BASE_RELOCATION) ((PBYTE) reloc + reloc->SizeOfBlock);
		}
	}
}


BOOL MapDllToKernel(const string& path, PVOID valCheck, PVOID hookOut, bool loadLib, const function<PVOID(SIZE_T)>& KernelMemoryAllocator) {

    auto file = ReadFile(path);
    if (file.size() == 0) {
        return FALSE;
    }

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)file.data();
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }

    PIMAGE_NT_HEADERS fileHeader = (PIMAGE_NT_HEADERS)((uint64_t)dosHeader + dosHeader->e_lfanew);
    if (fileHeader->Signature != IMAGE_NT_SIGNATURE || fileHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        return FALSE;
    }

	PIMAGE_OPTIONAL_HEADER optionalHeader = &fileHeader->OptionalHeader;
	if (optionalHeader->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return FALSE;
    }

    vector<BYTE> prologue = { 
		0x00, 0x00, // data
		0xF0, 0xFE, 0x05, 0xF8, 0xFF, 0xFF, 0xFF,                     // lock inc byte ptr [rip-n]
		                                                              // wait_lock:
		0x80, 0x3D, 0xF0, 0xFF, 0xFF, 0xFF, 0x00,                     // cmp byte ptr [rip-m], 0x0
		0xF3, 0x90,                                                   // pause
		0x74, 0xF5,                                                   // je wait_lock

		0x48, 0xB8, 0xAA, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x00, 0x00,   // mov rax, 0xAABBCCDDEEAA
		                                                              // data_sync_lock:
		0x0F, 0x0D, 0x08,                                             // prefetchw [rax]
		0x81, 0x38, 0xDD, 0xCC, 0xBB, 0xAA,                           // cmp dword ptr[rax], 0xAABBCCDD
		0xF3, 0x90,                                                   // pause
		0x75, 0xF3,                                                   // jne data_sync_lock

		0xF0, 0xFE, 0x0D, 0xCF, 0xFF, 0xFF, 0xFF,                     // lock dec byte ptr [rip-n]
		0x75, 0x41,                                                   // jnz continue_exec                         
		0x53,                                                         // push registers
		0x51, 
		0x52, 
		0x56, 
		0x57, 
		0x55, 
		0x41, 0x50, 
		0x41, 0x51, 
		0x41, 0x52, 
		0x41, 0x53, 
		0x41, 0x54, 
		0x41, 0x55, 
		0x41, 0x56, 
		0x41, 0x57, 
		0x9C, 
		0x48, 0x89, 0xE5,                                             // mov rbp, rsp
		0x48, 0x83, 0xEC, 0x20,                                       // sub rsp, 0x20
		0x48, 0x83, 0xE4, 0xF0,                                       // and rsp, 0xFFFFFFFFFFFFFFF0
		0xE8, 0x26, 0x00, 0x00, 0x00,                                 // call stub
		0x48, 0x89, 0xEC,                                             // mov rsp, rbp
		0x9D,                                                         // pop registers
		0x41, 0x5F, 
		0x41, 0x5E,
		0x41, 0x5D, 
		0x41, 0x5C, 
		0x41, 0x5B, 
		0x41, 0x5A, 
		0x41, 0x59, 
		0x41, 0x58, 
		0x5D, 
		0x5F, 
		0x5E, 
		0x5A, 
		0x59, 
		0x5B, 
		0x48, 0xB8, 0xAA, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x00,  // mov rax, 0xAABBCCDDEEFFAA
		0xFF, 0xE0                                                   // jmp rax
		                                                             // stub:
	};


	*(PVOID*) &prologue[0x77] = hookOut;
	*(PVOID*) &prologue[0x16] = valCheck;
	*(DWORD*) &prologue[0x23] = *(DWORD*)valCheck;


	vector<BYTE> jmpEntryPont = { 
		0x48, 0xB8, 0xDD, 0xCC, 0xBB, 0xAA, 0x00, 0x00, 0x00, 0x00,   // mov rax, 0xAABBCCDD
		0x48, 0x89, 0xC1,                                             // mov rcx, rax
		0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00,                     // mov rdx, 1
		0x4D, 0x31, 0xC0,                                             // xor r8, r8
		0x48, 0x05, 0xCD, 0xBB, 0xAA, 0x00,                           // add rax, 0xAABBCD
		0xFF, 0xE0                                                    // jmp rax
	};

	uint32_t shellSize = CreateImportShell(file.data(), nullptr, loadLib).size() + jmpEntryPont.size() + prologue.size();
    BYTE* memory = (BYTE*)KernelMemoryAllocator(optionalHeader->SizeOfImage + shellSize + 0xFFF);
	
	
    uint64_t imageMemory = ((uint64_t)memory + shellSize + 0xFFF)&(~0xFFF);

	*(uint64_t*)(&jmpEntryPont[0x02]) = imageMemory;
	*(uint32_t*)(&jmpEntryPont[0x19]) = fileHeader->OptionalHeader.AddressOfEntryPoint;

	auto shell = CreateImportShell(file.data(), PVOID(imageMemory), loadLib);
	PushBytes(shell, jmpEntryPont);
	PushBytes(prologue, shell);
	shell = prologue;
    RelocateImage(file.data(), PBYTE(imageMemory));
	memcpy(memory, shell.data(), shell.size());

	// cout << (PVOID)memory << endl;
	// cout << "relax" << endl;

    return TRUE;
}