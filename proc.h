#ifndef PROC_H
#define PROC_H

#include <stddef.h>
#include <limits.h>

typedef struct proc_info_s proc_info;
typedef struct proc_pipe_s proc_pipe;

#ifdef __cplusplus
extern "C" {
#endif

proc_info *proc_spawn(const char * const *argv, proc_pipe *in, proc_pipe *out, proc_pipe *err);
int proc_info_wait(proc_info *);

void proc_pipe_init(proc_pipe *);
int proc_pipe_write(proc_pipe *, const char *, unsigned int len);
int proc_pipe_read(proc_pipe *, char *, unsigned int len);
int proc_pipe_close(proc_pipe *);

int proc_pipe_open_file(proc_pipe *, const char *filename, const char *mode);

#ifdef __cplusplus
}
#endif

#ifdef _WIN32
#include <windows.h>

struct proc_info_s {
    HANDLE handle;
    int pid;
};

struct proc_pipe_s {
    HANDLE pipe;
};


#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

struct proc_info_s {
    int pid;
};

struct proc_pipe_s {
    int pipe;
};

#endif

#endif
