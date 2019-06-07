#include "proc.h"

#ifndef _WIN32
extern char** environ;
static int coe(int fd) {
    int flags = fcntl(fd, F_GETFD, 0) ;
    if (flags < 0) return -1 ;
    return fcntl(fd, F_SETFD, flags | FD_CLOEXEC) ;
}
#endif

#ifdef _WIN32
static unsigned int strcat_escape(char *d, const char *s) {
    if(d != NULL) d += lstrlen(d);

    int n = 0;
    char echar = '\0';
    int ecount = 0;
    while(*s) {
        ecount = 0;
        switch(*s) {
            case '"':  ecount=1; echar='\\'; break;
            case '\\': {
                if(lstrlen(s) == 1) {
                    ecount=1;echar='\\';
                }
                else {
                    if(*(s+1) == '"') {
                        ecount=1;echar='\\';
                    }
                }
            }
            break;
            default: break;
        }
        n += ecount + 1;
        if(d != NULL) {
            while(ecount) {
                *d++ = echar;
                ecount--;
            }
            *d++ = *s;
        }
        s++;
    }

    if(d != NULL) *d = '\0';
    return n;
}

#endif

static void proc_info_init(proc_info *info) {
#ifdef _WIN32
    info->handle = INVALID_HANDLE_VALUE;
    info->pid = -1;
#else
    info->pid = -1;
#endif
}

int proc_info_wait(proc_info *info) {
    int ecode = -1;
#ifdef _WIN32
    DWORD exitCode;
    if(WaitForSingleObject(info->handle,INFINITE) != 0) {
        return -1;
    }
    if(!GetExitCodeProcess(info->handle,&exitCode)) {
        return -1;
    }
    HeapFree(GetProcessHeap(),0,info);
    ecode = (int)exitCode;
#else
    int st;
    waitpid(info->pid,&st,0);
    if(WIFEXITED(st)) ecode = WEXITSTATUS(st);
    free(info);
#endif
    return ecode;
}


void proc_pipe_init(proc_pipe *pipe) {
#ifdef _WIN32
    pipe->pipe = INVALID_HANDLE_VALUE;
#else
    pipe->pipe = -1;
#endif
}

int proc_pipe_write(proc_pipe *pipe, const char *buf, unsigned int len) {
#ifdef _WIN32
    DWORD numBytes;
    if(WriteFile(
        pipe->pipe,
        buf,
        len,
        &numBytes,
        NULL) == 0) return -1;
    return (int)numBytes;
#else
    return write(pipe->pipe,buf,len);
#endif
}

int proc_pipe_read(proc_pipe *pipe, char *buf, unsigned int len) {
#ifdef _WIN32
    DWORD numBytes;
    if(ReadFile(
        pipe->pipe,
        buf,
        len,
        &numBytes,
        NULL) == 0) return -1;
    return (int)numBytes;
#else
    return read(pipe->pipe,buf,len);
#endif
}

int proc_pipe_close(proc_pipe *pipe) {
#ifdef _WIN32
    BOOL r;
    if(pipe->pipe == INVALID_HANDLE_VALUE) return 0;
    r = CloseHandle(pipe->pipe);
    if(r) pipe->pipe = INVALID_HANDLE_VALUE;
    return !r;
#else
    int r;
    r = close(pipe->pipe);
    if(r == 0) pipe->pipe = -1;
    return r;
#endif
}



