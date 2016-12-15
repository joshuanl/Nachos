// exception.cc 
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.  
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "synch.h"
#include "addrspace.h"
#include "network.h"
#include "../network/post.h"
//#include "interrupt.h"
#include <string>


using namespace std;

extern KernelLock* kernelLocks[];
extern KernelCV* kernelCVs[];
extern SingleProcess processTable[20];
extern int processTableCount;
extern Lock* processTableLock;
extern KernelMV* MVTable[200];
int nLockIndex = 0;
int num_MVs = 0;
int num_CVs = 0;
Lock* editCVsLock = new Lock("editCVsLock");
Lock* tlbLock = new Lock("Tlb Lock");
Lock* mvLock = new Lock("MV Lock");


int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;            // The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
      {
            result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
      } 
      
      buf[n++] = *paddr;
     
      if ( !result ) {
    //translation failed
    return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;            // The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
    //translation failed
    return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];    // Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
    printf("\t%s","Bad pointer passed to Create\n");
    delete buf;
    return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];    // Kernel buffer to put the name in
    OpenFile *f;            // The new open file
    int id;             // The openfile id

    if (!buf) {
    printf("\t%s","Can't allocate kernel buffer in Open\n");
    return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
    printf("\t%s","Bad pointer passed to Open\n");
    delete[] buf;
    return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
    if ((id = currentThread->space->fileTable.Put(f)) == -1 )
        delete f;
    return id;
    }
    else
    return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;      // Kernel buffer for output
    OpenFile *f;    // Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
    printf("\t%s","Error allocating kernel buffer for write!\n");
    return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
        printf("\t%s","Bad pointer passed to to write: data not written\n");
        delete[] buf;
        return;
    }
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
    printf("%c",buf[ii]);
      }

    } else {
    if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
        f->Write(buf, len);
    } else {
        printf("\t%s","Bad OpenFileId passed to Write\n");
        len = -1;
    }
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;      // Kernel buffer for input
    OpenFile *f;    // Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
    printf("\t%s","Error allocating kernel buffer in Read\n");
    return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
    printf("\t%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
    if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
        len = f->Read(buf, len);
        if ( len > 0 ) {
            //Read something from the file. Put into user's address space
            if ( copyout(vaddr, len, buf) == -1 ) {
            printf("\t%s","Bad pointer passed to Read: data not copied\n");
        }
        }
    } else {
        printf("\t%s","Bad OpenFileId passed to Read\n");
        len = -1;
    }
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("\t%s","Tried to close an unopen file\n");
    }
}

void Yield_Syscall() {
    currentThread->Yield();
   // printf("\tin yield syscall\n");
    //in test directory, put yield call testfile.c on top of main
    //gmake in test, cd to userprog, do a gmake to compile nachos, nachos
}
//returns int
int CreateLock_Syscall(unsigned int vaddr, int len) {
    printf("In CreateLock syscall\n");
    // Networking code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    //printf("machine id:%d\n", postOffice->getMachineID());
    char *buf = new char[len+1];    // Kernel buffer to put the name in
    if(vaddr == NULL) {
        printf("\tCheck vaddr parameter in CreateLock_Syscall");
        return -1;
    }
    
    if((vaddr < 0) || (vaddr > ((currentThread -> space -> numPages) * PageSize))) {
        printf("\tCheck if vaddr parameter is within bounds in CreateLock_Syscall");
        return -1;
    }
    
    if(copyin(vaddr, len, buf) == -1) {
        printf("\tCheck pointer passed into CreateLock_Syscall");
        delete buf;
        return -1;
    }
    
    if(!buf) {
        printf("\tCheck len parameter in CreateLock_Syscall");
        return -1;
    }
    // buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "CreateLock";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "CL" << buf << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending CL message\n");
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    printf("waiting for server\n");
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int lockIndex = atoi(buffer);
    if (lockIndex >= 0){
        nLockIndex++;
    }
    printf("Lock %d Created\n", lockIndex);
    // Done Networking Code

    return lockIndex;

}


