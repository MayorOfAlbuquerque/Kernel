#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "libc.h"

extern void receiverMain();

void senderMain() {
    write( STDOUT_FILENO, "GL", 2 );
    int fd[2];
    pipe(fd);
    if(fd[0] == 3) {
        write( STDOUT_FILENO, "XA", 2 );
    }
    if(fd[1] == 4) {
        write( STDOUT_FILENO, "XA", 2 );
    }
    //make pipe
    write(fd[0], "We did it reddit\n", 17);
    //fork child
    /*
    pid_t childId = fork(1);
    void *addr = &receiverMain;
    exec(addr);
*/
    //child & parent both use pipe


    exit( EXIT_SUCCESS );
}

//http://tldp.org/LDP/lpg/node11.html