proc_info *proc_spawn(const char * const *argv, proc_pipe *in, proc_pipe *out, proc_pipe *err) {
    proc_info *info = NULL;
#ifdef _WIN32
    char *cmdLine = NULL;
    unsigned int args_len = 0;

    SECURITY_ATTRIBUTES *sa = NULL;
    PROCESS_INFORMATION *pi = NULL;
    STARTUPINFO *si = NULL;

    HANDLE childStdInRd = INVALID_HANDLE_VALUE;
    HANDLE childStdInWr = INVALID_HANDLE_VALUE;
    HANDLE childStdOutRd = INVALID_HANDLE_VALUE;
    HANDLE childStdOutWr = INVALID_HANDLE_VALUE;
    HANDLE childStdErrRd = INVALID_HANDLE_VALUE;
    HANDLE childStdErrWr = INVALID_HANDLE_VALUE;

    const char * const *p = argv;

    info = (proc_info *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(proc_info));
    if(info == NULL) {
        goto error;
    }

    sa = (SECURITY_ATTRIBUTES *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(SECURITY_ATTRIBUTES));
    if(sa == NULL) {
        goto error;
    }

    pi = (PROCESS_INFORMATION *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(PROCESS_INFORMATION));
    if(pi == NULL) {
        goto error;
    }

    si = (STARTUPINFO *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(STARTUPINFO));
    if(si == NULL) {
        goto error;
    }

    while(*p != NULL) {
        args_len += strcat_escape(NULL,*p) + 3; /* +1 space, +2 quote */
        p++;
    }
    args_len += 25; /* null terminator, plus the api guide suggests having extra memory or something */

    cmdLine = (char *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,args_len);
    if(cmdLine == NULL) {
        goto error;
    }

    p = argv;
    lstrcat(cmdLine,"\"");
    strcat_escape(cmdLine,*p);
    lstrcat(cmdLine,"\"");
    p++;

    while(*p != NULL) {
        lstrcat(cmdLine," \"");
        strcat_escape(cmdLine,*p);
        lstrcat(cmdLine,"\"");
        p++;
    }

    proc_info_init(info);

    sa->nLength = sizeof(SECURITY_ATTRIBUTES);
    sa->lpSecurityDescriptor = NULL;
    sa->bInheritHandle = TRUE;

    if(in != NULL) {
        if(in->pipe == INVALID_HANDLE_VALUE) {
            if(!CreatePipe(&childStdInRd, &childStdInWr, sa, 0)) {
                goto error;
            }

            if(!SetHandleInformation(childStdInWr, HANDLE_FLAG_INHERIT, 0)) {
                goto error;
            }
            in->pipe = childStdInWr;
        }
        else {
            if(!DuplicateHandle(GetCurrentProcess(),in->pipe,GetCurrentProcess(),&childStdInRd,0,TRUE,DUPLICATE_SAME_ACCESS)) {
                goto error;
            }
        }
    }

    if(out != NULL) {
        if(out->pipe == INVALID_HANDLE_VALUE) {
            if(!CreatePipe(&childStdOutRd, &childStdOutWr, sa, 0)) {
                goto error;
            }

            if(!SetHandleInformation(childStdOutRd, HANDLE_FLAG_INHERIT, 0)) {
                goto error;
            }
            out->pipe = childStdOutRd;
        }
        else {
            if(!DuplicateHandle(GetCurrentProcess(),out->pipe,GetCurrentProcess(),&childStdOutWr,0,TRUE,DUPLICATE_SAME_ACCESS)) {
                goto error;
            }
        }
    }

    if(err != NULL) {
        if(err->pipe == INVALID_HANDLE_VALUE) {
            if(!CreatePipe(&childStdErrRd, &childStdErrWr, sa, 0)) {
                goto error;
            }

            if(!SetHandleInformation(childStdErrRd, HANDLE_FLAG_INHERIT, 0)) {
                goto error;
            }
            err->pipe = childStdErrRd;
        }
        else {
            if(!DuplicateHandle(GetCurrentProcess(),err->pipe,GetCurrentProcess(),&childStdErrWr,0,TRUE,DUPLICATE_SAME_ACCESS)) {
                goto error;
            }
        }
    }

    si->cb = sizeof(STARTUPINFO);
    si->dwFlags |= STARTF_USESTDHANDLES;

    if(childStdInRd != INVALID_HANDLE_VALUE) {
        si->hStdInput = childStdInRd;
    } else {
        si->hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    }

    if(childStdOutWr != INVALID_HANDLE_VALUE) {
        si->hStdOutput = childStdOutWr;
    } else {
        si->hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if(childStdErrWr != INVALID_HANDLE_VALUE) {
        si->hStdError = childStdErrWr;
    } else {
        si->hStdError = GetStdHandle(STD_ERROR_HANDLE);
    }

    if(!CreateProcess(NULL,
        cmdLine,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        si,
        pi)) goto error;

    CloseHandle(pi->hThread);

    info->handle = pi->hProcess;
    info->pid = pi->dwProcessId;

    if(childStdInRd != INVALID_HANDLE_VALUE) {
        if(childStdInRd != in->pipe) {
            CloseHandle(childStdInRd);
        }
    }

    if(childStdOutWr != INVALID_HANDLE_VALUE) {
        if(childStdOutWr != out->pipe) {
            CloseHandle(childStdOutWr);
        }
    }

    if(childStdErrWr != INVALID_HANDLE_VALUE) {
        if(childStdErrWr != err->pipe) {
            CloseHandle(childStdErrWr);
        }
    }

    goto success;

#else
    char *path;
    char argv0[4096];
    int argv0len;
    pid_t pid;
    char *t;
    int in_fds[2] = { -1, -1 };
    int out_fds[2] = { -1, -1 };
    int err_fds[2] = { -1, -1 };

    path = NULL;
    argv0[0] = '\0';
    argv0len = 0;
    pid = -1;
    t = NULL;

    path = getenv("PATH");
    if(path == NULL) path = "/usr/bin:/usr/sbin:/bin:/sbin";

    info = (proc_info *)malloc(sizeof(proc_info));
    proc_info_init(info);

    if(in != NULL) {
        if(in->pipe == -1) {
            if(pipe(in_fds) != 0) goto error;
            if(coe(in_fds[1]) == -1) goto error;
            in->pipe = in_fds[1];
        } else {
            in_fds[0] = in->pipe;
        }
    }

    if(out != NULL) {
        if(out->pipe == -1) {
            if(pipe(out_fds) != 0) goto error;
            if(coe(out_fds[0]) == -1) goto error;
            out->pipe = out_fds[0];
        } else {
            out_fds[1] = out->pipe;
        }
    }

    if(err != NULL) {
        if(err->pipe == -1) {
            if(pipe(err_fds) != 0) goto error;
            if(coe(err_fds[0]) == -1) goto error;
            err->pipe = err_fds[0];
        } else {
            err_fds[1] = err->pipe;
        }
    }

    pid = fork();
    if(pid == -1) {
        goto error;
    }
    if(pid == 0) {
        if(in_fds[0] != -1) {
            dup2(in_fds[0],0);
            close(in_fds[0]);
        }

        if(out_fds[1] != -1) {
            dup2(out_fds[1],1);
            close(out_fds[1]);
        }

        if(err_fds[1] != -1) {
            dup2(err_fds[1],2);
            close(err_fds[1]);
        }

        if(strchr(argv[0],'/') == NULL) {
            while(path) {
                strcpy(argv0,path);
                t = strchr(argv0,':');
                if(t != NULL) {
                    *t = '\0';
                }

                if(strlen(argv0)) {
                    strcat(argv0,"/");
                }
                if(strlen(argv0) + argv0len < 4095) {
                    strcat(argv0,argv[0]);
                }
                execve(argv0,(char * const*)argv,environ);

                if(errno != ENOENT) exit(1);
                t = strchr(path,':');
                if(t) {
                    path = t + 1;
                }
                else {
                    path = 0;
                }
            }
            exit(1);
        }
        else {
            execve(argv[0],(char * const*)argv,environ);
            exit(1);
        }
    }

    if(in_fds[0] != -1) {
        close(in_fds[0]);
    }
    if(out_fds[1] != -1) {
        close(out_fds[1]);
    }
    if(err_fds[1] != -1) {
        close(err_fds[1]);
    }

    info->pid = pid;
    goto success;
#endif

error:
#ifdef _WIN32
    if(info != NULL) HeapFree(GetProcessHeap(),0,info);

    if(childStdInRd != INVALID_HANDLE_VALUE) {
        CloseHandle(childStdInRd);
    }

    if(childStdInWr != INVALID_HANDLE_VALUE) {
        CloseHandle(childStdInWr);
    }

    if(childStdOutRd != INVALID_HANDLE_VALUE) {
        CloseHandle(childStdOutRd);
    }

    if(childStdOutWr != INVALID_HANDLE_VALUE) {
        CloseHandle(childStdOutWr);
    }

    if(childStdErrRd != INVALID_HANDLE_VALUE) {
        CloseHandle(childStdErrRd);
    }

    if(childStdErrWr != INVALID_HANDLE_VALUE) {
        CloseHandle(childStdErrWr);
    }
#else
    free(info);
    if(in_fds[0] > -1) close(in_fds[0]);
    if(in_fds[1] > -1) close(in_fds[1]);
    if(out_fds[0] > -1) close(out_fds[0]);
    if(out_fds[1] > -1) close(out_fds[1]);
    if(err_fds[0] > -1) close(err_fds[0]);
    if(err_fds[1] > -1) close(err_fds[1]);
#endif
    info = NULL;

success:
#ifdef _WIN32
    if(cmdLine != NULL) HeapFree(GetProcessHeap(),0,cmdLine);
    if(sa != NULL) HeapFree(GetProcessHeap(),0,sa);
    if(pi != NULL) HeapFree(GetProcessHeap(),0,pi);
    if(si != NULL) HeapFree(GetProcessHeap(),0,si);
#endif

    return info;
}

int proc_pipe_open_file(proc_pipe *pipe, const char *filename, const char *mode) {
#ifdef _WIN32
    DWORD disp = 0;
    DWORD access = 0;
    switch(mode[0]) {
        case 'r': {
            access = GENERIC_READ;
            disp = OPEN_EXISTING;
            break;
        }
        case 'w': {
            access = GENERIC_WRITE;
            disp = CREATE_ALWAYS;
            break;
        }
        case 'a': {
            access = GENERIC_WRITE;
            disp = OPEN_ALWAYS;
            break;
        }
        default: return -1;
    }
    mode++;
    switch(mode[0]) {
        case '+': access = GENERIC_READ | GENERIC_WRITE;
    }
    pipe->pipe = CreateFile(filename,access,0,NULL,disp,0,0);
    if(pipe->pipe == INVALID_HANDLE_VALUE) return 1;
    if(disp == OPEN_ALWAYS) {
        SetFilePointer(pipe->pipe,0,0,FILE_END);
    }
    return 0;
#else
    int flags = 0;
    int wr = 0;
    switch(mode[0]) {
        case 'r': {
            flags |= O_RDONLY;
            break;
        }
        case 'w': {
            wr = 1;
            break;
        }
        case 'a': {
            wr = 2;
            break;
        }
        default: return 1;
    }
    mode++;
    switch(mode[0]) {
        case '+': flags = O_RDWR; break;
        default: if(wr) flags |= O_WRONLY;
    }

    switch(wr) {
        case 1: {
            flags |= O_CREAT;
            flags |= O_TRUNC;
            break;
        }
        case 2: {
            flags |= O_CREAT;
            flags |= O_APPEND;
            break;
        }
    }
    pipe->pipe = open(filename,flags,0666);
    if(pipe->pipe == -1) return 1;
    return 0;
#endif

}