void Acquire_Syscall(int id){
    printf("In Acquire syscall\n");
    // Networking code
    if (id < 0 || id > 200){
        printf("\tAcquire invalid lock index, out of bound...\n");
        return;
    }
    // buf[len]='\0';
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "AcquireLock";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "AL" << id  <<  " " << outPktHdr.from <<" "<< outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending AL message: Lock %d\n", id);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int acquireSuccess = atoi(buffer);
    if (acquireSuccess == -1){
        printf("Lock %d Acquired UNSUCCESSFUL.\n", id);
        return;
    }
    if (acquireSuccess == 0){
        printf("Lock %d BUSY.\n", id);
        return;
    }
    printf("Lock %d Acquired\n", id);

    // Done Networking Code
}

void Release_Syscall(int id){
    printf("In Release syscall\n");
    // Networking code
    if (id < 0 || id > 200){
        printf("\tReleaseing a lock that is out of bound...\n");
        return;
    }
    // buf[len]='\0';
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "ReleaseLock";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "RL" << id <<" "<< outPktHdr.from <<" "<< outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending RL message: Lock %d\n", id);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int releaseSuccess = atoi(buffer);
    if (releaseSuccess != 1){
        printf("Lock %d Release UNSUCCESSFUL.\n", id);
        return;
    }
    printf("Lock %d Released\n", id);
    // Done Networking Code
}

void DestroyLock_Syscall(int id){
    printf("In DestroyLock syscall\n");
    // Networking code
    if(id >= nLockIndex || id < 0) {
        printf("\tCheck if lock index exists – cannot destroy lock out of bounds, id:%d, nLockIndex:%d\n", id, nLockIndex);
        return;
    }
    //buf[len]='\0';
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "DestroyLock";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "DL" << id << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending DL message: Lock %d\n", id);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int deleteSuccess = atoi(buffer);
    if (deleteSuccess){
        printf("Lock %d Destroyed\n", id);
    }
    else{
        printf("Lock %d Destroyed: UNSUCCESSFUL!\n", id);
    }

    // Done Networking Code
}

//Condition variable syscalls
int CreateCondition_Syscall(unsigned int vaddr, int len) {
    printf("In CreateCondition syscall\n");
    // Networking code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    char *buf = new char[len+1];    // Kernel buffer to put the name in
    if(vaddr == NULL) {
     printf("\tCheck vaddr parameter in CreateCondition_Syscall\n");
     return -1;
    }
    if((vaddr < 0) || (vaddr > ((currentThread -> space -> numPages) * PageSize))) {
     printf("\tCheck if vaddr parameter is within bounds in CreateCondition_Syscall\n");
     return -1;
    }
    if(copyin(vaddr, len, buf) == -1) {
        printf("\tCheck pointer passed into CreateCondition_Syscall");
        delete buf;
        return -1;
    }
    
    if(!buf) {
        printf("\tCheck len parameter in CreateCondition_Syscall");
        return -1;
    }
    //buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "CreateCondition";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "CC" << buf << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending CC message\n");
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int cvIndex = atoi(buffer);
    if (cvIndex == -1){
        printf("Error: Cannot create CV\n");
        return -1;
    }
    num_CVs++;
    printf("CV %d Created\n", cvIndex);
    // Done Networking Code
    return cvIndex;
}

void DestroyCondition_Syscall(int id) { 
    printf("In DestroyCondition syscall\n");    
    // Networking code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    if(id > num_CVs || id < 0) {
        printf("\tCheck if lock index exists – cannot destroy Condition out of bounds");
        return;
    }
    //buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "DestroyCondition";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "DC" << id << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending DC message: Cond: %d\n", id);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int deleteSuccess = atoi(buffer);
    if (deleteSuccess == 1){
        printf("CV %d Destroyed\n", id);
    }
    else{
        printf("CV %d Destroyed: UNSUCCESSFUL!\n", id);
    }

    // Done Networking Code
}

void Wait_Syscall(int cvIndex, int lockIndex) { //TO DO: change Lock type to the more protected KernelLock
    printf("In Wait syscall\n");
    // Networking code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    if (cvIndex < 0 || cvIndex >= 200){
        printf("\tError: Waiting on a CV out of bound...\n");
        return;
    }
    if (lockIndex < 0 || lockIndex>= 200){
        printf("\tError: Incorrect associated lock...\n");
        return;
    }
    // buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "WaitCondition";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "WC" << cvIndex << " " << lockIndex << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending WC message: Cond: %d Lock: %d\n", cvIndex, lockIndex);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    printf("Waiting for signal\n");
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int waitSuccess = atoi(buffer);
    if (waitSuccess == 1){
        printf("CV %d Wait\n", cvIndex);
    }
    else{
        printf("CV %d Wait: UNSUCCESSFUL!\n", cvIndex);
    }

    // Done Networking Code
}

