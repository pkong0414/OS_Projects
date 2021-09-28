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
#include <sys/time.h>
#include "detachandremove.h"
#include "license.h"
#define PERM (IPC_CREAT | S_IRUSR | S_IWUSR)

#define MAX_CANON 13

// FUNCTION PROTOTYPES
void initShm(key_t myKey);                          //This function will initialize shared memory.
void docommand( const int i );                      //This function will manage the giving and receiving of licenses.
void critical_section();                            //This helper function will operate the critical section.
void createChildren(int);                           //This function will create the children processes from main process
void createGranChildren();                          //This function will be doing the exec functions.
void parsingArgs(int argc, char* argv[]);            //This helper function will parse command line args.
void signalHandler(int SIGNAL);                     //This is our signal handler
int max(int numArr[], int n);                       //A function to find our max number in the arrays

// SIGNAL HANDLERS PROTOTYPES
static void myhandler(int SIGNAL);
static void myKillSignalHandler(int SIGNAL);
static int setupUserInterrupt(void);
static int setupinterrupt(void);
static int setupitimer(int TIMER);

/* THINGS TO DO:
 *
 *  We need to consider the case that maybe the file being read gives us more than 20 processes to exec.
 *  This means we should keep the currentConcurrentProcesses to 20 at max (or less).
 *
 *  We also need to rewrite the way we do our critical section. It seems that the whole bit will be on the
 *  getlicense(). Also we need to write logmsg() to also consider critical section since we want all data written
 *  in the order it was processed.
 */

// GLOBALS
enum state{idle, want_in, in_cs};
int opt, timer, nValue;                             //This is for managing our getopts
int currentConcurrentProcesses = 1;                 //Initialized as 1 since the main program is also a process.
int totalProcessesCreated = 0;                      //number of created process
int childPid, waitStatus;                           //This is for managing our processes
key_t myKey;                                        //Shared memory key
int shmID;                                          //shared memoryID
sharedMem *sharedHeap;                              //shared memory object

int main( int argc, char* argv[]){
    //gonna make a signal interrupt here just to see what happens

    char command[MAX_CANON];
    timer = MAX_SECONDS;
    if( setupUserInterrupt() == -1 ){
        perror( "failed to set up a user kill signal.\n");
        return 1;
    }

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

    parsingArgs(argc, argv);

    //setting up interrupts for the timer. We have 100 seconds.
    if (setupinterrupt() == -1) {
        perror("Failed to set up handler for SIGALRM");
        return 1;
    }

    if (setupitimer(timer) == -1) {
        perror("Failed to set up the ITIMER_PROF interval timer");
        return 1;
    }

    // Parsing is finished, now we are allocating and adding to licenses
    initShm(myKey);
    initlicense(sharedHeap);
    addtolicenses(sharedHeap, nValue);

    //creating child processes
    createChildren(nValue);

    //detaching shared memory

    if(detachandremove(shmID, sharedHeap) == -1){
        perror("Failed to destroy shared memory segment");
        exit(EXIT_FAILURE);
    } else {
        printf("Memory segment detached!\n");
    }

    return 0;
}

void initShm(key_t myKey){
    //********************* SHARED MEMORY PORTION ************************

    if((myKey = ftok(".",1)) == (key_t)-1){
        //if we fail to get our key.
        fprintf(stderr, "Failed to derive key from filename:\n");
        exit(EXIT_FAILURE);
    }
    printf("derived key from, myKey: %d\n", myKey);

    if( (shmID = shmget(myKey, sizeof(sharedMem), PERM)) == -1){
        perror("Failed to create shared memory segment\n");
        exit(EXIT_FAILURE);
    } else {
        // created shared memory segment!
        printf("created shared memory!\n");

        if ((sharedHeap = (sharedMem *) shmat(shmID, NULL, 0)) == (void *) -1) {
            perror("Failed to attach shared memory segment\n");
            if (shmctl(shmID, IPC_RMID, NULL) == -1) {
                perror("Failed to remove memory segment\n");
            }
            exit(EXIT_FAILURE);
        }
        // attached shared memory
        printf("attached shared memory\n");
    }
    //****************** END SHARED MEMORY PORTION ***********************
}

