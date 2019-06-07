#ifndef _WIN32
#include <stdlib.h>
#include <string.h>
#endif

#include "proc.h"

/* spawns a process, pipes it into another */
/* writes data to first process stdin */

char buffer[1024];

#ifdef _WIN32
int WINAPI mainCRTStartup(void) {
#else
int main(void) {
    char *newpath;
    char *path = getenv("PATH");
    if(path == NULL) path = "/usr/bin:/bin:/usr/sbin:/sbin";
    newpath = malloc(strlen(path) + strlen(".:") + 1);
    if(newpath == NULL) return 1;
    strcpy(newpath,".:");
    strcat(newpath,path);
    setenv("PATH",newpath,1);
    free(newpath);
#endif

    proc_info *child1 = NULL;
    proc_info *child2 = NULL;
    proc_info *child3 = NULL;

    proc_pipe child1_in;
    proc_pipe child1_out;
    proc_pipe child2_out;

    proc_pipe_init(&child1_in);
    proc_pipe_init(&child1_out);
    proc_pipe_init(&child2_out);

    buffer[0] = 0;

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

    if( (child1 = proc_spawn(child1_argv,&child1_in,&child1_out,NULL)) == NULL) {
        return 1;
    }

    if( (child2 = proc_spawn(child2_argv,&child1_out,&child2_out,NULL)) == NULL) {
        return 1;
    }


#ifdef _WIN32
    /* just using lstrlen to avoid needing msvcrt */
    proc_pipe_write(&child1_in,input,lstrlen(input));
#else
    proc_pipe_write(&child1_in,input,strlen(input));
#endif
    proc_pipe_close(&child1_in);
    int r = proc_pipe_read(&child2_out,buffer,1024);
    buffer[r] = 0;

#ifdef _WIN32
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),"demo: ",6,NULL,0);
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),buffer,lstrlen(buffer),NULL,0);
#else
    if(write(1,"demo: ",6) == -1) exit(1);
    if(write(1,buffer,strlen(buffer)) == -1) exit(1);
#endif

    proc_info_wait(child1);
    proc_info_wait(child2);

    if( (child3 = proc_spawn(child3_argv,NULL,NULL,NULL)) == NULL) {
        return 1;
    }
    proc_info_wait(child3);

    return 0;
}