void Signal_Syscall(int cvIndex, int lockIndex) {   //TO DO: change Lock type to the more protected KernelLock
    printf("In Signal syscall\n");
    // Networking code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    if (cvIndex < 0 || cvIndex >= 200){
        printf("\tError: Signalling on a CV out of bound...\n");
        return;
    }
    if (lockIndex < 0 || lockIndex>= 200){
        printf("\tError: Incorrect associated lock...\n");
        return;
    }
    // buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "SignalCondition";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "SC" << cvIndex << " " << lockIndex << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending SC message: Cond: %d Lock: %d\n", cvIndex, lockIndex);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int signalSuccess = atoi(buffer);
    if (signalSuccess == 1){
        printf("CV %d Signal\n", cvIndex);
    }
    else{
        printf("CV %d Signal: UNSUCCESSFUL!\n", cvIndex);
    }

    // Done Networking Code
}

void Broadcast_Syscall(int cvIndex, int lockIndex) {    //TO DO: change Lock type to the more protected KernelLock
    printf("In Broadcast syscall\n");
    // Networking code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    if (cvIndex < 0 || cvIndex >= 200){
        printf("\tError: Broadcasting on a CV out of bound...\n");
        return;
    }
    if (lockIndex < 0 || lockIndex>= 200){
        printf("\tError: Incorrect associated lock...\n");
        return;
    }
    // buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "BroadcastCondition";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "BC" << cvIndex << " " << lockIndex << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending BC message: Cond: %d Lock: %d\n", cvIndex, lockIndex);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int broadcastSuccess = atoi(buffer);
    if (broadcastSuccess == 1){
        printf("CV %d Broadcasted\n", cvIndex);
    }
    else{
        printf("CV %d Broadcasted: UNSUCCESSFUL!\n", cvIndex);
    }

    // Done Networking Code
}

int CreateMV_Syscall(unsigned int vaddr, int len, int arraySize){
    printf("In create MV syscall\n");
    // Network Code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    char *buf = new char[len+1];    // Kernel buffer to put the name in
    if(vaddr == NULL) {
        printf("\tCheck vaddr parameter in CreateMV_Syscall\n");
        return -1;
    }
    if((vaddr < 0) || (vaddr > ((currentThread -> space -> numPages) * PageSize))) {
        printf("\tCheck if vaddr parameter is within bounds in CreateMV_Syscall\n");
        return -1;
    }
    if(copyin(vaddr, len, buf) == -1) {
        printf("\tCheck pointer passed into CreateMV_Syscall");
        delete buf;
        return -1;
    }
    
    if(!buf) {
        printf("\tCheck len parameter in CreateMV_Syscall");
        return -1;
    }
    // buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "CreateMV";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "CM" << buf << " " << arraySize << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending CM message\n");
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int mvIndex = atoi(buffer);
    if (mvIndex == -1){
        printf("Error: Cannot create CV\n");
        return -1;
    }
    num_MVs++;
    printf("MV %d Created\n", mvIndex);
    // Done Networking Code
    return mvIndex;
}
void DestroyMV_Syscall(int mvIndex){
    printf("In Destroy MV syscall\n");
    // Network Code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    if(mvIndex > num_MVs || mvIndex < 0) {
        printf("\tCheck if MV index exists – cannot destroy Condition out of bounds");
        return;
    }
    // buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "DestroyMV";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "DM" << mvIndex << " " << outPktHdr.from <<" "<< outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending DM message: MV:%d\n", mvIndex);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int deleteSuccess = atoi(buffer);
    if (deleteSuccess == 1){
        printf("MV %d Destroyed\n", mvIndex);
    }
    else{
        printf("MV %d Destroyed: UNSUCCESSFUL!\n", mvIndex);
    }
    // Done Networking Code
}
int GetMV_Syscall(int mvIndex, int arrayIndex){
    printf("In Get MV syscall\n");
    // Network Code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    if(mvIndex > num_MVs || mvIndex < 0) {
        printf("\tCheck if MV index exists – cannot get Condition out of bounds");
        return -1;
    }
    // buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "GetMV";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "GM" << mvIndex << " " << arrayIndex << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending GM message: MV:%d, ArrayIndex:%d\n", mvIndex, arrayIndex);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int getValue = atoi(buffer);
    if (getValue != -1){
        printf("MV %d get index %d value %d success\n", mvIndex, arrayIndex, getValue);
    }
    else{
        printf("MV %d get: UNSUCCESSFUL!\n", mvIndex);
    }
    // Done Networking Code

    return getValue;
}
void SetMV_Syscall(int mvIndex, int arrayIndex, int newValue){
    printf("In Set MV syscall\n");
    // Network Code
    stringstream ss;
    ss.clear();//clear any bits set
    ss.str(std::string());
    if(mvIndex > num_MVs || mvIndex < 0) {
        printf("\tCheck if MV index exists – cannot get Condition out of bounds");
        return;
    }
    // buf[len]='\0';
 
    // creates the message to send to the post office
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    char *data = "SetMV";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = 0;     
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outPktHdr.from = postOffice->getMachineID();
    printf("outPktHdr.from: %d, outMailHdr.from: %d\n", outPktHdr.from, outMailHdr.from);
    ss << "SM" << mvIndex << " " << arrayIndex << " " << newValue << " " << outPktHdr.from << " " << outMailHdr.from;
    outMailHdr.length = ss.str().size() + 1;
    printf("Sending SM message: MV:%d, NewIndex:%d, newValue:%d\n", mvIndex, arrayIndex, newValue);
    // Send the message to server
    bool success = postOffice->Send(outPktHdr, outMailHdr, (char*)ss.str().c_str()); 
    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    int setSuccess = atoi(buffer);
    if (setSuccess == 1){
        printf("MV Set Success\n");
    }
    else{
        printf("MV %d Set: UNSUCCESSFUL!\n", mvIndex);
    }
    // Done Networking Code
}