void docommand(const int i){
    //This function will use getlicense() to gather an idea of how many nlicenses are around.
    //If there are 0 then we be blocking until it is ok to receive a license.

    //We'll be following Bakery's Algo for this one
    int j;
    int argumentC;
    char **argumentV;

    /* Usage: ./testsim [-s seconds for sleep] [-r number of repeats]
     * We'll need to set up just like this
    */
    if( getlicense(sharedHeap) == 1 ) {
        do {
            sharedHeap->choosing[i] = 1;
            sharedHeap->number[i] = 1 + sharedHeap->number[max(sharedHeap->number, MAX_PROC)];
            sharedHeap->choosing[i] = 0;
            for (j = 0; j < MAX_PROC; j++) {
                printf("process %d is waiting...\n", i);
                while (sharedHeap->choosing[j]);                //wait while someone is choosing
                while ((sharedHeap->number[j]) &&
                       (sharedHeap->number[j], j) < (sharedHeap->number[i], i));
                sleep(1);
            }

            //critical section time!
            critical_section();
            printf("process %d received the license!\n", i);
            sharedHeap->number[i] = 0;                          //giving up the number

            //remainder section
            break;
        } while (1);
        return;
    } else {
        printf("process %d received the license!\n", i);
        return;
    }
}

void critical_section(){
    if(getlicense(sharedHeap) == 0){
        printf("We have 0 license.\n");
    } else {
        printf("We have our license!\n");
    }
}

void createChildren( int children ){
    int myID = 0;

    while( (currentConcurrentProcesses <= MAX_PROC) && (totalProcessesCreated < children) ) {

        if ((childPid = fork()) == -1) {
            perror("Failed to create child process\n");
            if (detachandremove(shmID, sharedHeap) == -1) {
                perror("Failed to destroy shared memory segment");
            }
            exit(EXIT_FAILURE);
        }

        currentConcurrentProcesses++;
        totalProcessesCreated++;


        // made a child process!
        if (childPid == 0) {
            /* the child process */
            myID = totalProcessesCreated;                               //assuming at max I will create 20 procs for now

            //debugging output
            printf("current concurrent process %d: myPID: %ld\n", currentConcurrentProcesses, (long)getpid());
            printf("number of children allowed to make: %d\n", children);

            //calling the docommand to handle the licensing.
            docommand(myID);

            //exiting child process
            exit(EXIT_SUCCESS);
        } else {
            /* the parent process */

            // waiting for the child process
            if ((childPid = waitpid(childPid, &waitStatus, WNOHANG)) == -1) {
                perror("Failed to wait for child\n");
            } else {
                if( WIFEXITED(waitStatus) ) {
                    currentConcurrentProcesses--;
                    returnlicense(sharedHeap);
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

void parsingArgs(int argc, char* argv[]){
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
                        nValue = MAX_PROC;
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
}

static void myhandler(int SIGNAL) {
    char timeout[] = "timing out processes.\n";
    int timeoutSize = sizeof( timeout );
    int errsave;

    errsave = errno;
    write(STDERR_FILENO, timeout, timeoutSize );
    errno = errsave;
    int i;

    //waiting for max amount of children to terminate
    for (i = 0; i <= nValue ; ++i) {
        wait(NULL);
    }
    //detaching shared memory
    if (detachandremove(shmID, sharedHeap) == -1) {
        perror("failed to destroy shared memory segment\n");
        exit(0);
    } else {
        printf("destroyed shared memory segment\n");
    }
    exit(0);
}

static void myKillSignalHandler(int SIGNAL){
    char timeout[] = "caught ctrl+c, ending processes.\n";
    int timeoutSize = sizeof( timeout );
    int errsave;

    errsave = errno;
    write(STDERR_FILENO, timeout, timeoutSize );
    errno = errsave;
    int i;

    //waiting for max amount of children to terminate
    for (i = 0; i <= nValue ; ++i) {
        wait(NULL);
    }
    //detaching shared memory
    if (detachandremove(shmID, sharedHeap) == -1) {
        perror("failed to destroy shared memory segment\n");
        exit(0);
    } else {
        printf("destroyed shared memory segment\n");
    }
    exit(0);
}

static int setupUserInterrupt( void ){
    struct sigaction act;
    act.sa_handler = myKillSignalHandler;
    act.sa_flags = 0;
    return (sigemptyset(&act.sa_mask) || sigaction(SIGINT, &act, NULL));
}

static int setupinterrupt( void ){
    struct sigaction act;
    act.sa_handler = myhandler;
    act.sa_flags = 0;
    return (sigemptyset(&act.sa_mask) || sigaction(SIGALRM, &act, NULL));
}

static int setupitimer(int TIMER) {
    struct itimerval value;
    value.it_interval.tv_sec = TIMER;
    value.it_interval.tv_usec = 0;
    value.it_value = value.it_interval;
    return (setitimer( ITIMER_REAL, &value, NULL));
}