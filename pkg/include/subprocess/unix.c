int subprocess_init(void) {
	return 0;
}

#define subprocess_pipe_err(n) \
	{ \
		char _pipeout = (char)n; \
		write(pipe_fork[1],&_pipeout,1); \
		clnup_cll(cleanup); \
		exit(n); \
		return n; \
	}

int subprocess_open(subprocess_proc_t * output, char ** cmd, subprocess_pipe_t * proc_stdout, subprocess_pipe_t * proc_stderr, subprocess_pipe_t * proc_stdin) {
	clnup_srt(cleanup);
		clnup_var(cleanup, pid_t,pid,-1, kill(pid,SIGKILL); waitpid(pid,NULL,0));
		clnup_tbl(cleanup, int,pipe_fork[2]);
		clnup_tbs(cleanup, pipe_fork[0],-1, close(pipe_fork[0]));
		clnup_tbs(cleanup, pipe_fork[1],-1, close(pipe_fork[1]));
	clnup_end(cleanup, SUBPROCESS_OTHER);
	
	if (pipe((int *)pipe_fork) == -1) {
		clnup_cll(cleanup);
		return SUBPROCESS_MALLOC;
	}
	
	pid = fork();
	if (pid == -1) {
		clnup_cll(cleanup);
		return SUBPROCESS_MALLOC;
	}
	
	int drtn;
	if (pid == 0) { // Child
		clnup_cvr(pipe_fork[0],-1,close(pipe_fork[0]));
		
		int flags = fcntl(pipe_fork[1], F_GETFD);
		if (flags == -1) {
			subprocess_pipe_err(SUBPROCESS_OTHER);
		}
		
		if (fcntl(pipe_fork[1], F_SETFD, flags | FD_CLOEXEC) == -1) {
			subprocess_pipe_err(SUBPROCESS_OTHER);
		}
		
		if (proc_stdout != NULL) {
			drtn = dup2(proc_stdout->pipe[1],STDOUT_FILENO);
			close(proc_stdout->pipe[0]);
			if (drtn == -1) {
				close(proc_stdout->pipe[1]);
				subprocess_pipe_err(SUBPROCESS_MALLOC);
			}
		}
		
		if (proc_stderr != NULL) {
			drtn = dup2(proc_stderr->pipe[1],STDERR_FILENO);
			close(proc_stderr->pipe[0]);
			if (drtn == -1) {
				close(proc_stderr->pipe[1]);
				subprocess_pipe_err(SUBPROCESS_MALLOC);
			}
		}
		
		if (proc_stdin != NULL) {
			drtn = dup2(proc_stdin->pipe[0],STDIN_FILENO);
			close(proc_stdin->pipe[1]);
			if (drtn == -1) {
				close(proc_stdin->pipe[0]);
				subprocess_pipe_err(SUBPROCESS_MALLOC);
			}
		}
		
		execvp(cmd[0],cmd);
		
		// This should not run
		if (proc_stdout != NULL) { close(proc_stdout->pipe[1]); }
		if (proc_stderr != NULL) { close(proc_stderr->pipe[1]); }
		if (proc_stdin != NULL) { close(proc_stdin->pipe[0]); }
		subprocess_pipe_err(SUBPROCESS_OTHER);
	} else { // Parent
		clnup_cvr(pipe_fork[1],-1,close(pipe_fork[1]));
		
		char rtn;
		ssize_t n = read(pipe_fork[0], &rtn, 1);
		clnup_cvr(pipe_fork[0],-1,close(pipe_fork[0]));
		
		// Any error state
		if ((n == -1) || (n == 1)) {
			waitpid(pid,NULL,0);
			pid = -1;
			clnup_cll(cleanup);
			
			// Proper error return
			if (n == 1) { return (int)rtn; }
			
			// Error return failed
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
			if (max_wait == 1) { break; }
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

int subprocess_pipe_create(subprocess_pipe_t * p) {
	if (pipe(p->pipe) == -1) {
		return SUBPROCESS_MALLOC;
	}
	return 0;
}

int subprocess_pipe_free(subprocess_pipe_t * p) {
	close(p->pipe[0]);
	close(p->pipe[1]);
	return 0;
}
