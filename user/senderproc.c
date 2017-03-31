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
        write(1, "gg", 2);
        //read(fd[0], buffer, sizeof(buffer));
    }
    else {
        if(fd[0] == 3) {
            write(1, "ff", 2);

            //problem
            write(fd[0], "test\n", 5);
        }

    }
    //write(fd[0], "test\n", 5);
    //read(fd[0], buffer, 5);
    write(1, "asd", 3);
    write(1, "fgh", 3);
    //writeOut(buffer);

    //child & parent both use pipe


    exit(EXIT_SUCCESS);
}

//http://tldp.org/LDP/lpg/node11.html
