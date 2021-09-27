// license.h

#include <stdio.h>
#include <signal.h>
#include "config.h"

#ifndef LICENSE_H
#define LICENSE_H

int nlicense;                   //This is license object. This is the critical section object

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