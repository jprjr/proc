#ifndef JPR_PROC_H
#define JPR_PROC_H

#include <stddef.h>
#include <limits.h>

typedef struct jpr_proc_info_s jpr_proc_info;
typedef struct jpr_proc_pipe_s jpr_proc_pipe;

#ifdef __cplusplus
extern "C" {
#endif

jpr_proc_info *jpr_proc_spawn(const char * const *argv, jpr_proc_pipe *in, jpr_proc_pipe *out, jpr_proc_pipe *err);
int jpr_proc_info_wait(jpr_proc_info *);

void jpr_proc_pipe_init(jpr_proc_pipe *);
int jpr_proc_pipe_write(jpr_proc_pipe *, const char *, unsigned int len);
int jpr_proc_pipe_read(jpr_proc_pipe *, char *, unsigned int len);
int jpr_proc_pipe_close(jpr_proc_pipe *);

int jpr_proc_pipe_open_file(jpr_proc_pipe *, const char *filename, const char *mode);

#ifdef __cplusplus
}
#endif

#ifdef _WIN32
#include <windows.h>

struct jpr_proc_info_s {
    HANDLE handle;
    int pid;
};

struct jpr_proc_pipe_s {
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

struct jpr_proc_info_s {
    int pid;
};

struct jpr_proc_pipe_s {
    int pipe;
};

#endif

#endif
