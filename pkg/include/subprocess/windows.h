#if defined(UNICODE) && !defined(_UNICODE)
	#define _UNICODE
#endif
#if defined(_UNICODE) && !defined(UNICODE)
	#define UNICODE
#endif
#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <stdio.h>

SECURITY_ATTRIBUTES subprocess_windows_sa;

typedef DWORD subprocess_return_t;
typedef DWORD subprocess_pipesize_t;
typedef DWORD subprocess_pid_t;
typedef DWORD subprocess_time_t;
typedef TCHAR subprocess_char_t;

typedef struct subprocess_proc_t {
	HANDLE hProcess;
	HANDLE hThread;
	subprocess_pid_t pid;
	BOOL suspended;
} subprocess_proc_t;

typedef struct subprocess_pipe_t {
	HANDLE read;
	HANDLE write;
} subprocess_pipe_t;

subprocess_char_t * subprocess_windows_cmdstr(subprocess_char_t ** argv);
int subprocess_windows_suspend(subprocess_proc_t * proc, BOOL suspend);
