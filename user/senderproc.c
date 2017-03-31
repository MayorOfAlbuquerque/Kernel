#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "libc.h"

extern void receiverMain();


void senderMain() {
    char buffer[5];
    int fd[2];
    //make pipe
    pipe(fd);
    //fork child
    pid_t childId = fork(1);

    if(childId == 0) {
        write(1, "AA", 2);
        read(fd[0], buffer, sizeof(buffer));
        write(1, buffer, sizeof(buffer));
    }
    else {
        write(1, "BB", 2);
        //problem
        write(fd[1], "test\n", 5);

    }
    //child & parent both use pipe

    exit(EXIT_SUCCESS);
}

//http://tldp.org/LDP/lpg/node11.html
