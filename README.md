# proc

A small, easy library to spawn processes in a cross-platform way.

Supports chaining spawned processes together with pipes.

On Windows, this only uses the Win32 API and just needs to link
against kernel32.dll.

So far I've only tested with mingw gcc.

## Types

* `proc_info` - a struct to store the process
* `proc_pipe` - a wrapper around file descriptors/windows handles

## Functions

* `void proc_pipe_init(proc_pipe *)` - call this before using a `proc_pipe`
* `proc_info *proc_spawn(const char *const *argv, proc_pipe *in, proc_pipe *out, proc_pipe *err);`
    * Return value: a dynamically-allocated `proc_info` struct, free'd by `proc_info_wait`
    * `argv` - a NULL-terminated list of strings
    * `in` - pointer to a previously-allocated `proc_pipe` or NULL.
        * if `in` is "unused" (handles/fds are NULL/-1), then `in` becomes a pipe you can write data into
        * if `in` already has a handle/fd, it's connected to the process stdin.
    * `out` - pointer to a previously-allocated `proc_pipe` or NULL.
        * if `out` is "unused" (handles/fds are NULL/-1), then `out` becomes a pipe you can read data from
        * if `out` already has a handle/fd, it's connected to the process stdout.
    * `err` - pointer to a previously-allocated `proc_pipe` or NULL.
        * if `err` is "unused" (handles/fds are NULL/-1), then `err` becomes a pipe you can read data from
        * if `err` already has a handle/fd, it's connected to the process stderr.
* `int proc_info_wait(proc_info *)` - waits on a process, frees `proc_info`
* `int proc_pipe_write(proc_pipe *, const char *, unsigned int len)` - write data into a pipe
    * returns the number of bytes written, -1 on error/eof
* `int proc_pipe_read(proc_pipe *, char *, unsigned int len)` - read data from a pipe
    * returns the number of bytes read, -1 on error/eof
* `int proc_pipe_close(proc_pipe *)` - close a pipe
    * returns 0 on success

## Example

See `demo.c` for a small example program that chains some programs together.
