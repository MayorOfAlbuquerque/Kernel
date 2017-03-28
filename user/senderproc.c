#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "libc.h"

extern void receiverMain();

void senderMain() {
    write( STDOUT_FILENO, "GL", 2 );
    int fd[2];
    //make pipe
    pipe(fd);
    //make pipe
    write(fd[0], "We did\n", 6);
    //fork child
/*
    pid_t childId = fork(1);
    void *addr = &receiverMain;
    if(0 == childId) {
        exec(addr);
    }*/
    //child & parent both use pipe


    exit( EXIT_SUCCESS );
}

//http://tldp.org/LDP/lpg/node11.html