struct ForkThreadInfo{
    int vaddr;
    int pageAddress;
};

void ForkHelper(int threadInfo){
    ForkThreadInfo* tInfo = (ForkThreadInfo*) threadInfo;
    int vaddr = tInfo->vaddr;
    int pageAddress = tInfo->pageAddress;

    //update PCReg to vaddr
    //see save state in addrSpace
    machine->WriteRegister(PCReg, vaddr);
    machine->WriteRegister(NextPCReg, vaddr+4);

    //update StackReg so thread starts from starting point of stackReg
    machine->WriteRegister(StackReg, pageAddress);

    currentThread->space->RestoreState();

    machine->Run();
}

void Fork_Syscall(unsigned int vaddr){
    //data validation
    if(vaddr < 0){
        printf("\tThe address passed in is invalid...\n");
        return;
    }

    //valid address
    //init thread that will be forked
    Thread* newThread = new Thread("new_forked_thread");
    newThread->space = currentThread->space;
    
    //ForkThreadInfo* t;
    //t -> vaddr = vaddr;

    //save current state, PC, and machine registers before fork()
    int threadPageAddr = newThread->space->AllocatePages();
    //will return -1 if no bits are clear
    if (threadPageAddr != -1){
        newThread->firstStackPage = threadPageAddr;

        //create a ForkThreadInfo instance  
        //to store vaddr and pageAddr of newly created thread 
        ForkThreadInfo* threadInfo = new ForkThreadInfo();
        threadInfo->vaddr = vaddr;
        threadInfo->pageAddress = threadPageAddr;

        // update process table
        processTableLock->Acquire();
        for (int i = 0; i < 20; ++i){
            if(processTable[i].as == currentThread->space){
                processTable[i].threads[processTable[i].threadCount] = newThread;
                //t -> pageAddress = processTable[i].threadCount;
                processTable[i].totalThreads++;	
                processTable[i].threadCount++;
                break;
            }
        }
        processTableLock->Release();

        //addressSpace of thread done allocating 
        //pageTable done updating
        //now fork newly created thread: newThread
        newThread->Fork(ForkHelper, (int)threadInfo);
        currentThread->Yield();
    }
    //Should't happen...
    else{
        printf("\tno bits are clear in bitmap...\n");
    }

}

