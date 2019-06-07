/* this is just used for win32 demo purposes */
/* just reads in some text and slaps the program
 * name at the front, so you know where this came from */

/* the final output of demo.exe should produce:
 * demo: lolcat: cat: hello there!
 * Indcating that "hello there!" was piped into
 * "cat", then "lolcat", and finally back into "demo" */
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

char buffer[1024];

int main(int argc, char *argv[]) {
    (void)argc;
#ifdef _WIN32
    _setmode(fileno(stdin),O_BINARY);
    _setmode(fileno(stdout),O_BINARY);
#endif
    buffer[0] = 0;
    sprintf(buffer,"%s: ",argv[0]);
    unsigned int r = 0;
    do {
        r = fread(buffer + strlen(buffer),1,1023 - strlen(buffer),stdin);
        if(r > 0) {
            fwrite(buffer,1,strlen(buffer),stdout);
        }
    } while(r == 1023 - strlen(buffer));
    return 0;
}


