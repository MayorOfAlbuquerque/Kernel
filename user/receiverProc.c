#include "receiverProc.h"


void receiverMain() {
    //read from fd[3] in kernel
    char readBuffer[6];
    read(3, readBuffer, sizeof(readBuffer));
    write(1, readBuffer, 6);
    exit(EXIT_SUCCESS);
}
