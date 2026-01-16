A Python-inspired cross-platform subprocess library for C99. Works on Unix and Windows (MinGW).

## Inlining
**You don't need to compile this as a library.** You can simply include `pkg/include/subprocess/subprocess.c`.

## Compile
`./make.sh` -- You may set `CC`, `CFLAGS` and `LDFLAGS` environment variables. This will create `libsubprocess.so`.

## Pack
`./pack.sh` -- Makes a gzip compressed tar that can be installed.

## Install
`sudo ./install` -- You may set `PREFIX` to install it to a different location (`/usr/local` by default).

Note: With the default prefix, you may need to run `sudo ldconfig /usr/local/lib` to make the linker actually read libraries from it.

## Test
Run `./make-test.sh` and then `./subprocess-test-static` and `./subprocess-test-linked`.

## Functions
**All** (!) functions may return an error. `0` is success, `>0` means an error or a special event occured.

---

```c
int subprocess_init(void)
```

Initializes the library. Only call once.

---

```c
int subprocess_open(
	subprocess_proc_t * output,
	char ** cmd,
	subprocess_pipe_t * proc_stdout,
	subprocess_pipe_t * proc_stderr,
	subprocess_pipe_t * proc_stdin
);
```

Opens a process and puts the process data into `output`.

`cmd` is the commandline, it is a list of the executable file (the first argument), some optional arguments, and `NULL`. It may look like this:

```c
char * cmd[] = {"sh","-c","echo hello world",NULL}
```

This would execute the equivalent of `sh -c "echo hello world"`.

`proc_std*` are the pipes, either pass NULL, to leave the pipes unbound (use the parent's pipes) or pass pipes that you've created with `subprocess_pipe_create`.

---

```c
int subprocess_pipe_create(
	subprocess_pipe_t * output
)
```

Create a stdio pipe. You can use it to make the input/output accessible when creating a process with `subprocess_open`.

It is important that you use a pipe for only one direction at once. Only use it for a maximum of 1 output and/or 1 input.

You can, for example, reuse process 1's stdout (or stderr) as process 2's stdin or vice versa. The stdout of process 1 will go into process 2's stdin.

You may reuse the pipe, when the processes you used it for have closed (use `subprocess_wait`). The pipe exists in memory until you free it with `subprocess_pipe_free`.

---

```c
int subprocess_pipe_free(
	subprocess_pipe_t * pipe
)
```

Free a pipe from memory. The pipe should preferably not be in use anymore (use `subprocess_wait`). Do not free a pipe that you've already freed with this function.

---

```c
int subprocess_wait(
	subprocess_return_t * rtn,
	subprocess_proc_t * proc,
	subprocess_time_t max_wait
);
```

Wait for `proc` to close, and put the resulting status code into `rtn`. Cleans up memory taken up by the object. This should **always** be called, if you intend the process to end (unless the process failed creating when you called `subprocess_open`), even if you terminate or kill the process beforehand.

`max_wait` defines how long to wait for the process to close. Set to `0` for infinite. Set to `1` for an immediate return. `2` and up will wait N milliseconds.

If `max_wait` is not `0`, this may return `SUBPROCESS_EOF` on timeout, in which case the object's data has **not** been cleaned, and you should call this function again at some later point, until you've succeeded waiting.

---

```c
int subprocess_terminate(subprocess_proc_t * proc);
int subprocess_kill(subprocess_proc_t * proc);
```

Terminate (graceful) or kill (forced) `proc`. Even if killed, you still need to call upon `subprocess_wait`, to wait for the process to actually close and have its data cleaned.

Note, on Windows, these both do the same thing (kill).

---

```c
int subprocess_suspend(subprocess_proc_t * proc);
int subprocess_resume(subprocess_proc_t * proc);
```

Suspend (pause) or resume `proc`'s execution. When paused, no CPU usage will occur.

