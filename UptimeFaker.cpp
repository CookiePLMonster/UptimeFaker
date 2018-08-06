// UptimeFaker.cpp : Defines the exported functions for the DLL application.
//

#include <Windows.h>
#include "Detours/detours.h"
#include <cstdint>

#ifdef _WIN64
#pragma comment(lib, "detours_x64.lib")
#else
#pragma comment(lib, "detours.lib")
#endif

int64_t AddedTimeInDays;
int64_t AddedTimeInMS;
int64_t AddedTimeInQPCTicks;

namespace Kernel32
{
	HMODULE hModule;

	decltype(::QueryPerformanceCounter)* OrgQueryPerformanceCounter;
	BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
	{
		BOOL result = OrgQueryPerformanceCounter( lpPerformanceCount );
		if ( result != 0 )
		{
			lpPerformanceCount->QuadPart += AddedTimeInQPCTicks;
		}
		return result;
	}

	decltype(::GetTickCount)* OrgGetTickCount;
	DWORD WINAPI GetTickCount()
	{
		return static_cast<DWORD>(OrgGetTickCount() + AddedTimeInMS);
	}

	decltype(::GetTickCount64)* OrgGetTickCount64;
	ULONGLONG WINAPI GetTickCount64()
	{
		return OrgGetTickCount64() + AddedTimeInMS;
	}

	void AttachModule()
	{
		if ( GetModuleHandleExW( 0, L"kernel32", &hModule ) )
		{
			OrgQueryPerformanceCounter = (decltype(OrgQueryPerformanceCounter))GetProcAddress( hModule, "QueryPerformanceCounter" );
			OrgGetTickCount = (decltype(OrgGetTickCount))GetProcAddress( hModule, "GetTickCount" );
			OrgGetTickCount64 = (decltype(OrgGetTickCount64))GetProcAddress( hModule, "GetTickCount64" );

			DetourAttach( &(PVOID&)OrgQueryPerformanceCounter, QueryPerformanceCounter );
			DetourAttach( &(PVOID&)OrgGetTickCount, GetTickCount );
			DetourAttach( &(PVOID&)OrgGetTickCount64, GetTickCount64 );
		}
	}

	void DetachModule()
	{
		DetourDetach( &(PVOID&)OrgQueryPerformanceCounter, QueryPerformanceCounter );
		DetourDetach( &(PVOID&)OrgGetTickCount, GetTickCount );
		DetourDetach( &(PVOID&)OrgGetTickCount64, GetTickCount64 );
	}

	void Free()
	{
		if ( hModule != nullptr )
		{
			FreeLibrary(hModule);
		}
	}
}

void GetFakeTimeValues()
{
	// TODO: Customize
	AddedTimeInDays = 365;

	// Calculate helper values
	AddedTimeInMS = AddedTimeInDays * 24 * 60 * 60 * 1000;

	LARGE_INTEGER QPCFreq;
	::QueryPerformanceFrequency( &QPCFreq );
	AddedTimeInQPCTicks = AddedTimeInDays * 24 * 60 * 60 * QPCFreq.QuadPart; 
}


BOOL AttachFunctions()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	Kernel32::AttachModule();

	DetourTransactionCommit();

	return TRUE;
}

BOOL DetachFunctions()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	Kernel32::DetachModule();

	DetourTransactionCommit();

	Kernel32::Free();

	return TRUE;
}

