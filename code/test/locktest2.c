#include "syscall.h"
int lock;

void t1_t1() {
    lock = CreateLock("0", sizeof("0"));
    Acquire(lock);
    Yield();
    Exit(0);
}

void t1_t2() {
    Yield();
    Release(lock);
    Exit(0);
}

int main(){
    lock = CreateLock("0", sizeof("0"));
    Fork(t1_t1);
    Fork(t1_t2);
}