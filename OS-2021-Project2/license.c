//license.c

#include "license.h"
#include <stdlib.h>

int getlicense(void){
    //This function will detect the number of licenses available.
    //If there are no available licenses, it will block any processes from taking a license.
    if( nlicense = 0 ){
        return 1;
    } else {
        //we have licenses now.
        return 0;
    }

}

int returnlicense(void){
    nlicense++;
}

int initlicense(void){


    return 0;
}

void addtolicenses(int n){
    nlicense += n;
    printf("added %d licenses, current nlicense: %d\n", n, nlicense);
}

void removelicenses(int n){
    nlicense -= n;
    printf("subtracted %d licenses, current nlicense: %d\n", n, nlicense);
}

void logmsg(const char* msg){

}