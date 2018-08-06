// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

#include "Detours/detours.h"

BOOL AttachFunctions();
BOOL DetachFunctions();

void GetFakeTimeValues();

BOOL APIENTRY DllMain( HMODULE /*hModule*/,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
                     )
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		DetourRestoreAfterWith();

		GetFakeTimeValues();
		return AttachFunctions();
    case DLL_PROCESS_DETACH:
        return DetachFunctions();
    }
    return TRUE;
}

