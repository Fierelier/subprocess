#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#define SUBPROCESS_OTHER 1
#define SUBPROCESS_MALLOC 2
#define SUBPROCESS_EOF 3

#ifdef _WIN32
	#include "windows.h"
#else
	#include "unix.h"
#endif

int subprocess_init(void);
int subprocess_open(subprocess_proc_t * output, char ** cmd, int hook_stdout, int hook_stderr, int hook_stdin);
int subprocess_wait(subprocess_return_t * rtn, subprocess_proc_t * proc, subprocess_time_t max_wait);
int subprocess_read_stdout(char * output, subprocess_pipesize_t size, subprocess_pipesize_t * size_out, subprocess_proc_t * proc);
int subprocess_read_stderr(char * output, subprocess_pipesize_t size, subprocess_pipesize_t * size_out, subprocess_proc_t * proc);
int subprocess_write_stdin(char * input, subprocess_pipesize_t size, subprocess_proc_t * proc);
int subprocess_terminate(subprocess_proc_t * proc);
int subprocess_kill(subprocess_proc_t * proc);
int subprocess_suspend(subprocess_proc_t * proc);
int subprocess_resume(subprocess_proc_t * proc);

#endif
