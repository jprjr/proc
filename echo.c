/* just echoes arguments */
#include <stdio.h>

int main(int argc, char *argv[]) {
    int i=1;
    while(i<argc) {
        printf("argv[%d]: %s\n",i,argv[i]);
        i++;
    }
    return 0;
}