struct ExecThreadInfo {
    int vaddr;
    int pageAddress;
    ExecThreadInfo(): pageAddress(-1) {}
};

void exec_thread(int num){
    //initialize the registers by using currentthread->Space
    currentThread->space->InitRegisters();
    
    //Call Restore State through currentThread->Space
    currentThread->space->RestoreState();
    
    //Switch to User Mode
    machine->Run();
    
}

void Exec_Syscall(unsigned int vaddr, int len){
    char *buf = new char[len+1];
    
    if(!buf) {
        printf("Check len parameter in Exec_Syscall");
        return;
    }
    
    //Convert to physical address and read contents
    if(copyin(vaddr, len, buf) == -1) {
        printf("Check pointer passed into Exec_Syscall");
        delete buf;
        return;
    }
    
    //Open the file & store openfile pointer
    OpenFile *executable = fileSystem->Open(buf);
    
    if(executable == NULL){
        printf("Not able to open file for Exec_Syscall\n");
        return;
    }
    //New process, new address space
    AddrSpace *space;
    
    //AddrSpace arg takes in a filename. New address space for executable file
    //Address space consturtor cannot handle multiple processes.
    space = new AddrSpace(executable);
    space -> file = executable;
    
    //Create a new thread in new address space
    Thread* thread = new Thread(buf);
    //allocate the space created to this thread's space
    thread->space = space;
    thread->firstStackPage = thread->space->AllocatePages();
    thread->space->AllocatePages();
    
    //will return -1 if no bits are clear
    // if (threadPageAddr != -1){
    // newThread->firstStackPage = threadPageAddr;
    // }
    
    //Update the process table and related Data structures
    //  int pageAdress = newThread->space->AllocatePages();
    //processTableLock->Acquire();
    
    //processTableLock->Release();
    
    
    //write the space id to the register 2??
    
    //Fork new thread
    thread->Fork(exec_thread, 123);
}

int Exit_Syscall(int val){
    /* Exit requirements (as per assignment document): 
    The Exit system call must ensure that Thread::Finish is called, except for the very last thread running in Nachos. 
    For the very last thread (not the last thread in a process - unless it's the last process in Nachos), 
    you must call interrupt->Halt() to actually stop Nachos. If you don't do this, Nachos will not terminate. 
    This assignment requires that Nachos terminates.
    */
    //to be deleted
    printf("This is the val of matmult when Exit called: %d \n",val);
    if (true){
        printf("Exiting Thread..\n");
        currentThread->Finish();
        return 0;
    }
    //to be deleted
    processTableLock -> Acquire();				//beginning critical section
    int num_proc_with_threads, curr_thread_index = 0;
    int process_id_done = 0;					//shifted for project 3
    for(int i = 0; i < 10; i++) {
    	if(processTable[i].totalThreads > 0) {	//if a certain process does have running threads
    		num_proc_with_threads += processTable[i].totalThreads;
    	}
       if((processTable[i].as != NULL) && (processTable[i].as == currentThread -> space)){
             process_id_done = i;
       }
    	
    }
    
    if(num_proc_with_threads == 1) {
    	interrupt -> Halt();					//last thread in last process - done 
    }
    
	if(processTable[process_id_done].totalThreads == 1) {	
		//if current thread is part of a process in the process table 
		//&& this process has only the one thread
		//i.e. this is the last thread in a specific process
		
		for(int j = 0; j < nLockIndex; j++) {
			if(kernelLocks[j] -> addrSpace == processTable[process_id_done].as) {
				DestroyLock_Syscall(j);
				CreateLock_Syscall(j, 16);
			}
		}
		
		for(int k = 0; k < num_CVs; k++) {
			if(kernelCVs[k] -> as == processTable[process_id_done].as) {
				DestroyCondition_Syscall(k);
				CreateCondition_Syscall(k, 16);
			}
		}
		
		processTable[process_id_done].as -> DeallocatePages();	//deallocate all pages of this process – this process is done
		//delete processTable[i] -> as ->file;
		processTableLock -> Release();				//ending critical section
		currentThread -> Finish();
		return 0;
	}
    
    for(int i = 0; i < processTable[process_id_done].threadCount; i++) {
    	if(processTable[process_id_done].threads[i] != NULL){
			if(currentThread == processTable[process_id_done].threads[i]) {
				processTable[process_id_done].as -> DeallocateEightPages(processTable[process_id_done].threads[i] -> firstStackPage);
				processTable[process_id_done].threads[i] = NULL;	//new for project 3
				processTable[process_id_done].totalThreads--;		//new for project 3
				processTableLock -> Release();
				currentThread -> Finish();
				return 1;
			}
    	}
    }
    
    printf("\tExit syscall didn't work");    
    processTableLock -> Release();				//ending critical section
    return -1;
}


