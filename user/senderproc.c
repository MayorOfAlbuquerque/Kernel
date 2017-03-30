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
    char buffer[5];
    int fd[2];
    //make pipe
    pipe(fd);
    //fork child

    pid_t childId = fork(1);

    if(childId == 0) {
        writeOut("gg");
        //read(fd[0], buffer, sizeof(buffer));
    }
    else {
        writeOut("ff");

        //problem
        write(fd[0], "test\n", 5);
    }
    //write(fd[0], "test\n", 5);
    //read(fd[0], buffer, 5);
    writeOut("asd");
    writeOut("fgh");
    //writeOut(buffer);

    //child & parent both use pipe


    exit(EXIT_SUCCESS);
}

//http://tldp.org/LDP/lpg/node11.html
