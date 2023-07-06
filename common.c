#include "common.h"
#include "util.h"


DWORD GetPID(LPCWSTR procname)
{

	DWORD dwLsassPID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		wprintf(L"[-] CreateToolhelp32Snapshot failed, error code: %lu\n", GetLastError());
		return -1;
	}
	//PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
	PROCESSENTRY32W pe;
	ZeroMemory(&pe, sizeof(PROCESSENTRY32W));
	pe.dwSize = sizeof(PROCESSENTRY32W);
	
	BOOL bFind = Process32FirstW(hSnap, &pe);
	while (bFind)
	{
		if (my_wcsicmp(pe.szExeFile, procname) == 0)
		{
			dwLsassPID = pe.th32ProcessID;
			break;
		}
		bFind = Process32NextW(hSnap, &pe);
	}
	CloseHandle(hSnap);

	if (dwLsassPID == 0)
	{
		wprintf(L"[-] L$a$s process not found.\n");
		return -1;
	}

	return dwLsassPID;

}


VOID EnablePrivilege(LPCWSTR flag)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES TokenPrivileges;
	BOOL bResult;

	bResult = OpenProcessToken(NtCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
	if (!bResult)
	{
		wprintf(L"[-] OpenProcessToken failed, error code: %lu\n", GetLastError());
		return;
	}
	wprintf(L"[+] Open CurrentProcess Token Successful.\n");

	LUID luid;
	bResult = LookupPrivilegeValueW(NULL, flag, &luid);
	if (!bResult)
	{
		wprintf(L"[-] LookupPrivilegeValue failed, error code: %lu\n", GetLastError());
		return;
	}
	wprintf(L"[+] LookupPrivilegeValue Successful.\n");

	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Luid = luid;
	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	bResult = AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (!bResult)
	{
		wprintf(L"[-] AdjustTokenPrivileges failed, error code: %lu\n", GetLastError());
		return;
	}

	wprintf(L"[+] Enable %ls Privileges Successful.\n", flag);
}