void PrintInt_Syscall(int num){
    std::cout << "PRINT NUM: " << num << std::endl;
}

/*Populate the TLB*/
void HandlePageFault(int neededVPN, int physPageNum){
	if(machine -> tlb[tlbIndex].valid == true) {
		ipt[machine -> tlb[tlbIndex].physicalPage].dirty = machine -> tlb[tlbIndex].dirty;
	}

    machine->tlb[tlbIndex].virtualPage = ipt[neededVPN].virtualPage;
    machine->tlb[tlbIndex].physicalPage = physPageNum;
    machine->tlb[tlbIndex].valid = ipt[neededVPN].valid;
    machine->tlb[tlbIndex].use = ipt[neededVPN].use;
    machine->tlb[tlbIndex].dirty = ipt[neededVPN].dirty;
    machine->tlb[tlbIndex].readOnly = ipt[neededVPN].readOnly;
    tlbIndex++;
    tlbIndex = tlbIndex % TLBSize;
}

int HandleFullMemory() {
	//we come here when there is nothing to return in bitmap -- no pages available
	//this is where we use either the RANDOM or the FIFO eviction policy (as gotten through command line)
	//and use it to evict a page (i.e. make room)
	
    int physPageNum = -1;
    
    if(pageEvictionPolicy == FIFO) {
        while(true){
        	evictFifoLock -> Acquire();				//begin critical section
        	physPageNum = evictFifoList -> front();	//take the first page
        	evictFifoList -> pop_front();			//remove the first page from the list
        	if(physPageNum == -1) {					//something went wrong
        		printf("Fifo returned -1?\n");
                ASSERT(0);
        	}
        	else {									//found an acceptable page
        		evictFifoLock -> Release();			
        		break;								//done
        	}
        	evictFifoLock -> Release();				//end critical section (we have release here as well to account for the (physPageNum == -1) case
        }
    }
    
    else if(pageEvictionPolicy == RANDOM) {			//note: random is random, we have no lists or data structures to maintain pages for this
        while(true){
            physPageNum = rand() % NumPhysPages;	//pick a random page from the total number of pages

            if(ipt[physPageNum].use == FALSE){ 		
                ipt[physPageNum].use = TRUE; 		//Mark in use by this eviction
                break;
            }
        }
    }
    
    else {
    	printf("\nCheck page eviction policy -- neither of FIFO or RANDOM chosen\n");
    }

    IntStatus oldLevel = interrupt -> SetLevel(IntOff);
	for(int i = 0; i < TLBSize; i++){
		if((machine -> tlb[i].physicalPage == physPageNum) && (machine -> tlb[i].valid)) {
        	ipt[physPageNum].dirty = machine->tlb[i].dirty;	//tlb to ipt
			machine -> tlb[i].valid = FALSE; //invalidate
			break;
		}
	}
    interrupt->SetLevel(oldLevel);
	
    if(ipt[physPageNum].dirty == TRUE){ 	//do we need to write to swap file? 
        swapFileLock -> Acquire();			//begin critical section
        int freeLoc = swapFileBitMap -> Find(); 	//find first free location in swapFile
        if(freeLoc == -1) {						//i.e. swap file is full already
            printf("Swap file is full");
            ASSERT(0);
        }

        swap_file -> WriteAt(&(machine -> mainMemory[physPageNum * PageSize]), PageSize, freeLoc * PageSize);	//main stuff 
        swapFileLock -> Release();			//done writing to swap file

        ipt[physPageNum].as -> pageTableLock -> Acquire();							//begin critical section
        ipt[physPageNum].as -> pageTable[ipt[physPageNum].virtualPage].offset = freeLoc; 	//Set offset
        ipt[physPageNum].as -> pageTable[ipt[physPageNum].virtualPage].base = SWAP_FILE; 		//Set location to SWAP
        ipt[physPageNum].as -> pageTableLock -> Release();							//end critical section
    }
    
    else {
    
    }

    ipt[physPageNum].use = FALSE; 
  	return physPageNum;
}

