/* just echoes arguments */
#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

int main(int argc, char *argv[]) {
    int i=1;
#ifdef _WIN32
    _setmode(fileno(stdout),O_BINARY);
#endif
    while(i<argc) {
        printf("argv[%d]: %s\n",i,argv[i]);
        i++;
    }
    return 0;
}
