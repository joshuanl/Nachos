#include "syscall.h"

int lock;
int cv;

void cv_waiter() {
    Acquire(lock);
    Write("Waiter: waiting.\n", sizeof("Waiter: waiting.\n"), ConsoleOutput);
    Wait(cv,lock);
    Write("Waiter: signalled.\n", sizeof("Waiter: signalled.\n"), ConsoleOutput);
    Release(lock);
    Exit(0);
}
void cv_signaller() {
    
    Acquire(lock);
    Write("Signaller: pre-signal\n", sizeof("Signaller: pre-signal\n"), ConsoleOutput);
    Signal(cv,lock);
    Write("Signaller: post-signal\n", sizeof("Signaller: post-signal\n"), ConsoleOutput);
    Release(lock);
    Exit(0);
}

int main(){
    int i;
    Write("CV test", sizeof("CV test"), ConsoleOutput);
    lock = CreateLock("lock", sizeof("lock"));
    cv = CreateCondition("cv", sizeof("cv"));
    
    Fork(cv_waiter);
    Fork(cv_waiter);
    Fork(cv_waiter);
    
    Fork(cv_signaller);
    Yield();
    
    Exit(0);
    
}