A Python-inspired cross-platform subprocess library for C99. Works on Unix and Windows (MinGW).

## Inlining
**You don't need to compile this as a library.** You can simply include `pkg/include/subprocess/subprocess.c`.

## Compile
`./make.sh` -- You may set `CC`, `CFLAGS` and `LDFLAGS` environment variables. This will create `libsubprocess.so` and the test program: `subprocess-test`.

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
	int hook_stdout,
	int hook_stderr,
	int hook_stdin
);
```

Opens a process and puts the process data into `output`.

`cmd` is the commandline, it is a list of the executable file (the first argument), some optional arguments, and `NULL`. It may look like this:

```c
char * cmd[] = {"sh","-c","echo hello world",NULL}
```

This would execute the equivalent of `sh -c "echo hello world"`.

`hook_std*` are booleans (0 or 1), which decide whether stdout, stderr and stdin should be made interactable. Setting each to 1 allows you to call the corresponding `subprocess_read_std*` and `subprocess_write_stdin` functions, to interact with the process' input/output.

---

```c
int subprocess_wait(
	subprocess_return_t * rtn,
	subprocess_proc_t * proc,
	subprocess_time_t max_wait
);
```

Wait for `proc` to close, and put the resulting status code into `rtn`. Cleans up memory taken up by the object. This should **always** be called, if you intend the process to end (unless the process failed creating when you called `subprocess_open`), even if you terminate or kill the process beforehand.

`max_wait` defines how long to wait for the process to close. Set to `0` for infinite.

If `max_wait` is not `0`, this may return `SUBPROCESS_EOF` on timeout, in which case the object's data has **not** been cleaned, and you should call this function again at some later point, until you've succeeded waiting.

---

```c
int subprocess_read_stdout(
	char * output,
	subprocess_pipesize_t size,
	subprocess_pipesize_t * size_out,
	subprocess_proc_t * proc
);
```

```c
int subprocess_read_stderr(
	char * output,
	subprocess_pipesize_t size,
	subprocess_pipesize_t * size_out,
	subprocess_proc_t * proc
);
```

Read from `proc`'s stdout/stderr. Note, this only works if `hook_stdout` and/or `hook_stderr` were set to `1`.

A maximum of `size` bytes is put into `output`. The number of bytes returned is put into `size_out`.

May return `SUBPROCESS_EOF`, if the pipe has closed (program is done printing). More data may have still been returned when this happens. You should not read from this pipe again after this.

---

```c
int subprocess_write_stdin(
	char * input,
	subprocess_pipesize_t size,
	subprocess_proc_t * proc
);
```

Write `size` bytes of `input` into `proc`'s stdin. Note, this only works if `hook_stdin` was set to `1`.

May return `SUBPROCESS_EOF`, if the pipe has closed (program no longer accepts data). You should not write to this pipe again after this.

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

