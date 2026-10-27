#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#define SUBPROCESS_OTHER 1
#define SUBPROCESS_MALLOC 2
#define SUBPROCESS_EOF 3

#include "clnup.h"
#ifdef _WIN32
	#include "windows.h"
#else
	#include "unix.h"
#endif

int subprocess_init(void);
int subprocess_open(subprocess_proc_t * output, subprocess_char_t ** cmd, subprocess_pipe_t * proc_stdout, subprocess_pipe_t * proc_stderr, subprocess_pipe_t * proc_stdin);
int subprocess_wait(subprocess_return_t * rtn, subprocess_proc_t * proc, subprocess_time_t max_wait);
int subprocess_terminate(subprocess_proc_t * proc);
int subprocess_kill(subprocess_proc_t * proc);
int subprocess_suspend(subprocess_proc_t * proc);
int subprocess_resume(subprocess_proc_t * proc);
int subprocess_pipe_create(subprocess_pipe_t * p);
int subprocess_pipe_free(subprocess_pipe_t * p);
int subprocess_pipe_read(subprocess_pipe_t * p, subprocess_pipesize_t size, subprocess_pipesize_t * size_out, char * output);
int subprocess_pipe_write(subprocess_pipe_t * p, subprocess_pipesize_t size, char * input);

#endif
