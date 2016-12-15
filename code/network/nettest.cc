// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include <sstream>
#include <string>
#include <iostream>

extern KernelLock* kernelLocks[200];
extern KernelMV* kernelMVs[200];
extern KernelCV* kernelCVs[200];
ServerLock* serverLocks[200];
ServerCV* serverCVs[200];

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}

void NachosServer(){
    int nextLockIndex = 0;
    int nextCVIndex = 0;
    int nextMVIndex = 0;

    printf("In server code..\n");

    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char* ps;
    char name[MaxMailSize];
    char* out = new char[3];
    int addr;
    int machineID;
    int lockIndex;
    int cvIndex;
    int mvIndex;
    int newValue;
    int mvArraySize;
    int arrayIndex;
    bool success;
    string receivedString;
    string syscall;
    string parameter;
    stringstream ss;

    // Server always running 
    while(true){
        printf("Ready for new Syscall...\n");
        ss.clear();//clear any bits set
        ss.str(std::string());
        // Server wait for the first message from the other client machine
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        // printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        outPktHdr.to = inPktHdr.from;
        outMailHdr.to = inMailHdr.from;
        outMailHdr.from = 0; // from Server machine 0
        fflush(stdout);
        receivedString = string(buffer);
        //printf("%s\n", buffer);
        syscall = receivedString.substr(0,2);
        ps = buffer + 2;
        //printf("%s\n", ps);
        printf("outPktHdr.to: %d, outMailHdr.to: %d\n", outPktHdr.to, outMailHdr.to);

        if (syscall == "CL"){
            bool identicalLock = false;
            printf("\tCreateLock syscall\n");
            ss << ps;
            //printf("test: %s\n", kernelLocks[0]->lockName);
            ss >> name >> machineID >> addr;
            printf("name: %s\n", name);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            // for (int i = 0; i < nextLockIndex; i++){
            //     //printf("kernelLocks[%d]:%s\n",i,kernelLocks[0]->lockName);
            //     //printf("kernelLocks[%d]:%s\n",i,name);
            //     if(strcmp(kernelLocks[i]->lockName,name) == 0){
            //         identicalLock = true;
            //         printf("Found identical locks, no need to create new lock\n");
            //         kernelLocks[i]->numRequested++;
            //         sprintf(out,"%d",i);
            //         success = postOffice->Send(outPktHdr,outMailHdr,out);
            //         break;
            //     }
            // }
            // //printf("test: %s\n", kernelLocks[0]->lockName);
            // if (identicalLock == true){
            //     continue;
            // }
            KernelLock* kLock = new KernelLock;
            kLock->lockName = name;
            kLock->lock = new Lock(name);
            kLock->addrSpace = (AddrSpace*) addr;
            kLock->isToBeDeleted = false;
            kLock->numRequested = 1;
            if (nextLockIndex < 200){
                kernelLocks[nextLockIndex] = kLock;
                serverLocks[nextLockIndex] = new ServerLock();
                List* queue = new List;
                serverLocks[nextLockIndex] -> queue = queue;
                sprintf(out, "%d", nextLockIndex);
                outMailHdr.length = strlen(out) + 1;
                printf("outPktHdr: %d, outMailHdr: %d\n", outPktHdr.to, outMailHdr.to);
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
                nextLockIndex++;
            }
            else{
                printf("Exceed Maximum 200 locks, Exit server now.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
                break;
            }
        }
        else if (syscall == "DL"){
            printf("\tDestroyLock syscall\n");
            ss << ps;
            ss >> lockIndex >> machineID >> addr;
            printf("lockIndex: %d\n", lockIndex);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            // printf("lockIndex: %d\n", lockIndex);
            // printf("addr: %d\n", addr);
            // printf("machineID: %d\n", machineID);
            if (lockIndex >= 0 && lockIndex < 200){
                if(kernelLocks[lockIndex]->lock == NULL){
                    printf("Error: Cannot Destroy non existant lock.\n");
                    sprintf(out, "%d", -1);
                }
                else if (kernelLocks[lockIndex] -> lock -> value == false){
                    printf("Error: Lock %d is currently held by another thread, cannot Destroy.\n", lockIndex);
                    sprintf(out, "%d", -1);
                }
                else{
                    kernelLocks[lockIndex] -> numRequested--;
                    if (kernelLocks[lockIndex] -> numRequested == 0){
                        delete kernelLocks[lockIndex]->lock;
                        kernelLocks[lockIndex]->lock = NULL;
                        serverLocks[lockIndex] = NULL;
                    }
                    printf("Successfully destroyed Lock %d\n", lockIndex);
                    sprintf(out, "%d", 1);
                }
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }   
            }
            else{
                printf("Lock Index out of bound, Cannot destroy lock.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
        }
        else if (syscall == "AL"){
            printf("\tAcquireLock syscall\n");
            ss << ps;
            ss >> lockIndex >> machineID >> addr;
            printf("lockIndex: %d\n", lockIndex);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            if (!(lockIndex >= 0 && lockIndex < 200)){
                printf("Error, cannot acquire lock out of bounds.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (kernelLocks[lockIndex] -> lock == NULL){
                printf("Error: cannot acquire a lock that does not exist.\n" );
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else{
                if (serverLocks[lockIndex] -> busy == false ){
                    //kernelLocks[lockIndex] -> lock ->Acquire();
                    serverLocks[lockIndex] -> busy = true;
                    serverLocks[lockIndex] -> owner = addr;  //made a mistake, addr is machine
                    printf("Successfully acquired Lock %d\n", lockIndex);
                    sprintf(out, "%d", 1);
                    outMailHdr.length = strlen(out) + 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, out);
                    if ( !success ) {
                      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                      interrupt->Halt();
                    }
                }
                else{
                    serverLocks[lockIndex] -> queue -> Append((void*)machineID);
                    printf("Lock Busy, put in queue. %d\n", lockIndex);
                }
            }
            
        }
        else if (syscall == "RL"){
            printf("\tReleaseLock syscall\n");
            ss << ps;
            ss >> lockIndex >> machineID >> addr;
            printf("lockIndex: %d\n", lockIndex);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            if (!(lockIndex >= 0 && lockIndex < 200)){
                printf("Error, cannot release lock out of bounds.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (kernelLocks[lockIndex] -> lock == NULL){
                printf("Error: cannot release a lock that does not exist.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (serverLocks[lockIndex] -> busy == false){
                printf("Error: lock is not acquired, cannot release lock.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else{
                //if (serverLocks[lockIndex] -> busy == true ){
                    //kernelLocks[lockIndex] -> lock ->Release();
                    if (!serverLocks[lockIndex] -> queue -> IsEmpty()){
                        int lockOwner = (int)serverLocks[lockIndex] -> queue -> Remove();
                        printf("lockOwner: %d\n", lockOwner);
                        serverLocks[lockIndex] -> owner = lockOwner;
                        printf("Successfully released Lock %d, lock passed to %d\n", lockIndex, lockOwner);
                        sprintf(out, "%d", 1);
                        outMailHdr.length = strlen(out) + 1;
                        success = postOffice->Send(outPktHdr, outMailHdr, out);
                        if ( !success ) {
                          printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                          interrupt->Halt();
                        }

                        outPktHdr.to = lockOwner;
                        sprintf(out, "%d", 1);
                        outMailHdr.length = strlen(out) + 1;
                        success = postOffice->Send(outPktHdr, outMailHdr, out);
                        if ( !success ) {
                          printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                          interrupt->Halt();
                        }
                    }
                    else{
                        serverLocks[lockIndex] -> busy = false;
                        serverLocks[lockIndex] -> owner = NULL;
                        sprintf(out, "%d", 1);
                        outMailHdr.length = strlen(out) + 1;
                        success = postOffice->Send(outPktHdr, outMailHdr, out);
                        if ( !success ) {
                          printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                          interrupt->Halt();
                        }
                    }
                //}
            }
            
        }
        else if (syscall == "CC"){
            bool identicalCV = false;
            printf("\tCreateCV syscall\n");
            ss << ps;
            ss >> name >> machineID >> addr;
            printf("name: %s\n", name);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            // for (int i = 0; i < nextCVIndex; i++){
            //     if(strcmp(kernelCVs[i]->cvName,name) == 0){
            //         identicalCV = true;
            //         printf("Found identical CVs, no need to create new CV\n");
            //         kernelCVs[i]->numRequested++;
            //         sprintf(out,"%d",i);
            //         success = postOffice->Send(outPktHdr,outMailHdr,out);
            //         break;
            //     }
            // }
            // if (identicalCV == true){
            //     continue;
            // }
            KernelCV* kCV = new KernelCV;
            kCV->cvName = name;
            kCV->as = (AddrSpace*) addr;
            kCV->isToBeDeleted = false;
            kCV->numRequested = 1;
            kCV->cv = new Condition(name);
            if (nextCVIndex < 200){
                kernelCVs[nextCVIndex] = kCV;
                serverCVs[nextCVIndex] = new ServerCV();
                List* cvQueue = new List;
                serverCVs[nextCVIndex] -> queue = cvQueue;
                sprintf(out, "%d", nextCVIndex);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
                nextCVIndex++;
            }
            else{
                printf("Exceed Maximum 200 CVs, Exit server now.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
                break;
            }
        }
        else if (syscall == "DC"){
            printf("\tDestroyCV syscall\n");
            ss << ps;
            ss >> cvIndex >> machineID >> addr;
            printf("cvIndex: %d\n", cvIndex);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            if (cvIndex >= 0 && cvIndex < 200){
                if(kernelCVs[cvIndex]->cv == NULL){
                    printf("Error: Cannot Destroy non existant CV.\n");
                    sprintf(out, "%d", -1);
                }
                else if (kernelCVs[cvIndex] -> cv -> getWaitingLock() != NULL){
                    printf("Error: CV %d is currently held by another thread, cannot Destroy.\n", lockIndex);
                    sprintf(out, "%d", -1);
                }
                else{
                    kernelCVs[cvIndex] -> numRequested--;
                    if (kernelCVs[cvIndex] -> numRequested == 0){
                        delete kernelCVs[cvIndex]->cv;
                        kernelCVs[cvIndex]->cv = NULL;
                        serverCVs[cvIndex] = NULL;
                    }
                    printf("Successfully destroyed CV %d\n", cvIndex);
                    sprintf(out, "%d", 1);
                }
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }   
            }
            else{
                printf("CV Index out of bound, Cannot destroy CV.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
        }
        else if (syscall == "SC"){
            printf("\tSignalCV syscall\n");
            ss << ps;
            ss >> cvIndex >> lockIndex >> machineID >> addr;
            printf("cvIndex: %d\n", cvIndex);
            printf("lockIndex: %d\n", lockIndex);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            if (!(cvIndex >= 0 && cvIndex < 200)){
                printf("Error, cannot signal CV out of bounds.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (!(lockIndex >= 0 && lockIndex < 200)){
                printf("Error, cannot signal Lock out of bounds.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (kernelCVs[cvIndex]->cv == NULL){
                printf("Error, cannot signal CV that doesn't exist.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (kernelLocks[lockIndex]->lock == NULL){
                printf("Error, cannot signal Lock that doesn't exist.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            // else if (serverCVs[cvIndex]->inWait == false){
            //     printf("Error: no waiting CVs\n");
            //     sprintf(out, "%d", -1);
            // }
            else if (lockIndex != serverCVs[cvIndex]->lockIndex){
                printf("Error: lockIndex does not match\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else{
                // KernelLock* cvLock = kernelLocks[lockIndex];
                // kernelCVs[cvIndex]->cv->Signal(cvLock->lock);
                printf("Signalling CV %d on lock %d\n", cvIndex, lockIndex);
                sprintf(out, "%d", 1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
                machineID = (int)serverCVs[cvIndex] -> queue -> Remove();

                if (serverLocks[lockIndex] -> busy == false ){
                    //kernelLocks[lockIndex] -> lock ->Acquire();
                    serverLocks[lockIndex] -> busy = true;
                    serverLocks[lockIndex] -> owner = machineID;
                    printf("Successfully acquired Lock %d\n", lockIndex);
                    outPktHdr.to = machineID;
                    sprintf(out, "%d", 1);
                    outMailHdr.length = strlen(out) + 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, out);
                    //printf("hello\n");
                    if ( !success ) {
                      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                      interrupt->Halt();
                    }
                }
                else{
                    serverLocks[lockIndex] -> queue -> Append((void*)machineID);
                    printf("Lock Busy, put in queue. lock %d\n", lockIndex);
                }

            }
        }
        else if (syscall == "WC"){
            printf("\tWaitCV syscall\n");
            ss << ps;
            ss >> cvIndex >> lockIndex >> machineID >> addr;
            printf("cvIndex: %d\n", cvIndex);
            printf("lockIndex: %d\n", lockIndex);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            if (!(cvIndex >= 0 && cvIndex < 200)){
                printf("Error, cannot Wait CV out of bounds.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (!(lockIndex >= 0 && lockIndex < 200)){
                printf("Error, cannot Wait Lock out of bounds.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (kernelCVs[cvIndex]->cv == NULL){
                printf("Error, cannot Wait CV that doesn't exist.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (kernelLocks[lockIndex]->lock == NULL){
                printf("Error, cannot Wait Lock that doesn't exist.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else{
                // printf("hello\n");
                // KernelLock* cvLock = kernelLocks[lockIndex];
                // kernelCVs[cvIndex]->cv->Wait(cvLock->lock);
                printf("Waiting CV %d on lock %d\n", cvIndex, lockIndex);
                serverCVs[cvIndex] -> queue -> Append((void*)machineID);
                // sprintf(out, "%d", 1);
                serverCVs[cvIndex] -> lockIndex = lockIndex;
                if (!serverLocks[lockIndex] -> queue -> IsEmpty()){
                    int lockOwner = (int)serverLocks[lockIndex] -> queue -> Remove();
                    serverLocks[lockIndex] -> owner = lockOwner;
                    printf("Successfully released Lock %d, lock passed to %d\n", lockIndex, lockOwner);
                }
                else{
                    serverLocks[lockIndex] -> busy = false;
                    serverLocks[lockIndex] -> owner = NULL;
                }
            }
        }
        else if (syscall == "BC"){
            printf("\tBroadcastCV syscall\n");
            ss << ps;
            ss >> cvIndex >> lockIndex >> machineID >> addr;
            printf("cvIndex: %d\n", cvIndex);
            printf("lockIndex: %d\n", lockIndex);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            if (!(cvIndex >= 0 && cvIndex < 200)){
                printf("Error, cannot Broadcast CV out of bounds.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (!(lockIndex >= 0 && lockIndex < 200)){
                printf("Error, cannot Broadcast Lock out of bounds.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (kernelCVs[cvIndex]->cv == NULL){
                printf("Error, cannot Broadcast CV that doesn't exist.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else if (kernelLocks[lockIndex]->lock == NULL){
                printf("Error, cannot Broadcast Lock that doesn't exist.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
            else{
                //kernelCVs[cvIndex]->cv->Broadcast(kernelLocks[lockIndex]->lock);
                
                printf("Broadcasting CV %d on lock %d\n", cvIndex, lockIndex);
                sprintf(out, "%d", 1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
                while(!serverCVs[cvIndex] -> queue -> IsEmpty()){
                    machineID = (int)serverCVs[cvIndex] -> queue -> Remove();
                    if (serverLocks[lockIndex] -> busy == false ){
                        //kernelLocks[lockIndex] -> lock ->Acquire();
                        serverLocks[lockIndex] -> busy = true;
                        serverLocks[lockIndex] -> owner = machineID;
                        printf("Successfully acquired Lock %d\n", lockIndex);
                        outPktHdr.to = machineID;
                        sprintf(out, "%d", 1);
                        outMailHdr.length = strlen(out) + 1;
                        success = postOffice->Send(outPktHdr, outMailHdr, out);
                        //printf("hello\n");
                        if ( !success ) {
                          printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                          interrupt->Halt();
                        }
                    }
                    else{
                        serverLocks[lockIndex] -> queue -> Append((void*)machineID);
                        printf("Lock Busy, put in queue. lock %d\n", lockIndex);
                    }
                }

            }
            
        }
        else if (syscall == "CM"){
            bool identicalMV = false;
            printf("\tCreateMV syscall\n");
            ss << ps;
            ss >> name >> mvArraySize >> machineID >> addr;
            printf("name: %s\n", name);
            printf("mvArraySize: %d\n", mvArraySize);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            // for (int i = 0; i < nextMVIndex; i++){
            //     if(strcmp(kernelMVs[i]->mvName,name) == 0){
            //         identicalMV = true;
            //         printf("Found identical MVs, no need to create new MV\n");
            //         kernelMVs[i]->numRequested++;
            //         sprintf(out,"%d",i);
            //         success = postOffice->Send(outPktHdr,outMailHdr,out);
            //         break;
            //     }
            // }
            // if (identicalMV == true){
            //     continue;
            // }
            KernelMV* kMV = new KernelMV;
            kMV->mvName = name;
            kMV->size = mvArraySize;
            kMV->array = new int[mvArraySize];
            kMV->as = (AddrSpace*) addr;
            kMV->isToBeDeleted = false;
            kMV->numRequested = 1;
            if (nextMVIndex < 200){
                kernelMVs[nextMVIndex] = kMV;
                sprintf(out, "%d", nextMVIndex);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
                nextMVIndex++;
            }
            else{
                printf("Exceed Maximum 200 MVs, Exit server now.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
                
            }
        }
        else if (syscall == "DM"){
            printf("\tDestroyMV syscall\n");
            ss << ps;
            ss >> mvIndex >> machineID >> addr;
            printf("mvIndex: %d\n", mvIndex);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            if (mvIndex >= 0 && mvIndex < 200){
                if(kernelMVs[mvIndex] == NULL || kernelMVs[mvIndex]->array == NULL ){
                    printf("Error: Cannot Destroy non existant MV.\n");
                    sprintf(out, "%d", -1);
                }
                else{
                    kernelMVs[mvIndex] -> numRequested--;
                    if (kernelMVs[mvIndex] -> numRequested == 0){
                        delete kernelMVs[mvIndex];
                        kernelMVs[mvIndex] = NULL;
                    }
                    printf("Successfully destroyed MV %d\n", mvIndex);
                    sprintf(out, "%d", 1);
                }
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }   
            }
            else{
                printf("MV Index out of bound, Cannot destroy MV.\n");
                sprintf(out, "%d", -1);
                outMailHdr.length = strlen(out) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, out);
                if ( !success ) {
                  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                  interrupt->Halt();
                }
            }
        }
        else if (syscall == "GM"){
            printf("\tGetMV syscall\n");
            ss << ps;
            ss >> mvIndex >> arrayIndex >> machineID >> addr;
            printf("mvIndex: %d\n", mvIndex);
            printf("arrayIndex: %d\n", arrayIndex);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            if (!(mvIndex >= 0 && mvIndex < 200)){
                printf("Error, cannot Get MV out of bounds.\n");
                sprintf(out, "%d", -1);
            }
            else if (kernelMVs[mvIndex]->array == NULL){
                printf("Error, cannot Get MV that does not exist.\n");
                sprintf(out, "%d", -1);
            }
            else if (kernelMVs[mvIndex]->size <= arrayIndex){
                printf("Error, cannot Get MV null pointer.\n");
                sprintf(out, "%d", -1);
            }
            else{
                sprintf(out, "%d", kernelMVs[mvIndex]->array[arrayIndex]);
                outMailHdr.length = strlen(out) + 1;
            }
            success = postOffice->Send(outPktHdr, outMailHdr, out);
            if ( !success ) {
              printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
              interrupt->Halt();
            }

        }
        else if (syscall == "SM"){
            printf("\tSetMV syscall\n");
            ss << ps;
            ss >> mvIndex >> arrayIndex >> newValue >> machineID >> addr;
            printf("mvIndex: %d\n", mvIndex);
            printf("arrayIndex: %d\n", arrayIndex);
            printf("newValue: %d\n", newValue);
            printf("addr: %d\n", addr);
            printf("machineID: %d\n", machineID);
            if (!(mvIndex >= 0 && mvIndex < 200)){
                printf("Error, cannot Set MV out of bounds.\n");
                sprintf(out, "%d", -1);
            }
            else if (kernelMVs[mvIndex]->array == NULL){
                printf("Error, cannot Set MV that does not exist.\n");
                sprintf(out, "%d", -1);
            }
            else if (kernelMVs[mvIndex]->size <= arrayIndex){
                printf("Error, cannot Set MV null pointer.\n");
                sprintf(out, "%d", -1);
            }
            else{
                kernelMVs[mvIndex]->array[arrayIndex] = newValue;
                sprintf(out, "%d", 1);
                outMailHdr.length = strlen(out) + 1;  
            }
            success = postOffice->Send(outPktHdr, outMailHdr, out);
            if ( !success ) {
              printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
              interrupt->Halt();
            }
        }
        else{
            printf("Received syscall not found~\n");
        }
        ss.clear();//clear any bits set
        ss.str(std::string());
    }
    // Then we're done!
    interrupt->Halt();
}
