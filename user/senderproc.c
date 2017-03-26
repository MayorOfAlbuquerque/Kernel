#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "libc.h"

extern void receiverMain();

void senderMain() {
    write( STDOUT_FILENO, "GL", 2 );
    int fd[2];
    pid_t childId = fork(1);
    void *addr = &receiverMain;
    exec(addr);

    exit( EXIT_SUCCESS );
}

//http://tldp.org/LDP/lpg/node11.html
