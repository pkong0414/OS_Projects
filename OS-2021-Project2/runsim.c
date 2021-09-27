//runsim.c

#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "detachandremove.h"
#include "license.h"
#define PERM (IPC_CREAT | S_IRUSR | S_IWUSR)

#define MAX_CANON 13

// FUNCTION PROTOTYPES
void docommand(char *);                             //This function will perform the exec calls
void initShm(key_t myKey);                          //This function will initialize shared memory.
void license_manager( const int i );                //this function will manage the giving and receiving of licenses.
void critical_section();                            //This helper function will operate the critical section.
void createChildren(int);                           //This function will create the children processes from main process
void createGranChildren();                          //This function will be doing the exec functions.
int max(int numArr[], int n);

/* THINGS TO DO:
 *
 * We need a signal interrupt [ctrl+c]
 *
 * We also need to program a program timer. Default value is 100. Once the time is up,
 * the whole program shuts off no matter what
 *
 * NOTE: Both should detach memory and kill all the processes and end the program accordingly.
 */

// shared memory struct
typedef struct sharedMemory {
    bool choosing[MAX_PROC];                            //This will be the queue for the processes.
    int number[MAX_PROC];                           //This is the current turn of the waiting line.
} sharedMem;


// GLOBALS
enum state{idle, want_in, in_cs};
int opt, timer, nValue;                             //This is for managing our getopts
int currentConcurrentProcesses = 1;                 //Initialized as 1 since the main program is also a process.
int totalProcessesCreated = 0;                      //number of created process
int childPid, id, waitStatus;                       //This is for managing our processes
key_t myKey;                                        //Shared memory key
sharedMem *sharedHeap;                              //shared memory object


int main( int argc, char* argv[]){

    //gonna make a signal interrupt here just to see what happens


    char command[MAX_CANON];

    do{
        //We'll be using fgets() for our stdin. "testing.data" is what we will be receiving from.
        fgets(command, MAX_CANON, stdin);

        //printing debugging output for fgets received.
        printf("%s", command);

        //clearing the buffer
        command[0] = '\0';
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
                    // -n gives us the number of licenses available
                    nValue = atoi(optarg);
                    // we will check to make sure nValue is 1 to 20.
                    if (nValue < 1) {
                        printf("%s: processes cannot be less than 1.\n", argv[0]);
                        nValue = 1;
                    }
                    else if (nValue >= MAX_PROC) {
                        printf("%s: 20 is the max number of process.\n", argv[0]);
                        nValue = MAX_PROC-1;
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

    addtolicenses(nValue);
    //creating child processes
    createChildren( nValue );

    //detaching shared memory

    if(detachandremove(id, sharedHeap) == -1){
        perror("Failed to destroy shared memory segment");
        exit(EXIT_FAILURE);
    } else {
        printf("Memory segment detached!\n");
    }

    return 0;
}

void docommand(char *execCommand){

}

void initShm(key_t myKey){
    //********************* SHARED MEMORY PORTION ************************

    if((myKey = ftok(".",1)) == (key_t)-1){
        //if we fail to get our key.
        fprintf(stderr, "Failed to derive key from filename:\n");
        exit(EXIT_FAILURE);
    }
    printf("derived key from, myKey: %d\n", myKey);

    if( (id = shmget(myKey, sizeof(sharedMem), PERM)) == -1){
        perror("Failed to create shared memory segment\n");
        exit(EXIT_FAILURE);
    } else {
        // created shared memory segment!
        printf("created shared memory!\n");

        if ((sharedHeap = (sharedMem *) shmat(id, NULL, 0)) == (void *) -1) {
            perror("Failed to attach shared memory segment\n");
            if (shmctl(id, IPC_RMID, NULL) == -1) {
                perror("Failed to remove memory segment\n");
            }
            exit(EXIT_FAILURE);
        }
        // attached shared memory
        printf("attached shared memory\n");
    }


    //****************** END SHARED MEMORY PORTION ***********************

}

void license_manager(const int i){
    //This function will use getlicense() to gather an idea of how many nlicenses are around.
    //If there are 0 then we be blocking until it is ok to receive a license.

    //We'll be following Bakery's Algo for this one

    do{
        sharedHeap->choosing[i] = 1;
        sharedHeap->number[i] = 1 + sharedHeap->number[max(sharedHeap->number, MAX_PROC)];


    }while(1);

    if(getlicense() == 0){
        printf("We have 0 license.\n");

    } else {

    }

}

void critical_section(){

}

void createChildren( int children ){
    int ticket = 0;
    initlicense();
    while( (currentConcurrentProcesses <= MAX_PROC) && (totalProcessesCreated < children) ) {

        if ((childPid = fork()) == -1) {
            perror("Failed to create child process\n");
            if (detachandremove(id, sharedHeap) == -1) {
                perror("Failed to destroy shared memory segment");
            }
            exit(EXIT_FAILURE);
        }

        currentConcurrentProcesses++;
        totalProcessesCreated++;

        initShm(myKey);

        // made a child process!
        if (childPid == 0) {
            /* the child process */
            printf("current concurrent process %d: myPID: %ld\n", currentConcurrentProcesses, (long) getpid());
            printf("number of children allowed to make: %d\n", children);

            //calling the license_manager to handle everything.
            license_manager(totalProcessesCreated);

            exit(EXIT_SUCCESS);
        } else {
            /* the parent process */

            // waiting for the child process
            if ((childPid = waitpid(childPid, &waitStatus, WNOHANG)) == -1) {
                perror("Failed to wait for child\n");
            } else {
                if( WIFEXITED(waitStatus) ) {
                    currentConcurrentProcesses--;
                    printf("current concurrent process %d\n", currentConcurrentProcesses);
                    printf("Child process successfully exited with status: %d\n", waitStatus);
                }
            }
            printf("total processes created: %d\n", totalProcessesCreated);
            if( children == totalProcessesCreated ){
                break;
            }
        }

    }

}

void createGrandChildren(){

}

int max(int numArr[], int n)
{
    static int max=0;
    int i = 0;
    for(i; i < n; i++) {
        if(numArr[max] < numArr[i]) {
            max=i;
        }
    }

    return max;

}