# jpr_proc

A public domain, single-file library to spawn processes in a cross-platform way.

Supports chaining spawned processes together with pipes, as well
as connecting processes to files for input or output.

On Windows, this only uses the Win32 API and just needs to link
against kernel32.dll.

## Types

* `jpr_proc_info` - a struct to store the process
* `jpr_proc_pipe` - a wrapper around file descriptors/windows handles

## Functions

All functions return 0 on success, non-zero on error;

* `int jpr_proc_info_init(proc_info *info)` - call this before using a `proc_info`
* `int jpr_proc_pipe_init(proc_pipe *pipe)` - call this before using a `proc_pipe`
* `int jpr_proc_spawn(jpr_proc_info info*, const char *const *argv, jpr_proc_pipe *in, jpr_proc_pipe *out, jpr_proc_pipe *err);`
    * `info` - a previously-allocated `jpr_proc_info`
    * `argv` - a NULL-terminated list of strings
    * `in` - pointer to a previously-allocated `jpr_proc_pipe` or NULL.
        * if `in` is "unused" (handles/fds are NULL/-1), then `in` becomes a pipe you can write data into
        * if `in` already has a handle/fd, it's connected to the process stdin.
    * `out` - pointer to a previously-allocated `jpr_proc_pipe` or NULL.
        * if `out` is "unused" (handles/fds are NULL/-1), then `out` becomes a pipe you can read data from
        * if `out` already has a handle/fd, it's connected to the process stdout.
    * `err` - pointer to a previously-allocated `jpr_proc_pipe` or NULL.
        * if `err` is "unused" (handles/fds are NULL/-1), then `err` becomes a pipe you can read data from
        * if `err` already has a handle/fd, it's connected to the process stderr.
* `int jpr_proc_info_wait(jpr_proc_info *, int *exitcode)` - waits on a process, stores exit code
* `int jpr_proc_pipe_write(jpr_proc_pipe *, const char *, unsigned int len, unsigned int *written)` - write data into a pipe
* `int jpr_proc_pipe_read(jpr_proc_pipe *, char *, unsigned int len, unsigned int *read)` - read data from a pipe
* `int jpr_proc_pipe_close(jpr_proc_pipe *)` - close a pipe
    * returns 0 on success

* `int jpr_proc_pipe_open_file(jpr_proc_pipe *, const char *filename, const char *mode)`
    * `mode` is a string. The first character should be "r", "w", or "a" for read/write/append, an optional "+" lets you open in read/write mode.

## Example

See `demo.c` for a small example program that chains some programs together.

