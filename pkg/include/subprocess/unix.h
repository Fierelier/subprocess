#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>

typedef int subprocess_return_t;
typedef ssize_t subprocess_pipesize_t;
typedef pid_t subprocess_pid_t;
typedef useconds_t subprocess_time_t;
#define SUBPROCESS_CHECK_TIME 50 // Interval (in milliseconds) between process status checks

typedef struct subprocess_proc_t {
	int i_stdout;
	int i_stderr;
	int i_stdin;
	pid_t pid;
} subprocess_proc_t;
