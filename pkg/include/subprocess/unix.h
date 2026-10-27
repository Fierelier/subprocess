#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdlib.h>

typedef int subprocess_return_t;
typedef ssize_t subprocess_pipesize_t;
typedef pid_t subprocess_pid_t;
typedef useconds_t subprocess_time_t;
typedef char subprocess_char_t;
#ifndef TEXT
	#define TEXT(x) x
#endif
#define SUBPROCESS_CHECK_TIME 50 // Interval (in milliseconds) between process status checks

typedef struct subprocess_proc_t {
	subprocess_pid_t pid;
} subprocess_proc_t;

typedef struct subprocess_pipe_t {
	int pipe[2];
} subprocess_pipe_t;
