// dllmain.cpp : Defines the entry point for the DLL application.
#include "exdll.h"
#include <conio.h>
#include <cstdio>

bool bHooked = false;
DWORD CALLBACK HookFunctions(LPVOID) {
    if (!bHooked) {
        DoHook();
        bHooked = true;
    }
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    AllocConsole();
    FILE *f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    SetConsoleTitle(L"DEBUG");

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HookFunctions, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