int HandleIPTMiss(int virtPageNum) {
	AddrSpace* addrSpace = currentThread -> space;
	int physPageNum;
	bitMapLock -> Acquire();		//entering critical section
	physPageNum = memBitMap -> Find();	//looking for any existing physical pages in bitMap -- returns -1 if no pages available
	bitMapLock -> Release();		//exiting critical section
	
	iptLock -> Acquire();			//entering critical section
	if(physPageNum == -1) {
		physPageNum = HandleFullMemory();	//after this, we must have at least one page available
	}
	
	if((physPageNum < 0) || (physPageNum > 33)) {
		ASSERT(0);
	}
	
	if(pageEvictionPolicy == FIFO) {
		evictFifoLock -> Acquire();			//begin critical section
		evictFifoList -> push_back(physPageNum);
		evictFifoLock -> Release();			//end critical section
	}
	
    addrSpace -> pageTableLock -> Acquire();	//begin critical section
	if(addrSpace -> pageTable[virtPageNum].base == EXECUTABLE) { //virtual page is in code
		addrSpace -> pageTable[virtPageNum].physicalPage = physPageNum;
		addrSpace -> pageTable[virtPageNum].valid = TRUE;
		addrSpace -> pageTable[virtPageNum].base = EXECUTABLE;
		addrSpace -> pageTableLock -> Acquire();//end critical section

		ipt[physPageNum].virtualPage = virtPageNum;
		ipt[physPageNum].physicalPage = physPageNum;
		ipt[physPageNum].valid = TRUE;
		ipt[physPageNum].use = TRUE;
		ipt[physPageNum].dirty = FALSE;
		ipt[physPageNum].readOnly = FALSE;
		ipt[physPageNum].as = addrSpace;

		addrSpace -> file -> ReadAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, addrSpace -> pageTable[virtPageNum].offset);
	}
	
	else if(addrSpace -> pageTable[virtPageNum].base == SWAP_FILE) { //virtual page is in swap
        addrSpace -> pageTable[virtPageNum].physicalPage = physPageNum;
        addrSpace -> pageTable[virtPageNum].valid = TRUE;
        addrSpace -> pageTable[virtPageNum].base = NEITHER;
        addrSpace -> pageTableLock -> Release();
        swapFileLock -> Acquire();
        swap_file -> ReadAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, addrSpace -> pageTable[virtPageNum].offset * PageSize);
        swapFileBitMap -> Clear(addrSpace -> pageTable[virtPageNum].offset);
        swapFileLock -> Release();

		ipt[physPageNum].virtualPage = virtPageNum;
		ipt[physPageNum].physicalPage = physPageNum;
		ipt[physPageNum].valid = TRUE;
		ipt[physPageNum].dirty = TRUE;
		ipt[physPageNum].readOnly = FALSE;
		ipt[physPageNum].as = addrSpace;
	}
	
   else { //virtual page is in neither
        addrSpace -> pageTable[virtPageNum].physicalPage = physPageNum;
        addrSpace -> pageTable[virtPageNum].valid = TRUE;
        addrSpace -> pageTableLock -> Release();

        ipt[physPageNum].virtualPage = virtPageNum;
        ipt[physPageNum].physicalPage = physPageNum;
        ipt[physPageNum].valid = TRUE;
        ipt[physPageNum].dirty = FALSE;
        ipt[physPageNum].readOnly = FALSE;
        ipt[physPageNum].as = addrSpace;
    }
	
	return physPageNum;
}


