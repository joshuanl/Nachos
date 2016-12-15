// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"

#define UserStackSize (SectorSize * 50) 
//1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256

extern BitMap* memBitMap;
extern Lock* bitMapLock;

enum disk_loc {NEITHER, EXECUTABLE, SWAP_FILE};

class PTEntry : public TranslationEntry {	
public:
    PTEntry(){};			//Creates page table entry
    ~PTEntry (){};			//Deletes page table entry
    disk_loc base;				//Where is the page
    int offset;				//Where is the byte within the page
    
private:
    
};

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code
    int AllocatePages();
    void DeallocatePages();
    void DeallocateEightPages(int startingPage);
    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    Table fileTable;			// Table of openfiles
    unsigned int pageCount;    // Which page thread is on
    int stackStart;		//new for project 3
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
    PTEntry* pageTable;	// Assume linear page table translation
    Lock* pageTableLock;	//made public in project 3
    OpenFile* file;

 private:
    BitMap* pageBitMap;
    Lock* pageBitMapLock;
    
};

#endif // ADDRSPACE_H
