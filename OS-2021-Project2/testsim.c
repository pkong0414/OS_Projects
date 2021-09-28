//testsim.c

#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "license.h"
#define PERM (IPC_CREAT | S_IRUSR | S_IWUSR)

// FUNCTION PROTOTYPES
void parsingArgs(int argc, char* argv[]);                        //Function for parsing arguments
void initShm(key_t myKey);                                      //Function to init shared memory

// SIGNAL HANDLER PROTOTYPES
static void myKillSignalHandler(int SIGNAL);
static int setupUserInterrupt( void );

int opt, sleepValue, repeatFactor;                              //globals for opt parameters
key_t myKey;                                                    //Shared memory key
int shmID;                                                      //shared memoryID
sharedMem *sharedHeap;                                          //shared memory object

/*  THINGS TO DO
 *  We need to write the way we load up the message in the manner is required.
 *
 */


int main(int argc, char* argv){

    if( setupUserInterrupt() == -1 ){
        perror( "failed to set up a user kill signal.\n");
        return 1;
    }
    parsingArgs(argc, argv);
    initShm(myKey);


    return 0;
}

void parsingArgs(int argc, char* argv[]){
    while((opt = getopt(argc, argv, "s:r:")) != -1) {
        switch (opt) {
            case 's':
                if (!isdigit(argv[2][0])) {
                    //This case the user uses -t parameter but entered a string instead of an int.
                    printf("value entered: %s\n", argv[2]);
                    printf("%s: ERROR: -s <seconds for sleep>\n", argv[0]);
                    exit(EXIT_FAILURE);
                } else {
                    // -n gives us the number of licenses available
                    sleepValue = atoi(optarg);
                    // we will check to make sure nValue is 1 to 20.
                    printf("sleepValue: %d\n", sleepValue);
                    break;
                }
            case 'r':
                if (!isdigit(argv[3][0])) {
                    //This case the user uses -t parameter but entered a string instead of an int.
                    printf("value entered: %s\n", argv[2]);
                    printf("%s: ERROR: -r <number of repeats>\n", argv[0]);
                    exit(EXIT_FAILURE);
                } else {
                    // -n gives us the number of licenses available
                    repeatFactor = atoi(optarg);
                    // we will check to make sure nValue is 1 to 20.
                    printf("repeatFactor: %d\n", repeatFactor);
                    break;
                }
            default: /* '?' */
                printf("%s: ERROR: parameter not recognized.\n", argv[0]);
                fprintf(stderr, "Usage: %s [-s seconds for sleep] [-r number of repeats]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    } /* END OF GETOPT */
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

static void myKillSignalHandler(int SIGNAL){
    char timeout[] = "caught ctrl+c, ending processes.\n";
    int timeoutSize = sizeof( timeout );
    int errsave;

    errsave = errno;
    write(STDERR_FILENO, timeout, timeoutSize );
    errno = errsave;

    exit(EXIT_SUCCESS);
}

static int setupUserInterrupt( void ){
    struct sigaction act;
    act.sa_handler = myKillSignalHandler;
    act.sa_flags = 0;
    return (sigemptyset(&act.sa_mask) || sigaction(SIGINT, &act, NULL));
}
