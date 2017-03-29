#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "libc.h"

extern void receiverMain();

void writeOut(char *str) {
    int s = sizeof(str)/sizeof(char);
    write(1, str, s);
}



void senderMain() {
    char *buffer;
    int fd[2];
    //make pipe
    pipe(fd);
    //fork child

    //pid_t childId = fork(1);
    void *addr = &receiverMain;
    write(fd[0], "test\n", 4);
    read(fd[0], buffer, sizeof(buffer));
    writeOut(buffer);


    
    /*if(0 == childId) {
        read(fd[0], buffer, sizeof(buffer));
        write(1, buffer, 10);
    }
    else {
        write(fd[0], "test\n", 4);
    }*/
    //child & parent both use pipe


    exit(EXIT_SUCCESS);
}

//http://tldp.org/LDP/lpg/node11.html
