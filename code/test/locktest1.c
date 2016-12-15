#include  "syscall.h"
/*
 
 */
int lock;

int main(){
    lock = CreateLock("0", sizeof("0"));
    Acquire(lock);
    Yield();
    Acquire(lock);
    Release(lock);
    Exit(0);
}