void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0;   // the return value from a syscall

    if ( which == SyscallException ) {
    switch (type) {
        default:
        DEBUG('a', "Unknown syscall - shutting down.\n");
        case SC_Halt:
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
        break;
        case SC_Create:
        DEBUG('a', "Create syscall.\n");
        Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
        case SC_Open:
        DEBUG('a', "Open syscall.\n");
        rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
        case SC_Write:
        DEBUG('a', "Write syscall.\n");
        Write_Syscall(machine->ReadRegister(4),
                  machine->ReadRegister(5),
                  machine->ReadRegister(6));
        break;
        case SC_Read:
        DEBUG('a', "Read syscall.\n");
        rv = Read_Syscall(machine->ReadRegister(4),
                  machine->ReadRegister(5),
                  machine->ReadRegister(6));
        break;
        case SC_Close:
        DEBUG('a', "Close syscall.\n");
        Close_Syscall(machine->ReadRegister(4));
        break;
        
        case SC_Yield:
        DEBUG('a', "Yield syscall.\n");
        Yield_Syscall();
        break;
            
        case SC_CreateLock:
        DEBUG('a', "CreateLock syscall.\n");
        rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;
            
        case SC_DestroyLock:
        DEBUG('a', "DestroyLock syscall.\n");
        DestroyLock_Syscall(machine->ReadRegister(4));
        break;

        case SC_Acquire:
        DEBUG('a', "Acquire syscall.\n");
        Acquire_Syscall(machine->ReadRegister(4));
        break;

        case SC_Release:
        DEBUG('a', "Release syscall.\n");
        Release_Syscall(machine->ReadRegister(4));
        break;

        case SC_CreateCondition:
        DEBUG('a', "CreateCondition syscall.\n");
        rv = CreateCondition_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_DestroyCondition:
        DEBUG('a', "DestroyCondition syscall.\n");
        DestroyCondition_Syscall(machine->ReadRegister(4));
        break;

        case SC_Signal:
        DEBUG('a', "Signal syscall.\n");
        Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_Broadcast:
        DEBUG('a', "Broadcast syscall.\n");
        Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_Wait:
        DEBUG('a', "Wait syscall.\n");
        Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_CreateMV:
        DEBUG('a', "CreateMV syscall.\n");
        rv = CreateMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
        break;

        case SC_DestroyMV:
        DEBUG('a', "DestroyMC syscall.\n");
        DestroyMV_Syscall(machine->ReadRegister(4));
        break;

        case SC_GetMV:
        DEBUG('a', "GetMV syscall.\n");
        rv = GetMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_SetMV:
        DEBUG('a', "SetMV syscall.\n");
        SetMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
        break;

        case SC_Fork:
        DEBUG('a', "Fork syscall.\n");
        Fork_Syscall(machine->ReadRegister(4));
        break;
        
        case SC_Exec:
        DEBUG('a', "Exec syscall.\n");
        Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
        break;

        case SC_Exit:
        DEBUG('a', "Exit syscall.\n");
        rv = Exit_Syscall(machine->ReadRegister(4));
        break;
            
        case SC_PrintInt:
        DEBUG('a', "Print syscall.\n");
        PrintInt_Syscall(machine->ReadRegister(4));
        break;

    }

    // Put in the return value and increment the PC
    machine->WriteRegister(2,rv);
    machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
    return;
        //pagefault exception case
    }else if(which == PageFaultException ){
        int neededVPN = (machine->ReadRegister(BadVAddrReg))/PageSize;
    
        AddrSpace* addrSpace = currentThread->space; //we will use this to check with IPT contents
        iptLock -> Acquire();
        int physPageNum = -1;
        for(int i = 0; i < NumPhysPages; i++){
        	if((ipt[i].valid == TRUE) && (ipt[i].virtualPage == neededVPN) && (ipt[i].as == addrSpace)){
        		physPageNum = i;	//found page in IPT -- IPT "hit"
        		break;				//can exit
        	}
        }
        
        if(physPageNum == -1) {		//if this page was not in the IPT -- IPT "miss"
			//deal with IPT misses here
			printf("IPT miss");
			iptLock -> Release();	//end critical section
			physPageNum = HandleIPTMiss(neededVPN);
        }
        
        //update TLB
        IntStatus oldLevel = interrupt -> SetLevel(IntOff); 
		iptLock -> Release();
        HandlePageFault(neededVPN, physPageNum);
        interrupt -> SetLevel(oldLevel);
        //Create a function here called TLB MISS. This will find the needed virtual page & copy information into TLB. TLB is a trnaslation entry just like PageTable,
        //so it is easy to copy all the fields. The question now is what is the needed VPN?
        
        //^ the needed virtual page number is the page that is causing the pageFault -- 
        //^ the reason why we entered this function to begin with
        //^ you've got it right
		ipt[physPageNum].use = FALSE; 
    
    } else {
      //cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}

