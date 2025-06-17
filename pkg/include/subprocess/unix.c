int subprocess_init(void) {
	return 0;
}

#define subprocess_clean_pipe(pname) \
	if (hook_##pname) { \
		close(pipe_##pname[0]); \
		close(pipe_##pname[1]); \
	}


#define subprocess_clean_pipe_one(pname,n) \
	if (hook_##pname) { \
		close(pipe_##pname[n]); \
	}

#define subprocess_pipe_err(n) \
	{ \
		char _pipeout = (char)n; \
		write(pipe_fork[1],&_pipeout,1); \
		close(pipe_fork[1]); \
		exit(n); \
		return n; \
	}

int subprocess_open(subprocess_proc_t * output, char ** cmd, int hook_stdout, int hook_stderr, int hook_stdin) {
	int pipe_stdout[2];
	int pipe_stderr[2];
	int pipe_stdin[2];
	int pipe_fork[2];
	
	output->i_stdout = -1;
	output->i_stderr = -1;
	output->i_stdin = -1;
	
	if (pipe(pipe_fork) == -1) {
		return SUBPROCESS_MALLOC;
	}
	
	if (hook_stdout) {
		if (pipe(pipe_stdout) == -1) {
			close(pipe_fork[0]);
			close(pipe_fork[1]);
			return SUBPROCESS_MALLOC;
		}
	}
	
	if (hook_stderr) {
		if (pipe(pipe_stderr) == -1) {
			subprocess_clean_pipe(stdout);
			close(pipe_fork[0]);
			close(pipe_fork[1]);
			return SUBPROCESS_MALLOC;
		}
	}
	
	if (hook_stdin) {
		if (pipe(pipe_stdin) == -1) {
			subprocess_clean_pipe(stderr);
			subprocess_clean_pipe(stdout);
			close(pipe_fork[0]);
			close(pipe_fork[1]);
			return SUBPROCESS_MALLOC;
		}
	}
	
	pid_t pid = fork();
	if (pid == -1) {
		subprocess_clean_pipe(stdin);
		subprocess_clean_pipe(stderr);
		subprocess_clean_pipe(stdout);
		close(pipe_fork[0]);
		close(pipe_fork[1]);
		return SUBPROCESS_MALLOC;
	}
	
	if (pid == 0) { // Child
		int drtn;
		close(pipe_fork[0]);
		
		int flags = fcntl(pipe_fork[1], F_GETFD);
		if (flags == -1) {
			subprocess_clean_pipe(stdout);
			subprocess_clean_pipe(stderr);
			subprocess_clean_pipe(stdin);
			subprocess_pipe_err(SUBPROCESS_OTHER);
		}
		
		if (fcntl(pipe_fork[1], F_SETFD, flags | FD_CLOEXEC) == -1) {
			subprocess_clean_pipe(stdout);
			subprocess_clean_pipe(stderr);
			subprocess_clean_pipe(stdin);
			subprocess_pipe_err(SUBPROCESS_OTHER);
		}
		
		if (hook_stdout) {
			drtn = dup2(pipe_stdout[1],STDOUT_FILENO);
			close(pipe_stdout[0]);
			if (drtn == -1) {
				close(pipe_stdout[1]);
				subprocess_clean_pipe(stdin);
				subprocess_clean_pipe(stderr);
				subprocess_pipe_err(SUBPROCESS_MALLOC);
			}
		}
		
		if (hook_stderr) {
			drtn = dup2(pipe_stderr[1],STDERR_FILENO);
			close(pipe_stderr[0]);
			if (drtn == -1) {
				close(pipe_stderr[1]);
				subprocess_clean_pipe_one(stdout,1);
				subprocess_clean_pipe(stdin);
				subprocess_pipe_err(SUBPROCESS_MALLOC);
			}
		}
		
		if (hook_stdin) {
			drtn = dup2(pipe_stdin[0],STDIN_FILENO);
			close(pipe_stdin[1]);
			if (drtn == -1) {
				close(pipe_stdin[1]);
				subprocess_clean_pipe_one(stderr,1);
				subprocess_clean_pipe_one(stdout,1);
				subprocess_pipe_err(SUBPROCESS_MALLOC);
			}
		}
		
		execvp(cmd[0],cmd);
		subprocess_clean_pipe_one(stdout,0);
		subprocess_clean_pipe_one(stderr,0);
		subprocess_clean_pipe_one(stdin,1);
		subprocess_pipe_err(SUBPROCESS_OTHER);
	} else { // Parent
		close(pipe_fork[1]);
		
		if (hook_stdout) {
			close(pipe_stdout[1]);
			output->i_stdout = pipe_stdout[0];
		}
		if (hook_stderr) {
			close(pipe_stderr[1]);
			output->i_stderr = pipe_stderr[0];
		}
		if (hook_stdin) {
			close(pipe_stdin[0]);
			output->i_stdin = pipe_stdin[1];
		}
		
		char rtn;
		ssize_t n = read(pipe_fork[0], &rtn, 1);
		close(pipe_fork[0]);
		
		// Any error state
		if ((n == -1) || (n == 1)) {
			waitpid(pid,NULL,0);
			subprocess_clean_pipe_one(stdout,0);
			subprocess_clean_pipe_one(stderr,0);
			subprocess_clean_pipe_one(stdin,1);
			
			// Expected-ish error
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
	
	if (proc->i_stdout != -1) { close(proc->i_stdout); }
	if (proc->i_stderr != -1) { close(proc->i_stderr); }
	if (proc->i_stdin != -1) { close(proc->i_stdin); }
	return 0;
}

int subprocess_read_stdout(char * output, subprocess_pipesize_t size, subprocess_pipesize_t * size_out, subprocess_proc_t * proc) {
	*size_out = read(proc->i_stdout,output,size);
	if (*size_out == -1) {
		*size_out = 0;
		return SUBPROCESS_OTHER;
	}
	
	if (*size_out < size) {
		return SUBPROCESS_EOF;
	}
	
	return 0;
}

int subprocess_read_stderr(char * output, subprocess_pipesize_t size, subprocess_pipesize_t * size_out, subprocess_proc_t * proc) {
	*size_out = read(proc->i_stderr,output,size);
	if (*size_out == -1) {
		*size_out = 0;
		return SUBPROCESS_OTHER;
	}
	
	if (*size_out < size) {
		return SUBPROCESS_EOF;
	}
	
	return 0;
}

int subprocess_write_stdin(char * input, subprocess_pipesize_t size, subprocess_proc_t * proc) {
	subprocess_pipesize_t size_out = write(proc->i_stdin,input,size);
	if (size_out == -1) {
		return SUBPROCESS_OTHER;
	}
	
	if (size_out < size) {
		return SUBPROCESS_EOF;
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
