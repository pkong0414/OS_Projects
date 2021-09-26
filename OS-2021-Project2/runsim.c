//runsim.c

#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "detachandremove.h"
#include "license.h"
#define PERM (S_IRUSR | S_IWUSR)

#define MAX_CANON 13

/* THINGS TO DO:
 *
 * We need a signal interrupt [ctrl+c]
 *
 * We also need to program a program timer. Default value is 100. Once the time is up,
 * the whole program shuts off no matter what
 *
 * NOTE: Both should detach memory and kill all the processes and end the program accordingly.
 */

licenseList *licenses = NULL;                       //This is our shared heap for licenses

int main( int argc, char* argv[]){

    int opt, timer, nValue;                 //This is for managing our getopts
    int childPid, id, waitId;               //This is for managing our processes
    int *sharedTotal;                       //This is for managing our sharedMemory

    char docommand[MAX_CANON];

    do{
        //We'll be using fgets() for our stdin. "testing.data" is what we will be receiving from.
        fgets(docommand, MAX_CANON, stdin);

        //printing debugging output for fgets received.
        printf("%s", docommand);

        //clearing the buffer
        docommand[0] = '\0';
    }while(!feof(stdin));
    // Getting a new line after reading file
    printf("\n");

    while((opt = getopt(argc, argv, "hn:")) != -1) {
        switch (opt) {
            case 'h':
                //This is the help parameter. We'll be printing out what this program does and will end the program.
                //If this is entered along with others, we'll ignore the rest of the other parameters to print help
                //and end the program accordingly.
                printf("Usage: %s [-h] [-n processes(MAX 20)] < testing.data\n", argv[0]);
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
                        printf("%s: processes cannot be less than 1.\n", argv[0]);
                        nValue = 1;
                    }
                    else if (nValue > 19) {
                        printf("%s: number of processes cannot exceed 20.\n", argv[0]);
                        nValue = 19;
                    }
                    printf("nValue: %d\n", nValue);
                    break;
                }
            default: /* '?' */
                printf("%s: ERROR: parameter not recognized.\n", argv[0]);
                fprintf(stderr, "Usage: %s [-h] [-n number of processes]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    } /* END OF GETOPT */

    if( (id = shmget(IPC_PRIVATE, sizeof(struct licenseList), PERM)) == -1){
        perror("Failed to create shared memory segment\n");
        return 1;
    }

    //********************* SHARED MEMORY PORTION ************************

    // created shared memory segment!
    printf("created shared memory!\n");

    if((sharedTotal = (int *)shmat(id, NULL, 0)) == (void *)-1){
        perror("Failed to attach shared memory segment\n");
        if(shmctl(id, IPC_RMID, NULL) == -1){
            perror("Failed to remove memory segment\n");
        }
        return 1;
    }
    // attached shared memory
    printf("attached shared memory\n");


    //********************* SHARED MEMORY PORTION ************************


    if((childPid = fork()) == -1){
        perror("Failed to create child process\n");
        if(detachandremove(id, sharedTotal) == -1){
            perror("Failed to destroy shared memory segment");
        }
        return 1;
    }

    if(childPid == 0){
        /* the child process */

        *sharedTotal = 123;
        exit(EXIT_SUCCESS);
    }
    // made a child process!
    if((waitId = wait(NULL)) == -1) {
        perror("Failed to wait for child\n");
    } else {
        printf("successfully waited for child process!\n");
    }

    if(detachandremove(id, sharedTotal) == -1){
        perror("Failed to destroy shared memory segment");
        return 1;
    } else {
        printf("Memory segment detached!\n");
    }
    return 0;
}