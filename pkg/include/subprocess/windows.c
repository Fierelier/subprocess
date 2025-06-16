int subprocess_init(void) {
	subprocess_windows_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	subprocess_windows_sa.bInheritHandle = TRUE;
	subprocess_windows_sa.lpSecurityDescriptor = NULL;
	return 0;
}

char * subprocess_windows_cmdstr(char ** argv) {
	// Figure out how long the command string has to be
	size_t commandLength = 0;
	size_t i;
	for (i = 0; argv[i] != NULL; i++) {
		// Each argument will have a following space
		commandLength += strlen(argv[i]) + 1;
		
		// Argument with spaces will need quotes
		// Not just quoting every single argument may seem overkill, but it's actually required, for some programs do not use argv
		int quote = (strchr(argv[i],' ') != NULL);
		if (quote) {
			commandLength += 2;
			size_t si;
			for (si = 0; argv[i][si] != 0; si++) {
				if (argv[i][si] == '"') {
					// Each " will have to be escaped by a \ - add 1
					commandLength++;
				}
			}
		}
	}
	// There will be a trailing space due to the simplistic code constructing the string
	commandLength += 1;
	
	// Allocate memory
	char * command = malloc(commandLength);
	if (command == NULL) { return NULL; }
	
	command[0] = 0;
	// If the command is empty, return an empty string
	if (commandLength < 1) { return command; }
	
	// Add spaces and quotes where necessary
	for (i = 0; argv[i] != NULL; i++) {
		int quote = (strchr(argv[i],' ') != NULL);
		if (quote) { strcat(command,"\""); }
		size_t si;
		for (si = 0; argv[i][si] != 0; si++) {
			if (quote && (argv[i][si] == '"')) {
				strcat(command,"\\");
			}
			strncat(command,argv[i] + si,1);
		}
			
		if (quote) { strcat(command,"\""); }
		strcat(command," ");
	}
	
	// Get "rid" of the trailing space
	command[commandLength - 2] = 0;
	return command;
}

