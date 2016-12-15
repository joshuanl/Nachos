// synch.cc
//  Routines for synchronizing threads.  Three kinds of
//  synchronization routines are defined here: semaphores, locks
//      and condition variables (the implementation of the last two
//  are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  Initialize a semaphore, so that it can be used for synchronization.
//
//  "debugName" is an arbitrary name, useful for debugging.
//  "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  De-allocate semaphore, when no longer needed.  Assume no one
//  is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//  Wait until semaphore value > 0, then decrement.  Checking the
//  value and decrementing must be done atomically, so we
//  need to disable interrupts before checking the value.
//
//  Note that Thread::Sleep assumes that interrupts are disabled
//  when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    
    while (value == 0) {            // semaphore not available
        queue->Append((void *)currentThread);   // so go to sleep
        currentThread->Sleep();
    }
    value--;                    // semaphore available,
    // consume its value
    
    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//  Increment semaphore value, waking up a waiter if necessary.
//  As with P(), this operation must be atomic, so we need to disable
//  interrupts.  Scheduler::ReadyToRun() assumes that threads
//  are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    
    thread = (Thread *)queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
    value = true;
    name = debugName;
    lockWaitQueue = new List;
    lockOwner = NULL;
}

Lock::~Lock() {
    delete lockWaitQueue;
}

void Lock::Acquire() {
    //disable interrupts
    IntStatus old = interrupt->SetLevel(IntOff);
    //current thread is lock owner
    if(currentThread == lockOwner){
        //restore interrupts
        printf("\tCurrent Thread is already Lock owner!\n");
        interrupt->SetLevel(old);
        return;
    }
    //lock is available & current thread will become lock owner
    if(value == true){
        //I can have it-- make state busy
        value = false;
        //make myself the owner
        lockOwner = currentThread;
        
    }else{
        //lock is busy
        //put current thread on lock's wait q
        lockWaitQueue->Append((void *)currentThread);
        printf("\tLock is Acquired by someone already!\n");
        currentThread->Sleep();   // so go to sleep
        
    }
    
    //restore interrupts
    interrupt->SetLevel(old);
    
}
//need help with second if statement & wait queue
void Lock::Release() {
    //disable interrupts
    IntStatus old = interrupt->SetLevel(IntOff);
    //current thread is not lock owner
    if(currentThread != lockOwner){
        printf("You are not the owner of this lock!\n");
        //restore interrupts
        interrupt->SetLevel(old);
        return;
    }
    if(!lockWaitQueue->IsEmpty()){
        lockOwner = (Thread *)lockWaitQueue->Remove();
        scheduler->ReadyToRun(lockOwner);
    }else{
        //make lock available
        value = true;
        //unset lock owner
        lockOwner = NULL;
    }
    //restore interrupts
    interrupt->SetLevel(old);
}

bool Lock::isHeldByCurrentThread(){
    if(currentThread == lockOwner){
        return true;
    }else{
        return false;
    }
}

Condition::Condition(char* debugName) {
    name = debugName;
    waitingLock = NULL;
    cvWaitQueue = new List;
}

Condition::~Condition() {
    delete waitingLock;
    delete cvWaitQueue;
}

void Condition::Wait(Lock* conditionLock) {
    //disable interrupts
    IntStatus old = interrupt->SetLevel(IntOff);
    
    //no one has called wait yet
    //need to verify a bad lock isnt passed in
    if(conditionLock == NULL)
    {
        printf("Your lock is not valid!\n");
        //restore interrupts
        interrupt->SetLevel(old);
        return;
        
    }
    //first thread that has called wait()
    if(waitingLock == NULL){
        //no one waiting
        waitingLock = conditionLock;
    }
    if(waitingLock != conditionLock){
        printf("This is not the same lock!\n");
        //restore interrupts
        interrupt->SetLevel(old);
        return;
    }
    //okay to wait
    cvWaitQueue->Append((void *)currentThread);
    conditionLock->Release();
    currentThread->Sleep();
    conditionLock->Acquire();
    //restore interrupts
    interrupt->SetLevel(old);
}
void Condition::Signal(Lock* conditionLock) {
    //disable interrupts
    IntStatus old = interrupt->SetLevel(IntOff);
    //no thread waiting
    if(cvWaitQueue->IsEmpty()){
        printf("No thread is waiting for CV in Queue\n");
        //restore interrupts
        interrupt->SetLevel(old);
        return;
    }
    if(waitingLock != conditionLock){
        printf("This is not the same lock!\n");
        //restore interrupts
        interrupt->SetLevel(old);
        return;
        
    }
    //wake up 1 waiting thread
    Thread *thread;
    thread = (Thread *)cvWaitQueue->Remove();
    scheduler->ReadyToRun(thread);
    if(cvWaitQueue->IsEmpty()){
        waitingLock = NULL;
    }
    //restore interrupts
    interrupt->SetLevel(old);
}
void Condition::Broadcast(Lock* conditionLock) {
    //disable interrupts
    IntStatus old = interrupt->SetLevel(IntOff);
    if(conditionLock == NULL){
        printf("Your lock is not valid!\n");
        //restore interrupts
        interrupt->SetLevel(old);
        return;
    }
    if(conditionLock != waitingLock)
    {
        printf("This is not the same lock!\n");
        //restore interrupts
        interrupt->SetLevel(old);
        return;
    }
    //restore interrupts
    interrupt->SetLevel(old);
    
    while(!cvWaitQueue->IsEmpty()){
        Signal(conditionLock);
    }
    
}

Lock* Condition::getWaitingLock() {
	return waitingLock;
}

