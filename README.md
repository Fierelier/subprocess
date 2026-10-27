A Python-inspired cross-platform subprocess library for C99.

## Compatibility
Supports Linux and Windows 95+ (MinGW). It probably also works with other UNIXes, like MacOS and *BSD.

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
**All** (!) functions may return an error. `0` is success, `>0` means an error or a special event occured:
- `SUBPROCESS_OTHER`: Some unknown error occured.
- `SUBPROCESS_MALLOC`: Error likely related to resource limitations.
- `SUBPROCESS_EOF`: The end of something has been reached. Usually not an error.

---

```c
int subprocess_init(void)
```

Initializes the library. Only call once.

---

```c
int subprocess_open(
	subprocess_proc_t * output,
	subprocess_char_t ** cmd,
	subprocess_pipe_t * proc_stdout,
	subprocess_pipe_t * proc_stderr,
	subprocess_pipe_t * proc_stdin
);
```

Opens a process and puts the process data into `output`.

`cmd` is the commandline, it is a list of the executable file (the first argument), some optional arguments, and `NULL`. It may look like this:

```c
subprocess_char_t * cmd[] = {TEXT("sh"),TEXT("-c"),TEXT("echo hello world"),NULL}
```

This would execute the equivalent of `sh -c "echo hello world"`.

`proc_std*` are the pipes, either pass NULL, to leave the pipes unbound (use the parent's pipes) or pass pipes that you've created with `subprocess_pipe_create`.

**NOTE!** If you summon a console program, make sure that at least one of these is true:  
- Your program is a console program
- You have at least one allocated console (you called `AllocConsole()`)
- Every pipe (stdout/stderr/stdin) is bound

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
int subprocess_pipe_create(
	subprocess_pipe_t * output
)
```

Create a stdio pipe. You can use it to make the input/output accessible when creating a process with `subprocess_open`.

It is important that you use a pipe for only one direction at once. Only use it for a maximum of 1 output and/or 1 input.

You can, for example, reuse process 1's stdout (or stderr) as process 2's stdin or vice versa. The stdout of process 1 will go into process 2's stdin.

Otherwise, do not reuse the pipe. The pipe potentially exists in memory until you free it with `subprocess_pipe_free`. You best call it after waiting for the processes that use it with `subprocess_wait`.

---

```c
int subprocess_pipe_read(
	subprocess_pipe_t * p,
	subprocess_pipesize_t size,
	subprocess_pipesize_t * size_out,
	char * output
)
```

Read up to `size` bytes from a pipe `p` into `output`. Puts the actual size returned into `size_out`. If it returns `SUBPROCESS_EOF`, all data has been returned. In this case, do not call read on this pipe again. If the return is `0`, more data can be read.

---

```c
int subprocess_pipe_write(
	subprocess_pipe_t * p,
	subprocess_pipesize_t size,
	char * input
)
```

Write `size` bytes of `input` into pipe `p`. If it returns `SUBPROCESS_EOF`, that side of the pipe is closed and no more data can be sent. In this case, do not call write on this pipe again. If the return is `0`, more data can be written.

---

```c
int subprocess_pipe_free(
	subprocess_pipe_t * pipe
)
```

Free a pipe from memory. The pipe should preferably not be in use anymore (use `subprocess_wait` on the processes it's used in). It's okay to free pipes that have already been freed.

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

Note: If the binary is compiled for Windows versions older than Windows 2000, this is a no-op, and always just returns as if it was successful.
