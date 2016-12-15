#include "syscall.h"

int lock1;
int lock2;
int lock3;
int lock4;
int lock5;
int lock6;


void t5_t2() {
    
    
    Acquire(lock1);
    Acquire(lock2);
    Acquire(lock3);
    Acquire(lock4);
    Acquire(lock5);
    Acquire(lock6);
    Release(lock6);
    Release(lock5);
    Release(lock4);
    Release(lock3);
    Release(lock2);
    Release(lock1);
    
    Exit(0);
}


int main() {
    
    lock1 = CreateLock("0", sizeof("0"));
    lock2 = CreateLock("1", sizeof("1"));
    lock3 = CreateLock("2", sizeof("2"));
    lock4 = CreateLock("3", sizeof("3"));
    lock5 = CreateLock("4", sizeof("4"));
    lock6 = CreateLock("5", sizeof("5"));
    
    Fork(t5_t2);
    Yield();
}