#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "libc.h"

/*
void forking(int depth, int *fdArray1[2], int *fdArray2[2], int *fdArray3[2], int *fdArray4[2]) {
    depth = depth -1;
    if(depth > 0) {
        fork();
        if(pid == 0) {
            //close end of pipes: fd1[0], fd2[1]

            write(FD1[1]);
            //read reply FD2[0]


            //fork params based on previous use of fdarrays
            forking(depth);
        }
        else {
            //close pipes

            //read right first FD1[0]
            //reply FD2[1]


        }
    }

}


void main(int argc, char const *argv[])
{
    int *FD1[2];
    int *FD2[2];
    int *FD3[2];
    int *FD4[2];
    char *buffer;
    int depth = 4;
    //make pipe


    forking(depth);
    if(pid ==0) {
        //first parent connects left pipe to final child
    }



    return 0;
}*/
