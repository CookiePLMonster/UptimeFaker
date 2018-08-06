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
		hModule = LoadLibraryW( L"kernel32" );
		if ( hModule != nullptr )
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

namespace Winmm
{
	HMODULE hModule;

	decltype(::timeGetTime)* OrgtimeGetTime;
	DWORD WINAPI timeGetTime()
	{
		return static_cast<DWORD>(OrgtimeGetTime() + AddedTimeInMS);
	}

	decltype(::timeGetSystemTime)* OrgtimeGetSystemTime;
	MMRESULT WINAPI timeGetSystemTime(LPMMTIME pmmt,UINT cbmmt)
	{
		MMRESULT result = OrgtimeGetSystemTime(pmmt, cbmmt);
		if ( result == TIMERR_NOERROR )
		{
			if ( cbmmt == sizeof(MMTIME) && pmmt->wType == TIME_MS )
			{
				pmmt->u.ms += static_cast<DWORD>(AddedTimeInMS);
			}
		}
		return result;
	}

	void AttachModule()
	{
		hModule = LoadLibraryW( L"winmm" );
		if ( hModule != nullptr )
		{
			if ( IsFunctionHooked(L"winmm", L"timeGetTime") ) OrgtimeGetTime = (decltype(OrgtimeGetTime))GetProcAddress( hModule, "timeGetTime" );
			if ( IsFunctionHooked(L"winmm", L"timeGetSystemTime") ) OrgtimeGetSystemTime = (decltype(OrgtimeGetSystemTime))GetProcAddress( hModule, "timeGetSystemTime" );

			DetourAttach( &(PVOID&)OrgtimeGetTime, timeGetTime );
			DetourAttach( &(PVOID&)OrgtimeGetSystemTime, timeGetSystemTime );
		}
	}

	void DetachModule()
	{
		DetourDetach( &(PVOID&)OrgtimeGetTime, timeGetTime );
		DetourDetach( &(PVOID&)OrgtimeGetSystemTime, timeGetSystemTime );
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
	Winmm::AttachModule();

	DetourTransactionCommit();

	return TRUE;
}

BOOL DetachFunctions()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	Winmm::DetachModule();
	Kernel32::DetachModule();

	DetourTransactionCommit();

	Kernel32::Free();
	Winmm::Free();

	return TRUE;
}

