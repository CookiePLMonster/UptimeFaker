// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

BOOL AttachFunctions();
BOOL DetachFunctions();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		return AttachFunctions();
    case DLL_PROCESS_DETACH:
        return DetachFunctions();
    }
    return TRUE;
}

