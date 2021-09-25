//runsim.c

#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "detachandremove.h"
#define PERM (S_IRUSR | S_IWUSR)

int main( int argc, char* argv[]){

    int opt, timer, nValue;     //This is for managing our getopts
    int childPid, id;           //This is for managing our processes
    int *sharedTotal;           //This is for managing our sharedMemory

    while((opt = getopt(argc, argv, "hn:")) != -1) {
        switch (opt) {
            case 'h':
                //This is the help parameter. We'll be printing out what this program does and will end the program.
                //If this is entered along with others, we'll ignore the rest of the other parameters to print help
                //and end the program accordingly.
                printf("Usage: %s [-h] [-n processes(MAX 20)]\n", argv[0]);
                printf("This program is a license manager\n");
                exit(EXIT_SUCCESS);
            case 'n':
                if (!isdigit(argv[2][0])) {
                    //This case the user uses -t parameter but entered a string instead of an int.
                    printf("value entered: %s\n", argv[2]);
                    printf("%s: ERROR: -n <number of processes>\n", argv[0]);
                    exit(EXIT_FAILURE);
                } else {
                    //-t is entered with an integer so we assign this to our timeValue.
                    nValue = atoi(optarg);
                    //timeValue cannot have a value of 0. This will prevent that case.
                    if (nValue < 1) {
                        printf("$s: nValue cannot be less than 1.\n", argv[0]);
                        nValue = 1;
                    }
                    else if (nValue > 20) {
                        printf("%s: nValue cannot exceed 20.\n", argv[0]);
                        nValue = 20;
                    }
                    printf("nValue: %d\n", nValue);
                    break;
                }
            default: /* '?' */
                printf("%s: ERROR: parameter not recognized.\n", argv[0]);
                fprintf(stderr, "Usage: %s [-h] [-n number of processes]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}