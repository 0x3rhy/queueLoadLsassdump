#include <windows.h>
#include <tlhelp32.h>

#define NtCurrentProcess() ((HANDLE) (LONG_PTR)-1)



DWORD GetPID(LPCWSTR procname);
VOID EnablePrivilege(LPCWSTR flag);
