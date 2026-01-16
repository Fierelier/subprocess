int subprocess_init(void) {
	return 0;
}

#define subprocess_pipe_err(n) \
	{ \
		char _pipeout = (char)n; \
		write(pipe_fork[1],&_pipeout,1); \
		close(pipe_fork[1]); \
		exit(n); \
		return n; \
	}

int subprocess_open(subprocess_proc_t * output, char ** cmd) {
	// This pipe is used to get library errors from the forked process
	// [0] = read
	// [1] = write
	int pipe_fork[2];
	
	if (pipe(pipe_fork) == -1) {
		return SUBPROCESS_MALLOC;
	}
	
	pid_t pid = fork();
	if (pid == -1) {
		close(pipe_fork[0]);
		close(pipe_fork[1]);
		return SUBPROCESS_MALLOC;
	}
	
	if (pid == 0) { // Child
		close(pipe_fork[0]);
		
		int flags = fcntl(pipe_fork[1], F_GETFD);
		if (flags == -1) {
			subprocess_pipe_err(SUBPROCESS_OTHER);
		}
		
		if (fcntl(pipe_fork[1], F_SETFD, flags | FD_CLOEXEC) == -1) {
			subprocess_pipe_err(SUBPROCESS_OTHER);
		}
		
		execvp(cmd[0],cmd);
		subprocess_pipe_err(SUBPROCESS_OTHER);
	} else { // Parent
		close(pipe_fork[1]);
		
		char rtn;
		ssize_t n = read(pipe_fork[0], &rtn, 1);
		close(pipe_fork[0]);
		
		// Any error state
		if ((n == -1) || (n == 1)) {
			waitpid(pid,NULL,0);
			
			// Expected-ish error (?)
			if (n == 1) { return (int)rtn; }
			
			// Unknown
			return SUBPROCESS_OTHER;
		}
		
		output->pid = pid;
		return 0;
	}
}

int subprocess_wait(subprocess_return_t * rtn, subprocess_proc_t * proc, subprocess_time_t max_wait) {
	if (max_wait == 0) {
		if (waitpid(proc->pid,rtn,0) < 1) {
			return SUBPROCESS_OTHER;
		}
	} else {
		struct timeval t;
		if (gettimeofday(&t,NULL) != 0) { return SUBPROCESS_OTHER; }
		struct timeval start = t;
		subprocess_time_t elapsed = 0;
		
		int r;
		while (elapsed < max_wait) {
			r = waitpid(proc->pid,rtn,WNOHANG);
			if (r != 0) { break; }
			if (gettimeofday(&t,NULL) != 0) { return SUBPROCESS_OTHER; }
			usleep(SUBPROCESS_CHECK_TIME * 1000);
			elapsed = ((t.tv_sec - start.tv_sec) * 1000) - (start.tv_usec / 1000) + (t.tv_usec / 1000);
		}
		
		if (r == 0) { return SUBPROCESS_EOF; }
		if (r == -1) { return SUBPROCESS_OTHER; }
	}
	return 0;
}

int subprocess_kill(subprocess_proc_t * proc) {
	if (kill(proc->pid,SIGKILL) == -1) { return SUBPROCESS_OTHER; }
	return 0;
}

int subprocess_terminate(subprocess_proc_t * proc) {
	if (kill(proc->pid,SIGTERM) == -1) { return SUBPROCESS_OTHER; }
	return 0;
}

int subprocess_suspend(subprocess_proc_t * proc) {
	if (kill(proc->pid,SIGSTOP) == -1) { return SUBPROCESS_OTHER; }
	return 0;
}

int subprocess_resume(subprocess_proc_t * proc) {
	if (kill(proc->pid,SIGCONT) == -1) { return SUBPROCESS_OTHER; }
	return 0;
}
