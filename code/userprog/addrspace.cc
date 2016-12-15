// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
	table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
	}
	lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

//some variables and tools for addrspace
//to keep track of physical pages
BitMap* memBitMap = new BitMap(NumPhysPages);
Lock* bitMapLock = new Lock("bitMapLock");

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {
    NoffHeader noffH;
    unsigned int i, size;
    pageBitMap = new BitMap(100);
    pageBitMapLock = new Lock("pageBitMapLock");
    pageTableLock = new Lock("pageTableLock");

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
    pageCount = divRoundUp(noffH.code.size + noffH.initData.size, PageSize); //changed for project 3
    stackStart = divRoundUp(noffH.code.size + noffH.initData.size + noffH.uninitData.size, PageSize); //new for project 3
    numPages = divRoundUp(size, PageSize) + 400;
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 

    pageTableLock->Acquire();

    pageTable = new PTEntry[numPages];
    for (i = 0; i < numPages; i++) {

        bitMapLock->Acquire();
        //to find an unused page of physical memory
        int physicalPageNumber = memBitMap->Find();
        bitMapLock->Release();

        if (physicalPageNumber != -1){
            pageTable[i].virtualPage = i; 
            pageTable[i].physicalPage = physicalPageNumber;
            pageTable[i].valid = TRUE;
            pageTable[i].use = TRUE;
            pageTable[i].dirty = FALSE;
            pageTable[i].readOnly = FALSE;  
                            
            iptLock -> Acquire();
            ipt[physicalPageNumber].virtualPage = i;
            ipt[physicalPageNumber].physicalPage = physicalPageNumber;
            ipt[physicalPageNumber].valid = TRUE;
            ipt[physicalPageNumber].use = TRUE;
            ipt[physicalPageNumber].dirty = FALSE;
            ipt[physicalPageNumber].readOnly = FALSE;
            ipt[physicalPageNumber].as = this;
            iptLock -> Release();

            executable->ReadAt(&(machine->mainMemory[PageSize*physicalPageNumber]), PageSize, 40+i*PageSize);
        }
        //shouldn't happen
        else{
            printf("no bits are clear in bitmap...\n");
        }
        
    }
    pageTableLock->Release();
    //set up TLB

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    int startStack = AllocatePages();
    if (startStack == -1){
        printf("no clear bits in bitmap...\n");
        return;
    }
    machine->WriteRegister(StackReg, startStack);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

int AddrSpace::AllocatePages(){
    pageBitMapLock->Acquire();
    
    int startPage = pageBitMap->Find();
    if (startPage != -1){
        startPage = (pageCount + startPage*8)*PageSize;
    }
    

    pageBitMapLock->Release();

    return startPage;
}

void AddrSpace::DeallocatePages() {	//deallocate, i.e. dump or return, all pages of a specific process
	pageTableLock -> Acquire();		//enter critical section
	bitMapLock -> Acquire();
	iptLock -> Acquire();
	
	for(unsigned int i = 0; i < numPages; i++) {						//for all pages
		ipt[pageTable[i].physicalPage].valid = FALSE;	
		memBitMap -> Clear(pageTable[i].physicalPage);	//give away physical pages
	}
	
	iptLock ->  Release();
	bitMapLock -> Release();
	pageTableLock -> Release(); 	//exit critical section
}


void AddrSpace::DeallocateEightPages(int startingPage) {	//deallocate only eight pages, e.g. for one thread in a process, etc
	pageTableLock -> Acquire();												//enter critical section
	bitMapLock -> Acquire();
	iptLock -> Acquire();
	
	startingPage /= PageSize;
	
	for(int i = 0; i < 8; i++) {
		ipt[pageTable[startingPage + i].physicalPage].valid = FALSE;
		memBitMap -> Clear(pageTable[startingPage + i].physicalPage);	//give away that block of 8 pages
	}
	
	iptLock -> Release();
	bitMapLock -> Release();
	pageTableLock -> Release();												//exit critical section
	
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
    for(int i = 0; i < TLBSize; i++){
         IntStatus old = interrupt->SetLevel(IntOff);
         machine->tlb[i].valid = false;
        interrupt->SetLevel(old);
    }
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
   // machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
