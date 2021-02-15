#pragma once
#include <d3d9.h>


void DoHook();
void InitImGui(IDirect3DDevice9* pDevice);
HRESULT APIENTRY EndSceneHook(IDirect3DDevice9* pDevice);