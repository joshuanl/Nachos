#include "syscall.h"
/*
 Proves that Exec works by executing a file that has a fork forking a thread.
 */
int main(){
    Exec("../test/forktest2", sizeof("../test/forktest2"));
}