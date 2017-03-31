#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "libc.h"

extern void receiverMain();




void senderMain() {
    char buffer[10];
    int fd1[2], fd2[2], fd3[2];
    //make pipes
    pipe(fd1);
    pipe(fd2);
    //fork child
    pid_t childId = fork(1);

    if(childId == 0) {

        while(read(fd1[0], buffer, sizeof(buffer))) {
            continue;
        }
        write(1, buffer, sizeof(buffer));
        write(fd2[1], "second\n", 7);

        while(read(fd1[0], buffer, sizeof(buffer))) {
            continue;
        }
        write(1, buffer, sizeof(buffer));
        write(fd2[1], "fourth\n", 7);
    }
    else {
        
        write(fd1[1], "first\n", 6);
        while(read(fd2[0], buffer, sizeof(buffer))) {
            continue;
        }
        write(1, buffer, sizeof(buffer));

        write(fd1[1], "third\n", 6);
        while(read(fd2[0], buffer, sizeof(buffer))) {
            continue;
        }
        write(1, buffer, sizeof(buffer));
    }

    exit(EXIT_SUCCESS);
}   