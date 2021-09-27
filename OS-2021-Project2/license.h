// license.h

#include <stdio.h>
#include <signal.h>
#include "config.h"

#ifndef LICENSE_H
#define LICENSE_H

typedef struct sharedMemory {
    int waitQueue[MAX_PROC];    //This will be the queue for the processes
    sig_atomic_t ready;         //This is the active processes
    int nlicense;               //This is license object. This is the critical section object
} sharedMem;

extern sharedMem *sharedHeap;

// functions
int getlicense(void);           //Blocks until a license is available
int returnlicense(void);        //Increments the number of avail license object
int initlicense(void);          //Performs any needed initialization of the license object
void addtolicenses(int n);      //Adds n licenses to the number available
void removelicenses(int n);     //Decrements the number of licenses by n
void logmsg(const char* msg);   //Write the specified message to the log file.

/* NOTE: There is only one log file. This function will treat the log file as a critical resource.
 * It will open the file to append the message and close the file after appending the message.
 *
 */


#endif