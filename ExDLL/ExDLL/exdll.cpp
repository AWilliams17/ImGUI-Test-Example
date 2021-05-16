#include "exdll.h"
#include <conio.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include <WinUser.h>
#include "includes/imgui-1.81/imgui.h"
#include "includes/imgui-1.81/imgui_impl_dx9.h"
#include "includes/imgui-1.81/imgui_impl_win32.h"
#include "includes/imgui-1.81/imgui_internal.h"

typedef HRESULT(APIENTRY* EndScene) (IDirect3DDevice9 *pDevice);
EndScene EndSceneOriginal;

bool bImGuiInitialized = false;
HRESULT APIENTRY EndSceneHook(IDirect3DDevice9 *pDevice) {
	if (!bImGuiInitialized) {
		InitImGui(pDevice);
		bImGuiInitialized = true;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowAboutWindow(&bImGuiInitialized);

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return EndSceneOriginal(pDevice);
}

DWORD GetPatternAddress(const char* PATTERN, const char* PATTERN_MASK) {
	DWORD patternAddress = NULL;

	HMODULE hDXD9 = GetModuleHandle(L"shaderapidx9.dll");
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
					patternAddress = baseAddress + i; // Pattern matched.
				}
			}
		}
		else break;
	}

	return patternAddress;
}

void InitImGui(IDirect3DDevice9 *d3Device) {
	ImGui::CreateContext();
	ImGuiIO& imGuiIO = ImGui::GetIO();
	imGuiIO.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NavEnableKeyboard;
	imGuiIO.Fonts->AddFontDefault();
	D3DDEVICE_CREATION_PARAMETERS d3CreationParams;
	d3Device->GetCreationParameters(&d3CreationParams);

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(d3CreationParams.hFocusWindow);
	ImGui_ImplDX9_Init(d3Device);
}

void DoHook() {
	const char* pattern = "\xA1\x00\x00\x00\x00\x50\x8B\x08\xFF\x51\x0C";
	const char* mask = "x????xxxxxx";

	DWORD dwD9Device = **(DWORD**)(GetPatternAddress(pattern, mask) + 0x01);
	DWORD **pVTable = *(DWORD***)(dwD9Device);

	if (!dwD9Device) {
		printf("Pattern scan for Directx 9 Device failed.\n");
	} else {
		printf("VTable at: %x\n", *pVTable);
		printf("(pre-hook)EndScene at: %x\n", pVTable[42]);

		DWORD oldProtection;
		if (!VirtualProtect(pVTable[42], 4, PAGE_EXECUTE_READWRITE, &oldProtection)) {
			printf("First VirtualProtect call failed\n");
		}
		else {
			EndSceneOriginal = (EndScene)pVTable[42];
			pVTable[42] = (DWORD*)&EndSceneHook;

			if (!VirtualProtect(pVTable[42], 4, oldProtection, NULL)) {
				printf("Second VirtualProtect call failed\n");
			}

			printf("(post-hook)EndScene at: %x\n", pVTable[42]);
		}
	}
}
