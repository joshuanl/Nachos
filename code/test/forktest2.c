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
    Exit(0);
}

void t5_t3() {
    
    
    Acquire(lock4);
    Acquire(lock5);
    Acquire(lock6);
    Exit(0);
}


int main() {
    
    lock1 = CreateLock("0", sizeof("0"));
    lock2 = CreateLock("0", sizeof("0"));
    lock3 = CreateLock("0", sizeof("0"));
    lock4 = CreateLock("0", sizeof("0"));
    lock5 = CreateLock("0", sizeof("0"));
    lock6 = CreateLock("0", sizeof("0"));
    
    Fork(t5_t2);
    Fork(t5_t3);
    Yield();
}

