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

static bool IsFunctionHooked( LPCWSTR dllName, LPCWSTR functionName )
{
	return GetPrivateProfileIntW( dllName, functionName, 0, L".\\UptimeFaker.ini" ) != 0;
}

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
			if ( IsFunctionHooked(L"kernel32", L"QueryPerformanceCounter") ) OrgQueryPerformanceCounter = (decltype(OrgQueryPerformanceCounter))GetProcAddress( hModule, "QueryPerformanceCounter" );
			if ( IsFunctionHooked(L"kernel32", L"GetTickCount") ) OrgGetTickCount = (decltype(OrgGetTickCount))GetProcAddress( hModule, "GetTickCount" );
			if ( IsFunctionHooked(L"kernel32", L"GetTickCount64") ) OrgGetTickCount64 = (decltype(OrgGetTickCount64))GetProcAddress( hModule, "GetTickCount64" );

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
	AddedTimeInDays = static_cast<signed int>(GetPrivateProfileIntW( L"AddedUptime", L"AddUptimeDays", 0, L".\\UptimeFaker.ini" ));

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

