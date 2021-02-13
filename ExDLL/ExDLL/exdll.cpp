#include "exdll.h"
#include <conio.h>
#include <string>
#include <Psapi.h>
#include <WinUser.h>

typedef HRESULT(APIENTRY* EndScene) (LPDIRECT3DDEVICE9 pDevice);
EndScene EndSceneOriginal = nullptr;

HRESULT __stdcall EndSceneHook(IDirect3DDevice9* pDevice) {
	ImGui::ShowDemoWindow();
	return EndSceneOriginal(pDevice);
}

DWORD GetPatternAddress(const char* PATTERN, const char* PATTERN_MASK) {
	DWORD patternAddress = NULL;

	HMODULE hDXD9 = GetModuleHandle(L"d3d9.dll");
	MODULEINFO modInfo;
	if (!GetModuleInformation(GetCurrentProcess(), hDXD9, &modInfo, sizeof(MODULEINFO))) {
		return NULL;
	}

	DWORD baseAddress = (DWORD)modInfo.lpBaseOfDll;
	
	BYTE* pPattern = (BYTE*)PATTERN;
	int pattern_size = std::strlen(PATTERN_MASK);

	for (int i = 0; i < modInfo.SizeOfImage - pattern_size; i++) {
		if (!patternAddress) {
			for (int x = 0; x < pattern_size; x++) {
				if (PATTERN_MASK[x] == '?') {
					continue; // Wildcard; Ignore.
				}

				if (*(BYTE*)(baseAddress + i + x) != pPattern[x]) {
					break; // Pattern discrepancy; Break.
				}

				if (*(BYTE*)(baseAddress + i + x) == pPattern[x] && x == pattern_size - 1) {
					patternAddress = (DWORD)modInfo.lpBaseOfDll + i; // Pattern matched.
				}
			}
		}
		else break;
	}

	return patternAddress;
}

void PrintVTableAddress() {
	char *pVTable[119];

	const char* pattern = "\xC7\x06\x00\x00\x00\x00\x89\x86\x00\x00\x00\x00\x89\x86";
	const char* mask = "xx????xx????xx";

	DWORD dwD9Device = GetPatternAddress(pattern, mask);
	memcpy(&pVTable, *(void**)(dwD9Device+2), sizeof(pVTable));

	printf("VTable at: %x\n", dwD9Device);
	printf("EndScene at: %x\n", pVTable[42]);
}