#define subprocess_clean_pipe(pname) \
	if (hook_##pname) { \
		CloseHandle(pipe_##pname##_rd); \
		CloseHandle(pipe_##pname##_wr); \
	}

/*
void PrintErrorMessage(DWORD errorCode) {
    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,                            // lpSource
        errorCode,                       // dwMessageId
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // dwLanguageId
        (LPTSTR)&lpMsgBuf,               // lpBuffer (receives allocated buffer)
        0,                               // nSize
        NULL);                           // Arguments

    // Display the message
    printf("Error %lu: %s", errorCode, (char*)lpMsgBuf);

    // Free the buffer allocated by FormatMessage
    LocalFree(lpMsgBuf);
}
*/

int subprocess_open(subprocess_proc_t * output, char ** cmd, int hook_stdout, int hook_stderr, int hook_stdin) {
	char * wcmd = subprocess_windows_cmdstr(cmd);
	if (wcmd == NULL) {
		return SUBPROCESS_MALLOC;
	}
	
	HANDLE pipe_stdout_rd = NULL;
	HANDLE pipe_stdout_wr = NULL;
	HANDLE pipe_stderr_rd = NULL;
	HANDLE pipe_stderr_wr = NULL;
	HANDLE pipe_stdin_rd = NULL;
	HANDLE pipe_stdin_wr = NULL;
	
	if (hook_stdout) {
		if (!CreatePipe(&pipe_stdout_rd, &pipe_stdout_wr, &subprocess_windows_sa, 0)) {
			free(wcmd);
			return SUBPROCESS_MALLOC;
		}
		
		if (!SetHandleInformation(pipe_stdout_rd, HANDLE_FLAG_INHERIT, 0)) {
			free(wcmd);
			CloseHandle(pipe_stdout_rd);
			CloseHandle(pipe_stdout_wr);
			return SUBPROCESS_OTHER;
		}
	}
	
	if (hook_stderr) {
		if (!CreatePipe(&pipe_stderr_rd, &pipe_stderr_wr, &subprocess_windows_sa, 0)) {
			free(wcmd);
			subprocess_clean_pipe(stdout);
			return SUBPROCESS_MALLOC;
		}
		
		if (!SetHandleInformation(pipe_stderr_rd, HANDLE_FLAG_INHERIT, 0)) {
			free(wcmd);
			subprocess_clean_pipe(stdout);
			CloseHandle(pipe_stderr_rd);
			CloseHandle(pipe_stderr_wr);
			return SUBPROCESS_OTHER;
		}
	}
	
	if (hook_stdin) {
		if (!CreatePipe(&pipe_stdin_rd, &pipe_stdin_wr, &subprocess_windows_sa, 0)) {
			free(wcmd);
			subprocess_clean_pipe(stderr);
			subprocess_clean_pipe(stdout);
			return SUBPROCESS_MALLOC;
		}
		
		if (!SetHandleInformation(pipe_stdin_wr, HANDLE_FLAG_INHERIT, 0)) {
			free(wcmd);
			subprocess_clean_pipe(stderr);
			subprocess_clean_pipe(stdout);
			CloseHandle(pipe_stdin_rd);
			CloseHandle(pipe_stdin_wr);
			return SUBPROCESS_OTHER;
		}
	}
	
	PROCESS_INFORMATION proc_info;
	ZeroMemory(&proc_info, sizeof(PROCESS_INFORMATION));
	
	STARTUPINFO proc_start;
	ZeroMemory(&proc_start, sizeof(STARTUPINFO));
	proc_start.cb = sizeof(STARTUPINFO);
	
	proc_start.dwFlags |= STARTF_USESTDHANDLES;
	
	if (hook_stdout) { proc_start.hStdOutput = pipe_stdout_wr; }
		else { proc_start.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE); }
	if (hook_stderr) { proc_start.hStdError = pipe_stderr_wr; }
		else { proc_start.hStdError = GetStdHandle(STD_ERROR_HANDLE); }
	if (hook_stdin) { proc_start.hStdInput = pipe_stdin_rd; }
		else { proc_start.hStdInput = GetStdHandle(STD_INPUT_HANDLE); }
	
	BOOL success = CreateProcessA(
		NULL,                     // Application name
		(LPSTR)wcmd,              // Command line
		NULL,                     // Process security attributes
		NULL,                     // Primary thread security attributes
		TRUE,                     // Handles are inherited
		CREATE_NEW_PROCESS_GROUP, // Creation flags
		NULL,                     // Use parent's environment
		NULL,                     // Use parent's current directory
		&proc_start,              // STARTUPINFO
		&proc_info                // PROCESS_INFORMATION
	);
	
	free(wcmd);
	if (hook_stdout) { CloseHandle(pipe_stdout_wr); }
	if (hook_stderr) { CloseHandle(pipe_stderr_wr); }
	if (hook_stdin) { CloseHandle(pipe_stdin_rd); }
	
	if (!success) {
		if (hook_stdout) { CloseHandle(pipe_stdout_rd); }
		if (hook_stderr) { CloseHandle(pipe_stderr_rd); }
		if (hook_stdin) { CloseHandle(pipe_stdin_wr); }
		return SUBPROCESS_OTHER;
	}
	
	output->hProcess = proc_info.hProcess;
	output->hThread = proc_info.hThread;
	output->pid = proc_info.dwProcessId;
	output->stdout_handle = pipe_stdout_rd;
	output->stderr_handle = pipe_stderr_rd;
	output->stdin_handle = pipe_stdin_wr;
	output->suspended = FALSE;
	return 0;
}

int subprocess_wait(subprocess_return_t * rtn, subprocess_proc_t * proc, subprocess_time_t max_wait) {
	if (max_wait == 0) { max_wait = INFINITE; }
	
	DWORD waitrtn = WaitForSingleObject(proc->hProcess, max_wait);
	if (waitrtn == WAIT_TIMEOUT) {
		return SUBPROCESS_EOF;
	} else if (waitrtn) {
		return SUBPROCESS_OTHER;
	}
	
	if (!GetExitCodeProcess(proc->hProcess, rtn)) {
		return SUBPROCESS_OTHER;
	}
	
	CloseHandle(proc->hProcess);
	CloseHandle(proc->hThread);
	if (proc->stdout_handle != NULL) { CloseHandle(proc->stdout_handle); }
	if (proc->stderr_handle != NULL) { CloseHandle(proc->stderr_handle); }
	if (proc->stdin_handle != NULL) { CloseHandle(proc->stdin_handle); }
	return 0;
}

int subprocess_read_stdout(char * output, subprocess_pipesize_t size, subprocess_pipesize_t * size_out, subprocess_proc_t * proc) {
	if (!ReadFile(proc->stdout_handle,output,size,size_out,NULL)) {
		if (GetLastError() == ERROR_NO_DATA) { return SUBPROCESS_EOF; }
		if (GetLastError() == ERROR_BROKEN_PIPE) { return SUBPROCESS_EOF; }
		return SUBPROCESS_OTHER;
	}
	if (*size_out == 0) { return SUBPROCESS_EOF; }
	return 0;
}

int subprocess_read_stderr(char * output, subprocess_pipesize_t size, subprocess_pipesize_t * size_out, subprocess_proc_t * proc) {
	if (!ReadFile(proc->stderr_handle,output,size,size_out,NULL)) {
		if (GetLastError() == ERROR_NO_DATA) { return SUBPROCESS_EOF; }
		if (GetLastError() == ERROR_BROKEN_PIPE) { return SUBPROCESS_EOF; }
		return SUBPROCESS_OTHER;
	}
	if (*size_out == 0) { return SUBPROCESS_EOF; }
	return 0;
}

int subprocess_write_stdin(char * input, subprocess_pipesize_t size, subprocess_proc_t * proc) {
	subprocess_pipesize_t size_out;
	if (!WriteFile(proc->stdin_handle,input,size,&size_out,NULL)) {
		if (GetLastError() == ERROR_NO_DATA) { return SUBPROCESS_EOF; }
		if (GetLastError() == ERROR_BROKEN_PIPE) { return SUBPROCESS_EOF; }
		return SUBPROCESS_OTHER;
	}
	
	if (size_out < size) {
		return SUBPROCESS_EOF;
	}
	
	return 0;
}

int subprocess_kill(subprocess_proc_t * proc) {
	if (!TerminateProcess(proc->hProcess,STATUS_CONTROL_C_EXIT)) {
		return SUBPROCESS_OTHER;
	}
	return 0;
}

int subprocess_terminate(subprocess_proc_t * proc) {
	/*if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, proc->pid)) {
		return SUBPROCESS_OTHER;
	}
	return 0;*/
	return subprocess_kill(proc);
}

int subprocess_windows_suspend(subprocess_proc_t * proc, BOOL suspend) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return SUBPROCESS_MALLOC;
	}
	
	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	if (!Thread32First(snapshot, &te32)) {
		CloseHandle(snapshot);
		return SUBPROCESS_OTHER;
	}
	
	do {
		if (te32.th32OwnerProcessID == proc->pid) {
			HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
			if (hThread != NULL) {
				if (suspend) {
					SuspendThread(hThread);
				} else {
					ResumeThread(hThread);
				}
				CloseHandle(hThread);
			}
		}
	} while (Thread32Next(snapshot, &te32));
	
	CloseHandle(snapshot);
	return 0;
}

int subprocess_suspend(subprocess_proc_t * proc) {
	BOOL suspend = TRUE;
	int rtn = 0;
	if (proc->suspended == suspend) { return 0; }
	if ((rtn = subprocess_windows_suspend(proc,suspend))) {
		return rtn;
	};
	proc->suspended = suspend;
	return 0;
}

int subprocess_resume(subprocess_proc_t * proc) {
	BOOL suspend = FALSE;
	int rtn = 0;
	if (proc->suspended == suspend) { return 0; }
	if ((rtn = subprocess_windows_suspend(proc,suspend))) {
		return rtn;
	};
	proc->suspended = suspend;
	return 0;
}
