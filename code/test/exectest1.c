#include "syscall.h"
/*
 This is a basic test to make sure our Exec can create a new process with the executable file
 'testfiles.' In testfiles, it is a simple program to test the file handling system calls. Also,
 makes a call to yield, and forks a function three times.
*/

int main(){
    Exec("../test/testfiles", sizeof("../test/testfiles"));
}