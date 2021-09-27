//license.c

#include "license.h"

int getlicense(void){
    //This function will detect the number of licenses available.
    //If there are no available licenses, it will block any processes from taking a license.

}

int returnlicense(void){
    sharedHeap->nlicense++;
}

int initlicense(void){
    //just initializing sharedMemory object
    sharedHeap->nlicense = 0;
    return 0;
}

void addtolicenses(int n){
    sharedHeap->nlicense += n;
    printf("added %d licenses, current nlicense: %d\n", n, sharedHeap->nlicense);
}

void removelicenses(int n){
    sharedHeap->nlicense -= n;
    printf("subtracted %d licenses, current nlicense: %d\n", n, sharedHeap->nlicense);
}

void logmsg(const char* msg){

}