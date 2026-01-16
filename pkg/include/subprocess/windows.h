#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

SECURITY_ATTRIBUTES subprocess_windows_sa;

typedef DWORD subprocess_return_t;
typedef DWORD subprocess_pipesize_t;
typedef DWORD subprocess_pid_t;
typedef DWORD subprocess_time_t;

typedef struct subprocess_proc_t {
	HANDLE hProcess;
	HANDLE hThread;
	subprocess_pid_t pid;
	BOOL suspended;
} subprocess_proc_t;

char * subprocess_windows_cmdstr(char ** argv);
int subprocess_windows_suspend(subprocess_proc_t * proc, BOOL suspend);
