// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "list"
#include "../network/post.h"

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.


//include lock and addrspace?
class AddrSpace;
class Lock;
class Condition;
class IPT;
extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
struct KernelLock{  //Kernel Lock Table
    char* lockName;
    Lock* lock;
    AddrSpace* addrSpace;
    bool isToBeDeleted;
    int numRequested;
    KernelLock(): lockName(""),lock(NULL),addrSpace(NULL),isToBeDeleted(false),numRequested(0){};
};
extern KernelLock* kernelLocks[200];

struct ServerLock{  //Server Lock Table
    bool busy;
    int owner;
    List* queue;
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    ServerLock(): busy(false), owner(0),queue(NULL){};
};

struct ServerCV{
    bool inWait;
    int lockIndex; //ServerLock table index
    List* queue;
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    ServerCV() : inWait(false),lockIndex(-1),queue(NULL) {};
};

//Condition variable data structures
struct KernelCV {
    char* cvName;
    Condition* cv;
    AddrSpace* as;
    bool isToBeDeleted;
    int numRequested;
};
extern KernelCV* kernelCVs[200];

//kernelMV table data structure
struct KernelMV{
    char* mvName;
    int size;
    int* array;
    AddrSpace* as;
    bool isToBeDeleted;
    int numRequested;
    KernelMV(): mvName("mvName"),size(NULL),array(NULL), as(NULL), isToBeDeleted(0){};
};
extern KernelMV* kernelMVs[200];

//Process data stucture
struct SingleProcess{
	int threadCount;
	AddrSpace* as;
	Thread* threads[50];
	int totalThreads;
	SingleProcess(): threadCount(0), as(NULL){};
};
extern IPT* ipt;
 //can i do this instead of new? for ITP & what is extern?
extern int tlbIndex;
 //extern ITP itp[numPhysPages];
extern Lock* iptLock;
//can i do this instead of new? for ITP & what is extern?

extern OpenFile* swap_file;
//Now, for the above, we also need methods of eviction as suggested by the Assignment Document
//"Implement Random page replacement policy and FIFO page replacement ONLY"
enum evict {RANDOM, FIFO};
extern evict pageEvictionPolicy;
//(!) to keep track of an available page in the swap file, we will need a swap BitMap
extern BitMap* swapFileBitMap;
extern Lock* swapFileLock;
//if our eviction policy is FIFO
extern list<int>* evictFifoList;
extern Lock* evictFifoLock;

extern SingleProcess processTable[20];
extern int processTableCount;
extern Lock* processTableLock;
//does my ipt lock go here or can i put in exception?

class IPT : public TranslationEntry {
public:
    IPT(){};	// Create an IPT
    ~IPT(){};			// De-allocate an IPT
    AddrSpace* as;
    
    //functions
    
private:
    
};
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;


//array extern of locks? why? include addrspace?

#endif

#endif // SYSTEM_H
