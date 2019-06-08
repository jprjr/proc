#define JPR_PROC_IMPLEMENTATION
#include "jpr_proc.h"

#ifndef _WIN32
#include <stdlib.h>
#include <string.h>
#endif

/* spawns a process, pipes it into another */
/* writes data to first process stdin */

#ifdef _DEBUG
#ifdef _WIN32
#define log(s) \
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),s,lstrlen(s),NULL,0); \
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),"\n",1,NULL,0);
#else
#define log(s) \
    write(1,s,strlen(s)); \
    write(1,"\n",1);
#endif
#else
#define log(s)
#endif

#ifdef _WIN32
int WINAPI mainCRTStartup(void) {
#else
int main(void) {
#endif
    int t;
    unsigned int c;
    char buffer[1024];
    jpr_proc_info child1;
    jpr_proc_info child2;
    jpr_proc_info child3;

    jpr_proc_pipe child1_in;
    jpr_proc_pipe child1_out;
    jpr_proc_pipe child2_out;
    jpr_proc_pipe text_in;
    jpr_proc_pipe text_out;

    const char *const child1_argv[] = {
        "cat",
        NULL
    };

    const char *const child2_argv[] = {
        "lolcat",
        NULL
    };

    const char *const child3_argv[] = {
        "echo",
        "1",
        "2",
        "3",
        "C:\\some path\\",
        "a \"quoted\" string",
        "another 'quoted' string",
        "\"a string that begins with a quote",
        "a string that ends with a quote\"",
        "\"a string that begins and ends with a quote\"",
        "'a string that begins and ends with apostrophe'",
        "a string with \\ a backslash",
        "'a string ''\"' that has '\"\"\"random\"'\"'''' quotes and \\ slash \\\"",
        "' a string with [ and ] in there, also () and !  !!!",
        "*.c",
        "*.?",
        "a string with > in it",
        ">",
        "^",
        "&",
        "|",
        "<",
        "\\",
        "C:\\some path",
        "%",
        "$",
        "$SOMEVAR",
        "%SOMEVAR%",
        "%SOMEVAR",
        NULL
    };

    char *input = "hello there!\n";

#ifndef WIN32
    char *newpath;
    char *path;

    newpath = NULL;
    path = getenv("PATH");
    if(path == NULL) path = "/usr/bin:/bin:/usr/sbin:/sbin";
    newpath = malloc(strlen(path) + strlen(".:") + 1);
    if(newpath == NULL) return 1;
    strcpy(newpath,".:");
    strcat(newpath,path);
    setenv("PATH",newpath,1);
    free(newpath);
#endif

    log("init child1");
    jpr_proc_info_init(&child1);
    log("init child2");
    jpr_proc_info_init(&child2);
    log("init child3");
    jpr_proc_info_init(&child3);

    log("init child1_in");
    jpr_proc_pipe_init(&child1_in);
    log("init child1_out");
    jpr_proc_pipe_init(&child1_out);
    log("init child2_out");
    jpr_proc_pipe_init(&child2_out);

    c = 0;
    buffer[c] = 0;


    if(jpr_proc_spawn(&child1,child1_argv,&child1_in,&child1_out,NULL)) {
        return 1;
    }
    log("spawned 1");

    if(jpr_proc_spawn(&child2,child2_argv,&child1_out,&child2_out,NULL)) {
        return 1;
    }
    log("spawned 2");


#ifdef _WIN32
    /* just using lstrlen to avoid needing msvcrt */
    jpr_proc_pipe_write(&child1_in,input,lstrlen(input),&c);
#else
    jpr_proc_pipe_write(&child1_in,input,strlen(input),&c);
#endif
    jpr_proc_pipe_close(&child1_in);

    jpr_proc_pipe_read(&child2_out,buffer,1024,&c);
    buffer[c] = 0;

#ifdef _WIN32
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),"demo: ",6,NULL,0);
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),buffer,lstrlen(buffer),NULL,0);
#else
    if(write(1,"demo: ",6) == -1) exit(1);
    if(write(1,buffer,strlen(buffer)) == -1) exit(1);
#endif

    log("wait 1");
    jpr_proc_info_wait(&child1,&t);
    log("wait 1");
    jpr_proc_info_wait(&child1,&t);
    log("wait 2");
    jpr_proc_info_wait(&child2,&t);
    log("done wait");

    if(jpr_proc_spawn(&child3,child3_argv,NULL,NULL,NULL)) {
        return 1;
    }
    log("spawn 3");
    jpr_proc_info_wait(&child3,&t);

    if(jpr_proc_pipe_open_file(&text_in,"Makefile","r")) return 1;
    if(jpr_proc_spawn(&child1,child1_argv,&text_in,NULL,NULL)) {
        return 1;
    }
    jpr_proc_info_wait(&child1,&t);
    jpr_proc_pipe_close(&text_in);

    if(jpr_proc_pipe_open_file(&text_out,"test.txt","w")) return 1;
    if(jpr_proc_spawn(&child3,child3_argv,NULL,&text_out,NULL)) {
        return 1;
    }
    jpr_proc_info_wait(&child3,&t);
    jpr_proc_pipe_close(&text_out);

    if(jpr_proc_pipe_open_file(&text_out,"test.txt","a")) return 1;
    if(jpr_proc_spawn(&child3,child3_argv,NULL,&text_out,NULL)) {
        return 1;
    }
    jpr_proc_info_wait(&child3,&t);
    jpr_proc_pipe_close(&text_out);

    return 0;
}

