//license.c

#include "license.h"


int getlicense(sharedMem* sharedHeap){
    //This function will detect the number of licenses available.
    //If there are no available licenses, it will block any processes from taking a license.
    if( sharedHeap->nlicense == 0 ){
        return 1;
    } else {
        //we have licenses now.
        sharedHeap->nlicense--;
        printf("we now have %d number of licenses\n", sharedHeap->nlicense);
        return 0;
    }

}

int returnlicense(sharedMem* sharedHeap){
    sharedHeap->nlicense++;
    printf("current available license: %d\n", sharedHeap->nlicense);
}

int initlicense(sharedMem *sharedHeap){
    sharedHeap->nlicense = 0;
    return 0;
}

void addtolicenses(sharedMem *sharedHeap, int n){
    sharedHeap->nlicense += n;
    printf("added %d licenses, current nlicense: %d\n", n, sharedHeap->nlicense);
}

void removelicenses(sharedMem* sharedHeap,int n){
    sharedHeap->nlicense -= n;
    printf("subtracted %d licenses, current nlicense: %d\n", n, sharedHeap->nlicense);
}

void logmsg(sharedMem *sharedHeap,const char* msg){

}