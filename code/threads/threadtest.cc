// threadtest.cc 
//  Simple test case for the threads assignment.
//
//  Create two threads, and have them context switch
//  back and forth between themselves by calling Thread::Yield, 
//  to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

#ifdef CHANGED
#include "synch.h"
#endif

//----------------------------------------------------------------------
// SimpleThread
//  Loop 5 times, yielding the CPU to another ready thread 
//  each iteration.
//
//  "which" is simply a number identifying the thread, for debugging
//  purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
    printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
//  Set up a ping-pong between two threads, by forking a thread 
//  to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
// lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the
// lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
// lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
// done
Lock t1_l1("t1_l1");          // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
    
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
            t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
            t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {
    
    t1_s1.P();  // Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock
    
    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
           t1_l1.getName());
    t1_l1.Acquire();
    
    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
            t1_l1.getName());
    for (int i = 0; i < 10; i++)
        ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
            t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {
    
    t1_s2.P();  // Wait until t2 is ready to try to acquire the lock
    
    t1_s3.V();  // Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
        printf("%s: Trying to release Lock %s\n",currentThread->getName(),
               t1_l1.getName());
        t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");        // For mutual exclusion
Condition t2_c1("t2_c1");   // The condition variable to test
Semaphore t2_s1("t2_s1",0); // To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
// done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
           t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();  // release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();  // Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
           t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");        // For mutual exclusion
Condition t3_c1("t3_c1");   // The condition variable to test
Semaphore t3_s1("t3_s1",0); // To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
// done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
           t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {
    
    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ )
        t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
           t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}

// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");        // For mutual exclusion
Condition t4_c1("t4_c1");   // The condition variable to test
Semaphore t4_s1("t4_s1",0); // To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
// done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
           t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {
    
    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ )
        t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
           t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");        // For mutual exclusion
Lock t5_l2("t5_l2");        // Second lock for the bad behavior
Condition t5_c1("t5_c1");   // The condition variable to test
Semaphore t5_s1("t5_s1",0); // To make sure t5_t2 acquires the lock after
// t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();  // release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
           t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();  // Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
           t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t5_l1.getName());
    t5_l1.Release();
}

// ---------------------------------------------------
//PART 2 - has to be defined before TestSuite() so it can be picked up by TestSuite()

//Initialize Variables
//General
#define NUM 5
#define NUM_CUST 50
#define NUM_SENATOR 10
int test_chosen = -1;
enum clerkStates {BUSY, AVAILABLE, ONBREAK, NOTAVAILABLE};  //NOTAVAILABLE = No clerk there, not on break, just no one there
Semaphore p2_done("p2_done",0);
//Semaphore customer_waiting("customer_waiting",0);
int numCustomers = 0; //number of customers
bool customer_apps[NUM_CUST];
bool customer_likesPhoto[NUM_CUST];
bool customer_photos[NUM_CUST];
bool customer_passport[NUM_CUST];
int totalCustomerMoney = 0;
Condition* customerWaitSenatorCV;
int waitingCustomer = 0;

//Application Clerk Variables
int numAppClerk = 0;
Lock* appClerkLineLock = new Lock("appClerkLineLock"); 
Lock* appClerkSSNLock[NUM];
int appClerkLineCount[NUM] = {0,0,0,0,0};       //just values for testing
int appClerkBribeLineCount[NUM] = {0,0,0,0,0};  //just values for testing
clerkStates appClerkState[NUM] = {AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE};
Condition* appClerkSSNCV[NUM];
Condition* appClerkLineCV[NUM];
Condition* appClerkBribeLineCV[NUM];
Lock* appClerkLock[NUM];
Condition* appClerkCV[NUM];
int appClerkMoney[NUM] = {0,0,0,0,0};
bool appClerkCustPhoto[NUM];
int appClerkCustSSN[NUM]={-1,-1,-1,-1,-1};
bool appClerkCustApplication[NUM];
Lock* appClerkMoneyLock[NUM];



//Photo Clerk Variables
int numPhotoClerk = 0;
Lock* photoClerkLineLock = new Lock("photoClerkLineLock"); 
int photoClerkLineCount[NUM] = {0,0,0,0,0};    //just values for testing
int photoClerkBribeLineCount[NUM] = {0,0,0,0,0};//just values for testing
clerkStates photoClerkState[NUM] = {AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE};
Condition* photoClerkLineCV[NUM];
Condition* photoClerkBribeLineCV[NUM];
Lock* photoClerkLock[NUM];
Condition* photoClerkCV[NUM];
int photoClerkMoney[NUM] = {0,0,0,0,0};
bool photoClerkCustPhoto[NUM];
int photoClerkCustSSN[NUM]={-1,-1,-1,-1,-1};
bool photoClerkCustApplication[NUM];
Lock* photoClerkMoneyLock[NUM];


//Passport Clerk Variables
int numPassportClerk = 0;
Lock* passportClerkLineLock = new Lock("passportClerkLineLock"); 
int passportClerkLineCount[NUM] = {0,0,0,0,0};
int passportClerkBribeLineCount[NUM] = {0,0,0,0,0};
clerkStates passportClerkState[NUM] = {AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE};
Condition* passportClerkLineCV[NUM];
Condition* passportClerkBribeLineCV[NUM];
Lock* passportClerkLock[NUM];
Condition* passportClerkCV[NUM];
int passportClerkMoney[NUM] = {0,0,0,0,0};
bool passportClerkCustPhoto[NUM];
int passportClerkCustSSN[NUM]={-1,-1,-1,-1,-1};
bool passportClerkCustApplication[NUM];
Lock* passportClerkMoneyLock[NUM];


//Cashier Variables
int numCashier = 0;
Lock* cashierLineLock = new Lock("cashierLineLock"); 
int cashierLineCount[NUM] = {0,0,0,0,0};
clerkStates cashierState[NUM] = {AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE,AVAILABLE};
Condition* cashierLineCV[NUM]; //?
Lock* cashierLock[NUM];
Condition* cashierCV[NUM];
int cashierMoney[NUM] = {0,0,0,0,0};
bool cashierCustPhoto[NUM];
int cashierCustSSN[NUM]={-1,-1,-1,-1,-1};
bool cashierCustApplication[NUM];
Lock* cashierMoneyLock[NUM];


//Manager variables – timer thread
Lock* appClerkBreakLock = new Lock("appClerkBreakLock");
Lock* photoClerkBreakLock = new Lock("photoClerkBreakLock");
Lock* passportClerkBreakLock = new Lock("passportClerkBreakLock");
Lock* cashierBreakLock = new Lock("cashierBreakLock");
Condition* appClerkBreakCV[NUM];
Condition* photoClerkBreakCV[NUM];
Condition* passportClerkBreakCV[NUM];
Condition* cashierBreakCV[NUM];
bool senatorInOffice = false;

//Senator variables
int numSenator = 0; 
int totalSenatorMoney = 0;
int senatorCount;
Lock* senatorLock = new Lock("senatorLock");
Condition* senatorCV;
bool senator_apps[NUM_SENATOR];
bool senator_likesPhoto[NUM_SENATOR];
bool senator_photos[NUM_SENATOR];
bool senator_passport[NUM_SENATOR];

int senator_count = 0;


// --------------------------------------------------
//Thread Code
//Customer variables
struct customer {
    int myLine;
    int myMoney;
    int SSN;
    bool isDone; 
    int pickyness;
} customers[NUM_CUST];

//Senator variables
struct senator {
    int myLine;
    int myMoney;
    int SSN;
    bool isDone; 
    int pickyness;
} senators[NUM_SENATOR];

//Initialize all customer data
void init_customer_data() {
    srand(time(NULL));
    int money_options[4] = {100, 600, 1100, 1600};
    for(int i = 0; i < numCustomers; i++) {
        customers[i].myLine = -1;        
        customers[i].myMoney = money_options[rand() % 4];
        totalCustomerMoney = totalCustomerMoney + customers[i].myMoney;
        customers[i].pickyness = (rand() % 98)+1;
        customers[i].SSN = i;
        customers[i].isDone = false;    //this is true when the customer is entirely done with the passport office
        customer_likesPhoto[i] = false; //true if customer likes the photo
        customer_apps[i] = false;       //true when finish visiting app clerk
        customer_photos[i] = false;     //true when finish visiting photo clerk
        customer_passport[i] = false;   //true when finish visiting passport clerk
    }
}

//Initialize all senator data
void init_senator_data() {
    srand(time(NULL));
    for(int i = 0; i < numSenator; i++) {
        senators[i].myLine = -1;        
        senators[i].myMoney = 100;
        totalSenatorMoney = totalSenatorMoney + senators[i].myMoney;
        senators[i].pickyness = (rand() % 98)+1;
        senators[i].SSN = i;
        senators[i].isDone = false;    //this is true when the customer is entirely done with the passport office
        senator_likesPhoto[i] = false; //true if customer likes the photo
        senator_apps[i] = false;       //true when finish visiting app clerk
        senator_photos[i] = false;     //true when finish visiting photo clerk
        senator_passport[i] = false;   //true when finish visiting passport clerk
    }
}


//initialize all locks and CV
void init_locks_and_CV(){
    char *name;

    //init locks and CVs for Senator

    name = new char [30];
    sprintf(name,"senatorCV");
    senatorCV = new Condition(name);

    //init Locks and CVs for Customer
    name = new char [30];
    sprintf(name,"customerWaitSenatorCV");
    customerWaitSenatorCV = new Condition(name);
    

    //init locks and CVs for Application Clerk
    //
    //Condition* appClerkLineCV[NUM];
    //Condition* appClerkBribeLineCV[NUM];
    //Lock* appClerkLock[NUM];
    //Condition* appClerkCV[NUM];
    //Lock* appClerkMoneyLock[NUM];
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"appClerkLineCV_%d",i);
        appClerkLineCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"appClerkBribeLineCV_%d",i);
        appClerkBribeLineCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"appClerkCV_%d",i);
        appClerkCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"appClerkSSNCV_%d",i);
        appClerkSSNCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"appClerkLock_%d",i);
        appClerkLock[i] = new Lock(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"appClerkSSNLock_%d",i);
        appClerkSSNLock[i] = new Lock(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"appClerkMoneyLock_%d",i);
        appClerkMoneyLock[i] = new Lock(name);
    }

    //init locks and CVs for Photo Clerk
    //
    // Condition* photoClerkLineCV[NUM];
    // Condition* photoClerkBribeLineCV[NUM];
    // Lock* photoClerkLock[NUM];
    // Condition* photoClerkCV[NUM];
    // Lock* photoClerkMoneyLock[NUM];
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"photoClerkLineCV_%d",i);
        photoClerkLineCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"photoClerkBribeLineCV_%d",i);
        photoClerkBribeLineCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"photoClerkCV_%d",i);
        photoClerkCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"photoClerkLock_%d",i);
        photoClerkLock[i] = new Lock(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"photoClerkMoneyLock%d",i);
        photoClerkMoneyLock[i] = new Lock(name);
    }

    //init locks and CVs for Passport Clerk
    //
    // Condition* passportClerkLineCV[NUM];
    // Condition* passportClerkBribeLineCV[NUM];
    // Lock* passportClerkLock[NUM];
    // Condition* passportClerkCV[NUM];
    // Lock* passportClerkMoneyLock[NUM];
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"passportClerkLineCV_%d",i);
        passportClerkLineCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"passportClerkBribeLineCV_%d",i);
        passportClerkBribeLineCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"passportClerkCV_%d",i);
        passportClerkCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"passportClerkLock_%d",i);
        passportClerkLock[i] = new Lock(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"passportClerkMoneyLock_%d",i);
        passportClerkMoneyLock[i] = new Lock(name);
    }

    //init locks and CVs for cashier
    //
    // Condition* cashierLineCV[NUM]; //?
    // Lock* cashierLock[NUM];
    // Condition* cashierCV[NUM];
    // Lock* cashierMoneyLock[NUM];
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"cashierLineCV_%d",i);
        cashierLineCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"cashierCV_%d",i);
        cashierCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [20];
        sprintf(name,"cashierLock_%d",i);
        cashierLock[i] = new Lock(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [20];
        sprintf(name,"cashierMoneyLock_%d",i);
        cashierMoneyLock[i] = new Lock(name);
    }

    //init locks and CVs for Manager
    //
    // Condition* appClerkBreakCV[5];
    // Condition* photoClerkBreakCV[5];
    // Condition* passportClerkBreakCV[5];
    // Condition* cashierBreakCV[5];
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"appClerkBreakCV_%d",i);
        appClerkBreakCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"photoClerkBreakCV_%d",i);
        photoClerkBreakCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"passportClerkBreakCV_%d",i);
        passportClerkBreakCV[i] = new Condition(name);
    }
    for (int i = 0; i < NUM; ++i){
        name = new char [30];
        sprintf(name,"cashierBreakCV_%d",i);
        cashierBreakCV[i] = new Condition(name);
    }
}

//index is customer ssd
void Customer(int customerIndex) {
    
    int myLine = customers[customerIndex].myLine;
    int myMoney = customers[customerIndex].myMoney;
    //initialize successful
    //printf("Customer_%d has arrived the passport office, carrying $%d\n", customerIndex, myMoney);


    //customer check if there is senator coming into the office
    senatorLock->Acquire();
    if (senatorCount > 0){
     //   printf("there is a senator waiting, customer %d waiting...\n", customerIndex);
        waitingCustomer++;
        //customer_waiting.V();
        customerWaitSenatorCV->Wait(senatorLock);
       // printf("All senators have left, Customer %d is back in the office...\n", customerIndex);
    }
    senatorLock->Release();
    
    appClerkLineLock->Acquire();
    photoClerkLineLock->Acquire();
    int peopleInAppClerk[numAppClerk];
    for (int i = 0; i < numAppClerk; i++){
        peopleInAppClerk[i] = appClerkLineCount[i] + appClerkBribeLineCount[i];
    }
    int peopleInPhotoClerk[numPhotoClerk];
    for (int i = 0; i < numPhotoClerk; i++){
        peopleInPhotoClerk[i] = photoClerkLineCount[i] + photoClerkBribeLineCount[i];
    }

    //find shorter line among app clerk and photo clerk
    int leastPeopleInApp = peopleInAppClerk[0];    //number of people in the shortest line in App
    int myAppLine = 0;
    for (int i = 0; i < numAppClerk; i++){
        if((peopleInAppClerk[i] < leastPeopleInApp) && (appClerkState[i] != NOTAVAILABLE) && (appClerkState[i] != ONBREAK)){
            myAppLine = i;
            leastPeopleInApp = peopleInAppClerk[i];
        }
    }
    //printf("shortest line in App: %d, with %d people..\n", myAppLine, leastPeopleInApp);
    int leastPeopleInPhoto = peopleInPhotoClerk[0];    //number of people in the shortest line in Photo
    int myPhotoLine = 0;
    for (int i = 0; i < numPhotoClerk; i++){
        if((peopleInPhotoClerk[i] < leastPeopleInPhoto) && (photoClerkState[i] != NOTAVAILABLE) && (photoClerkState[i] != ONBREAK)){
            myPhotoLine = i;
            leastPeopleInPhoto = peopleInPhotoClerk[i];
        }
    }
    //printf("shortest line in Photo: %d, with %d people..\n", myPhotoLine, leastPeopleInPhoto);
    int leastPeople = min(leastPeopleInPhoto, leastPeopleInApp);
    myLine = (leastPeopleInApp > leastPeopleInPhoto)? myPhotoLine:myAppLine;

    //go to Application Clerk, then Photo Clerk
    if(leastPeopleInPhoto >= leastPeopleInApp){
        //printf("Customer_%d sees application clerk line is shorter, go there!\n", customerIndex );
        photoClerkLineLock->Release();
        if (appClerkState[myLine] == AVAILABLE){
            appClerkState[myLine] = BUSY;
        }
        if (myMoney > 500){
            myMoney = myMoney - 500;
            appClerkBribeLineCount[myLine]++;   //customer increment line count it is in
            //printf("Customer_%d is waiting in bribe line %d of Application department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, appClerkLineCount[myLine], appClerkBribeLineCount[myLine], myMoney);
            if((test_chosen == 8) || (test_chosen == 1) || (test_chosen == 7)) {
                printf("Customer %d has gotten in bribe line for ApplicationClerk %d.\n", customerIndex, myLine);
            }
            appClerkBribeLineCV[myLine]->Wait(appClerkLineLock);
            appClerkBribeLineCount[myLine]--;   //customer decrement line count after waken up

            appClerkSSNLock[myLine]->Acquire();
            appClerkLineLock->Release();

            appClerkCustSSN[myLine] = customerIndex;
            appClerkSSNCV[myLine]->Signal(appClerkSSNLock[myLine]);
            appClerkSSNCV[myLine]->Wait(appClerkSSNLock[myLine]);

            appClerkLineLock->Acquire();
            appClerkSSNLock[myLine]->Release();
        }
        else{
            appClerkLineCount[myLine]++;    //customer increment line count it is in
            //printf("Customer_%d is waiting in regular line %d of Application department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, appClerkLineCount[myLine], appClerkBribeLineCount[myLine], myMoney);
            if((test_chosen == 8) || (test_chosen == 1) || (test_chosen == 7)) {
                printf("Customer %d has gotten in regular line for ApplicationClerk %d.\n", customerIndex, myLine);
            }

            appClerkLineCV[myLine]->Wait(appClerkLineLock);
            appClerkLineCount[myLine]--;    //customer decrement line count after waken up
        }
        appClerkLineLock->Release();

        //interaction between customer and application clerk
        appClerkLock[myLine]->Acquire();
        appClerkCustSSN[myLine] = customerIndex;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Customer %d has given SSN %d to ApplicationClerk %d\n", customerIndex, customerIndex, myLine);
        }
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    //customer signals app clerk that customer is ready to be serviced
        appClerkCV[myLine]->Wait(appClerkLock[myLine]);      //wait for app clerk to respond clerk is done processing application
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    //customer signals app clerk that customer is leaving now, telling clerk can get ready for next customer
        appClerkLock[myLine]->Release();
        //Application Done...
        //update money and myLine
        customers[customerIndex].myMoney = myMoney;
        customers[customerIndex].myLine = myLine;

        //customer check if there is senator coming into the office
        senatorLock->Acquire();
        printf("senatorCount:%d\n", senatorCount);
        if (senatorCount > 0){
         //   printf("there is a senator waiting, customer %d waiting...\n", customerIndex);
            waitingCustomer++;
            //customer_waiting.V();
            //printf("hello\n");
            customerWaitSenatorCV->Wait(senatorLock);
         //   printf("All senators have left, Customer %d is back in the office...\n", customerIndex);
        }
        senatorLock->Release();


        //Now go to Photo Clerk
        while(true){
            photoClerkLineLock->Acquire();
            //printf("Customer_%d just left Application Clerk, now going to the Photo Clerk!\n", customerIndex);
            for (int i = 0; i < numPhotoClerk; i++){
                peopleInPhotoClerk[i] = photoClerkLineCount[i] + photoClerkBribeLineCount[i];
            }
            myLine = 0;
            leastPeopleInPhoto = peopleInPhotoClerk[0];
            for (int i = 0; i < numPhotoClerk; i++){
                if((peopleInPhotoClerk[i] < leastPeopleInPhoto) && (photoClerkState[i] != NOTAVAILABLE) && (photoClerkState[i] != ONBREAK)){
                    myLine = i;
                    leastPeopleInPhoto = peopleInPhotoClerk[i];
                }
            }

            if (photoClerkState[myLine] == AVAILABLE){
                photoClerkState[myLine] = BUSY;
            }
            if (myMoney > 500){
                myMoney = myMoney - 500;
                photoClerkBribeLineCount[myLine]++;     //customer increment line count it is in
                //printf("Customer_%d is waiting in bribe line %d of Photo department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, photoClerkLineCount[myLine], photoClerkBribeLineCount[myLine], myMoney);
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Customer %d has gotten in bribe line for PictureClerk %d.\n", customerIndex, myLine);
                }
                photoClerkBribeLineCV[myLine]->Wait(photoClerkLineLock);
                photoClerkBribeLineCount[myLine]--;     //customer decrement line count after waken up
            }
            else{
                photoClerkLineCount[myLine]++;  //customer increment line count it is in
                //printf("Customer_%d is waiting in regular line %d of Photo department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, photoClerkLineCount[myLine], photoClerkBribeLineCount[myLine], myMoney);
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Customer %d has gotten in regular line for PictureClerk %d.\n", customerIndex, myLine);
                }
                photoClerkLineCV[myLine]->Wait(photoClerkLineLock);   
                photoClerkLineCount[myLine]--;  //customer decrement line count after waken up
            }
            
            photoClerkLineLock->Release();
            //interaction between customer and photo clerk
            photoClerkLock[myLine]->Acquire();
            photoClerkCustSSN[myLine] = customerIndex;
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Customer %d has given SSN %d to PictureClerk %d\n", customerIndex, customerIndex, myLine);
            }
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);   //customer signals photo clerk that customer is ready to be serviced
            photoClerkCV[myLine]->Wait(photoClerkLock[myLine]);     //wait for photo clerk to respond that clerk is done taking photo
            bool likePicture;
            if (customer_likesPhoto[customerIndex] == true){
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Customer %d does like their picture from PictureClerk %d.\n",customerIndex, myLine);
                }
                likePicture = true;
            }
            else{
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Customer %d does not like their picture from PictureClerk %d.\n",customerIndex, myLine);
                }
                likePicture = false;
            }
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);   //customer signals photo clerk that customer is leaving now, telling clerk can get ready for next customer
            photoClerkLock[myLine]->Release();

            //customer check if there is senator coming into the office
            senatorLock->Acquire();
            if (senatorCount > 0){
              //  printf("there is a senator waiting, customer %d waiting...\n", customerIndex);
                waitingCustomer++;
                //customer_waiting.V();
                customerWaitSenatorCV->Wait(senatorLock);
            //    printf("All senators have left, Customer %d is back in the office...\n", customerIndex);
            }
            senatorLock->Release();

            if (likePicture == true){
                break;
            }
        }
        //Photo done...
        //update money and myLine
        customers[customerIndex].myMoney = myMoney;
        customers[customerIndex].myLine = myLine;

        //printf("Customer_%d is done with Application and Photo, going to Passport Clerk next.\n", customerIndex);
    }
    //go to Photo Clerk, then App Clerk
    else{
        //printf("Customer_%d sees Photo clerk line is shorter, go there!\n", customerIndex );
        
        appClerkLineLock->Release();
        while(true){
            if (photoClerkState[myLine] == AVAILABLE){
                photoClerkState[myLine] = BUSY;
            }
            if (myMoney > 500){
                myMoney = myMoney - 500;
                photoClerkBribeLineCount[myLine]++;     //customer increment line count it is in
                //printf("Customer_%d is waiting in bribe line %d of Photo department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, photoClerkLineCount[myLine], photoClerkBribeLineCount[myLine], myMoney);
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Customer %d has gotten in bribe line for PictureClerk %d.\n", customerIndex, myLine);
                }
                photoClerkBribeLineCV[myLine]->Wait(photoClerkLineLock);
                photoClerkBribeLineCount[myLine]--;     //customer decrement line count after waken up
            }
            else{
                photoClerkLineCount[myLine]++;  //customer increment line count it is in
                //printf("Customer_%d is waiting in regular line %d of Photo department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, photoClerkLineCount[myLine], photoClerkBribeLineCount[myLine], myMoney);
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Customer %d has gotten in regular line for PictureClerk %d.\n", customerIndex, myLine);
                }
                photoClerkLineCV[myLine]->Wait(photoClerkLineLock);   
                photoClerkLineCount[myLine]--;  //customer decrement line count after waken up
            }
            photoClerkLineLock->Release();

            //interaction between customer and photo clerk
            photoClerkLock[myLine]->Acquire();
            photoClerkCustSSN[myLine] = customerIndex;
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Customer %d has given SSN %d to PictureClerk %d\n", customerIndex, customerIndex, myLine);
            }
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);   //customer signals photo clerk that customer is ready to be serviced
            photoClerkCV[myLine]->Wait(photoClerkLock[myLine]);     //wait for photo clerk to respond that clerk is done taking photo
            bool likePicture;
            if (customer_likesPhoto[customerIndex] == true){
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Customer %d does like their picture from PictureClerk %d.\n",customerIndex, myLine);
                }
                likePicture = true;
            }
            else{
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Customer %d does not like their picture from PictureClerk %d.\n",customerIndex, myLine);
                }
                likePicture = false;
            }
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);   //customer signals photo clerk that customer is leaving now, telling clerk can get ready for next customer
            photoClerkLock[myLine]->Release();

            //customer check if there is senator coming into the office
            senatorLock->Acquire();
            //printf("senatorCount:%d\n", senatorCount);
            if (senatorCount > 0){
              //  printf("there is a senator waiting, customer %d waiting...\n", customerIndex);
                waitingCustomer++;
                //customer_waiting.V();
                customerWaitSenatorCV->Wait(senatorLock);
              //  printf("All senators have left, Customer %d is back in the office...\n", customerIndex);
            }
            senatorLock->Release();

            if (likePicture == true){
                break;
            }
            photoClerkLineLock->Acquire();
        }
        //Photo done...
        //update money and myLine
        customers[customerIndex].myMoney = myMoney;
        customers[customerIndex].myLine = myLine;

        //Now go to Application Clerk
        appClerkLineLock->Acquire();
        //printf("Customer %d just left Photo Clerk, now going to the Application Clerk!\n", customerIndex);
        for (int i = 0; i < numAppClerk; i++){
            peopleInAppClerk[i] = appClerkLineCount[i] + appClerkBribeLineCount[i];
        }
        myLine = 0;
        leastPeopleInApp = peopleInAppClerk[0];
        for (int i = 0; i < numAppClerk; i++){
            if((peopleInAppClerk[i] < leastPeopleInApp) && (appClerkState[i] != NOTAVAILABLE) && (appClerkState[i] != ONBREAK)){
                myLine = i;
                leastPeopleInApp = peopleInAppClerk[i];
            }
        }

        if (appClerkState[myLine] == AVAILABLE){
            appClerkState[myLine] = BUSY;
        }
        if (myMoney > 500){
            myMoney = myMoney - 500;
            appClerkBribeLineCount[myLine]++;     //customer increment line count it is in
            //printf("Customer_%d is waiting in bribe line %d of Application department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, appClerkLineCount[myLine], appClerkBribeLineCount[myLine], myMoney);
            if((test_chosen == 8) || (test_chosen == 1) || (test_chosen == 7)) {
                printf("Customer %d has gotten in bribe line for ApplicationClerk %d.\n", customerIndex, myLine);
            }
            appClerkBribeLineCV[myLine]->Wait(appClerkLineLock);
            appClerkBribeLineCount[myLine]--;   //customer decrement line count after waken up

            appClerkSSNLock[myLine]->Acquire();
            appClerkLineLock->Release();

            appClerkCustSSN[myLine] = customerIndex;
            appClerkSSNCV[myLine]->Signal(appClerkSSNLock[myLine]);
            appClerkSSNCV[myLine]->Wait(appClerkSSNLock[myLine]);

            appClerkLineLock->Acquire();
            appClerkSSNLock[myLine]->Release();
        }
        else{
            appClerkLineCount[myLine]++;  //customer increment line count it is in
            //printf("Customer_%d is waiting in regular line %d of Application department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, appClerkLineCount[myLine], appClerkBribeLineCount[myLine], myMoney);
            if((test_chosen == 8) || (test_chosen == 1) || (test_chosen == 7)) {
                printf("Customer %d has gotten in regular line for ApplicationClerk %d.\n", customerIndex, myLine);
            }
            appClerkLineCV[myLine]->Wait(appClerkLineLock);   
            appClerkLineCount[myLine]--;  //customer decrement line count after waken up
        }
        appClerkLineLock->Release();

        //interaction between customer and application clerk
        appClerkLock[myLine]->Acquire();
        appClerkCustSSN[myLine] = customerIndex;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Customer %d has given SSN %d to ApplicationClerk %d\n", customerIndex, customerIndex, myLine);
        }
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    //customer signals app clerk that customer is ready to be serviced
        appClerkCV[myLine]->Wait(appClerkLock[myLine]);      //wait for app clerk to respond clerk is done processing application
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    //customer signals app clerk that customer is leaving now, telling clerk can get ready for next customer
        appClerkLock[myLine]->Release();

        //printf("Customer %d done with Application Clerk\n", customerIndex);

        //customer check if there is senator coming into the office
        senatorLock->Acquire();
        if (senatorCount > 0){
      //      printf("there is a senator waiting, customer %d waiting...\n", customerIndex);

            waitingCustomer++;
            //customer_waiting.V();
            customerWaitSenatorCV->Wait(senatorLock);
           // printf("All senators have left, Customer %d is back in the office...\n", customerIndex);
        }
        senatorLock->Release();

        //app clerk done..
        //update money and myLine
        customers[customerIndex].myMoney = myMoney;
        customers[customerIndex].myLine = myLine;

        //printf("Customer_%d is done with Photo and Application, going to Passport Clerk next.\n", customerIndex);
    }
    //at this point, customer has finished submitting application and taking photos
    //now going to the passport clerk
    //find shortest line of passport clerk
    //if by the time passport clerk is ready to service customer but customer variables hasn't yet been updated by app clerk and photo clerk
    //go back to find shortest line and repeat the whole process again
    //perhaps..
    //while(true){..break}
    while(true){

        //customer check if there is senator coming into the office
        senatorLock->Acquire();
        if (senatorCount > 0){
        //    printf("there is a senator waiting, customer %d waiting...\n", customerIndex);
            waitingCustomer++;
            //customer_waiting.V();
            customerWaitSenatorCV->Wait(senatorLock);
          //  printf("All senators have left, Customer %d is back in the office...\n", customerIndex);
        }
        senatorLock->Release();


        passportClerkLineLock->Acquire();
        int peopleInPassportClerk[numPassportClerk];
        for (int i = 0; i < numPassportClerk; ++i){
            peopleInPassportClerk[i] = passportClerkLineCount[i] + passportClerkBribeLineCount[i];
        }
        int leastPeopleInPassport = peopleInPassportClerk[0];
        myLine = 0;
        for (int i = 0; i < numPassportClerk; i++){
            if((peopleInPassportClerk[i] < leastPeopleInPassport) && (passportClerkState[i] != NOTAVAILABLE) && (passportClerkState[i] != ONBREAK)){
                myLine = i;
                leastPeopleInPassport = peopleInPassportClerk[i];
            }
        }


        //after finding shortest line, get in line and wait till passport clerk thread signals customer thread
        if (passportClerkState[myLine] == AVAILABLE){
                passportClerkState[myLine] = BUSY;
        }
        if (myMoney > 500){
            myMoney = myMoney - 500;
            passportClerkBribeLineCount[myLine]++;     //customer increment line count it is in
            //printf("Customer_%d is waiting in bribe line %d of Passport department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, passportClerkLineCount[myLine], passportClerkBribeLineCount[myLine], myMoney);
            if((test_chosen == 8)  || (test_chosen == 5) || (test_chosen == 7)) {
                printf("Customer %d has gotten in bribe line for PassportClerk %d.\n", customerIndex, myLine);
            }
            passportClerkBribeLineCV[myLine]->Wait(passportClerkLineLock);
            passportClerkBribeLineCount[myLine]--;     //customer decrement line count after waken up
        }
        else{
            passportClerkLineCount[myLine]++;  //customer increment line count it is in
            //printf("Customer_%d is waiting in regular line %d of Passport department, with %d people in regular line and %d people in bribe line...$%d left\n", customerIndex, myLine, passportClerkLineCount[myLine], passportClerkBribeLineCount[myLine], myMoney);
            if((test_chosen == 8) || (test_chosen == 5) || (test_chosen == 7)) {
                printf("Customer %d has gotten in regular line for PassportClerk %d.\n", customerIndex, myLine);
            }
            passportClerkLineCV[myLine]->Wait(passportClerkLineLock);   
            passportClerkLineCount[myLine]--;  //customer decrement line count after waken up
        }
        passportClerkLineLock->Release();

        //update money and myLine
        customers[customerIndex].myMoney = myMoney;
        customers[customerIndex].myLine = myLine;

        //interaction between customer and passport clerk
        passportClerkLock[myLine]->Acquire();
        passportClerkCustSSN[myLine] = customerIndex;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Customer %d has given SSN %d to PassportClerk %d\n", customerIndex, customerIndex, myLine);
        }
        passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);    //customer signals passport clerk that customer is ready to be serviced
        passportClerkCV[myLine]->Wait(passportClerkLock[myLine]);      //wait for passport clerk to respond clerk is done processing application
        if(!(customer_apps[customerIndex]==true && customer_photos[customerIndex]==true)){
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Customer %d has gone to PassportClerk %d too soon. They are going to the back of the line.\n", customerIndex, myLine);
            }
            int yieldCount = rand()%900+100;      
            for(int i = 0; i < yieldCount; i++){    
                currentThread->Yield();             //being punished to yield 100 to 1000 times
            }
            passportClerkLock[myLine]->Release();
            continue;
        }
        passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);    //customer signals passport clerk that customer is leaving now, telling clerk can get ready for next customer
        passportClerkLock[myLine]->Release();
        //passport clerk done..
        //update money and myLine
        customers[customerIndex].myMoney = myMoney;
        customers[customerIndex].myLine = myLine;
        //printf("Customer_%d is done with passport Clerk, now going to cashier to pay fees.\n", customerIndex);
        break;
    }
    //customer finished visiting passport clerk


    //customer check if there is senator coming into the office
    senatorLock->Acquire();
    if (senatorCount > 0){
      //  printf("there is a senator waiting, customer %d waiting...\n", customerIndex);
        waitingCustomer++;
        //customer_waiting.V();
        customerWaitSenatorCV->Wait(senatorLock);
     //   printf("All senators have left, Customer %d is back in the office...\n", customerIndex);
    }
    senatorLock->Release();
    

    //customer finished visiting passport clerk
    //now customer should head to the cashier
    //find the shortest line of the cashier
    //if by the time cashier is ready to service customer but customer variables hasn't yet been updated by the passport clerk
    //go back to find shortest line and repeat the whole process again
    //perhaps....
    //while(true){..break}
    while(true){


        //customer check if there is senator coming into the office
        senatorLock->Acquire();
        if (senatorCount > 0){
          //  printf("there is a senator waiting, customer %d waiting...\n", customerIndex);
            waitingCustomer++;
            //customer_waiting.V();
            customerWaitSenatorCV->Wait(senatorLock);
          //  printf("All senators have left, Customer %d is back in the office...\n", customerIndex);
        }
        senatorLock->Release();

        cashierLineLock->Acquire();
        int leastPeopleInCashier = cashierLineCount[0];
        myLine = 0;
        for (int i = 0; i < numCashier; i++){
            if((cashierLineCount[i] < leastPeopleInCashier) && (cashierState[i] != NOTAVAILABLE) && (cashierState[i] != ONBREAK)){
                myLine = i;
                leastPeopleInCashier = cashierLineCount[i];
            }
        }


        //after finding shortest line, get in line and wait till cashier thread signals customer thread
        if (cashierState[myLine] == AVAILABLE){
                cashierState[myLine] = BUSY;
        }
        //no bribe line in casheir
        cashierLineCount[myLine]++;  //customer increment line count it is in
        //printf("Customer_%d is waiting in line %d of Cashier , with %d people in line...$%d left\n", customerIndex, myLine, cashierLineCount[myLine], myMoney);
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Customer %d has gotten in regular line for Cashier %d.\n", customerIndex, myLine);
        }
        cashierLineCV[myLine]->Wait(cashierLineLock);   
        cashierLineCount[myLine]--;  //customer decrement line count after waken up
        cashierLineLock->Release();
        
        //update money and myLine
        customers[customerIndex].myMoney = myMoney;
        customers[customerIndex].myLine = myLine;

        
    
    
        //interaction between customer and cashier
        cashierLock[myLine]->Acquire();
        cashierCustSSN[myLine] = customerIndex;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Customer %d has given SSN %d to Cashier %d\n", customerIndex, customerIndex, myLine);
        }
        cashierCV[myLine]->Signal(cashierLock[myLine]);    //customer signals cashier that customer is ready to be serviced
        
        
        cashierCV[myLine]->Wait(cashierLock[myLine]);      //wait for cashier to respond cashier is done collecting fees
        if (!(customer_passport[customerIndex]==true)){
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Customer %d has gone to Cashier %d too soon. They are going to the back of the line.\n", customerIndex, myLine);
            }
            cashierLineLock->Release();
            int yieldCount = rand()%900+100;      
            for(int i = 0; i < yieldCount; i++){    
                currentThread->Yield();             //being punished to yield 100 to 1000 times
            }
            continue;
        }
        myMoney = myMoney - 100;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Customer %d has given Cashier %d $100.\n", customerIndex, myLine);
        }
        cashierCV[myLine]->Signal(cashierLock[myLine]);    //customer signals cashier that customer is leaving now, telling cashier can get ready for next customer
        cashierLock[myLine]->Release();
        //cashier done..
        //update money and myLine
        customers[customerIndex].myMoney = myMoney;
        customers[customerIndex].myLine = myLine;
        //printf("Customer_%d is done with Cashier, whole passport application done\n", customerIndex);
        //customer finished visiting cashier

        //now customer is finished the whole applying for passport process
        if((test_chosen == 8) || (test_chosen == 3) || (test_chosen == 7)) {
            printf("Customer %d is leaving the Passport Office\n", customerIndex);
        }
        //leaving the office....
        break;
    }
    p2_done.V();
}

//SENATOR
void Senator(int senatorIndex) {

    
    senatorLock->Acquire();
  //  printf("Senator: entered the passport office, waiting on manager to signal them.\n");
    //increment senator number
    printf("Senator %d coming in\n", senatorIndex);
    senatorCount = senatorCount + 1;
    //wait till passport office is empty, manager will signal senator
    senatorCV->Wait(senatorLock);
    senatorLock->Release();
    
    int myLine = senators[senatorIndex].myLine;
    int myMoney = senators[senatorIndex].myMoney;
    //initialize successful
    //printf("Customer_%d has arrived the passport office, carrying $%d\n", senatorIndex, myMoney);
    
    appClerkLineLock->Acquire();
    photoClerkLineLock->Acquire();
    int peopleInAppClerk[numAppClerk];
    for (int i = 0; i < numAppClerk; i++){
        peopleInAppClerk[i] = appClerkLineCount[i];
    }
    int peopleInPhotoClerk[numPhotoClerk];
    for (int i = 0; i < numPhotoClerk; i++){
        peopleInPhotoClerk[i] = photoClerkLineCount[i];
    }

    //find shorter line among app clerk and photo clerk
    int leastPeopleInApp = peopleInAppClerk[0];    //number of people in the shortest line in App
    int myAppLine = 0;
    for (int i = 0; i < numAppClerk; i++){
        if((peopleInAppClerk[i] < leastPeopleInApp) && (appClerkState[i] != NOTAVAILABLE) && (appClerkState[i] != ONBREAK)){
            myAppLine = i;
            leastPeopleInApp = peopleInAppClerk[i];
        }
    }
    //printf("shortest line in App: %d, with %d people..\n", myAppLine, leastPeopleInApp);
    int leastPeopleInPhoto = peopleInPhotoClerk[0];    //number of people in the shortest line in Photo
    int myPhotoLine = 0;
    for (int i = 0; i < numPhotoClerk; i++){
        if((peopleInPhotoClerk[i] < leastPeopleInPhoto) && (photoClerkState[i] != NOTAVAILABLE) && (photoClerkState[i] != ONBREAK)){
            myPhotoLine = i;
            leastPeopleInPhoto = peopleInPhotoClerk[i];
        }
    }
    //printf("shortest line in Photo: %d, with %d people..\n", myPhotoLine, leastPeopleInPhoto);
    int leastPeople = min(leastPeopleInPhoto, leastPeopleInApp);
    myLine = (leastPeopleInApp > leastPeopleInPhoto)? myPhotoLine:myAppLine;

    //go to Application Clerk, then Photo Clerk
    if(leastPeopleInPhoto >= leastPeopleInApp){
        //printf("Customer_%d sees application clerk line is shorter, go there!\n", senatorIndex );
        photoClerkLineLock->Release();
        if (appClerkState[myLine] == AVAILABLE){
            appClerkState[myLine] = BUSY;
        }

        appClerkLineCount[myLine]++;    //customer increment line count it is in
        //printf("Customer_%d is waiting in regular line %d of Application department, with %d people in regular line and %d people in bribe line...$%d left\n", senatorIndex, myLine, appClerkLineCount[myLine], appClerkBribeLineCount[myLine], myMoney);
        if((test_chosen == 8) || (test_chosen == 1) || (test_chosen == 7)) {
            printf("Senator %d has gotten in regular line for ApplicationClerk %d.\n", senatorIndex, myLine);
        }
        appClerkLineCV[myLine]->Wait(appClerkLineLock);
        appClerkLineCount[myLine]--;    //customer decrement line count after waken up
    
        appClerkLineLock->Release();

        //interaction between customer and application clerk
        appClerkLock[myLine]->Acquire();
        appClerkCustSSN[myLine] = senatorIndex;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Senator %d has given SSN %d to ApplicationClerk %d\n", senatorIndex, senatorIndex, myLine);
        }
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    //customer signals app clerk that customer is ready to be serviced
        appClerkCV[myLine]->Wait(appClerkLock[myLine]);      //wait for app clerk to respond clerk is done processing application
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    //customer signals app clerk that customer is leaving now, telling clerk can get ready for next customer
        appClerkLock[myLine]->Release();
        //Application Done...
        //update money and myLine
        senators[senatorIndex].myMoney = myMoney;
        senators[senatorIndex].myLine = myLine;

        //Now go to Photo Clerk
        while(true){
            photoClerkLineLock->Acquire();
            //printf("Customer_%d just left Application Clerk, now going to the Photo Clerk!\n", senatorIndex);
            for (int i = 0; i < numPhotoClerk; i++){
                peopleInPhotoClerk[i] = photoClerkLineCount[i];
            }
            myLine = 0;
            leastPeopleInPhoto = peopleInPhotoClerk[0];
            for (int i = 0; i < numPhotoClerk; i++){
                if((peopleInPhotoClerk[i] < leastPeopleInPhoto) && (photoClerkState[i] != NOTAVAILABLE) && (photoClerkState[i] != ONBREAK)){
                    myLine = i;
                    leastPeopleInPhoto = peopleInPhotoClerk[i];
                }
            }

            if (photoClerkState[myLine] == AVAILABLE){
                photoClerkState[myLine] = BUSY;
            }

            photoClerkLineCount[myLine]++;  //customer increment line count it is in
            //printf("Customer_%d is waiting in regular line %d of Photo department, with %d people in regular line and %d people in bribe line...$%d left\n", senatorIndex, myLine, photoClerkLineCount[myLine], photoClerkBribeLineCount[myLine], myMoney);
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Senator %d has gotten in regular line for PictureClerk %d.\n", senatorIndex, myLine);
            }
            photoClerkLineCV[myLine]->Wait(photoClerkLineLock);   
            photoClerkLineCount[myLine]--;  //customer decrement line count after waken up
            
            
            photoClerkLineLock->Release();
            //interaction between customer and photo clerk
            photoClerkLock[myLine]->Acquire();
            photoClerkCustSSN[myLine] = senatorIndex;
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Senator %d has given SSN %d to PictureClerk %d\n", senatorIndex, senatorIndex, myLine);
            }
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);   //customer signals photo clerk that customer is ready to be serviced
            photoClerkCV[myLine]->Wait(photoClerkLock[myLine]);     //wait for photo clerk to respond that clerk is done taking photo
            bool likePicture;
            if (customer_likesPhoto[senatorIndex] == true){
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Senator %d does like their picture from PictureClerk %d.\n",senatorIndex, myLine);
                }
                likePicture = true;
            }
            else{
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Senator %d does not like their picture from PictureClerk %d.\n",senatorIndex, myLine);
                }
                likePicture = false;
            }
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);   //customer signals photo clerk that customer is leaving now, telling clerk can get ready for next customer
            photoClerkLock[myLine]->Release();

            if (likePicture == true){
                break;
            }
        }
        //Photo done...
        //update money and myLine
        senators[senatorIndex].myMoney = myMoney;
        senators[senatorIndex].myLine = myLine;

        //printf("Customer_%d is done with Application and Photo, going to Passport Clerk next.\n", senatorIndex);
    }
    //go to Photo Clerk, then App Clerk
    else{
        //printf("Customer_%d sees Photo clerk line is shorter, go there!\n", senatorIndex );
        
        appClerkLineLock->Release();
        while(true){
            if (photoClerkState[myLine] == AVAILABLE){
                photoClerkState[myLine] = BUSY;
            }


            photoClerkLineCount[myLine]++;  //customer increment line count it is in
            //printf("Customer_%d is waiting in regular line %d of Photo department, with %d people in regular line and %d people in bribe line...$%d left\n", senatorIndex, myLine, photoClerkLineCount[myLine], photoClerkBribeLineCount[myLine], myMoney);
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Senator %d has gotten in regular line for PictureClerk %d.\n", senatorIndex, myLine);
            }
            photoClerkLineCV[myLine]->Wait(photoClerkLineLock);   
            photoClerkLineCount[myLine]--;  //customer decrement line count after waken up
            
            photoClerkLineLock->Release();

            //interaction between customer and photo clerk
            photoClerkLock[myLine]->Acquire();
            photoClerkCustSSN[myLine] = senatorIndex;
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Senator %d has given SSN %d to PictureClerk %d\n", senatorIndex, senatorIndex, myLine);
            }
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);   //customer signals photo clerk that customer is ready to be serviced
            photoClerkCV[myLine]->Wait(photoClerkLock[myLine]);     //wait for photo clerk to respond that clerk is done taking photo
            bool likePicture;
            if (customer_likesPhoto[senatorIndex] == true){
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Senator %d does like their picture from PictureClerk %d.\n",senatorIndex, myLine);
                }
                likePicture = true;
            }
            else{
                if((test_chosen == 8) || (test_chosen == 7)) {
                    printf("Senator %d does not like their picture from PictureClerk %d.\n",senatorIndex, myLine);
                }
                likePicture = false;
            }
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);   //customer signals photo clerk that customer is leaving now, telling clerk can get ready for next customer
            photoClerkLock[myLine]->Release();

            if (likePicture == true){
                break;
            }
            photoClerkLineLock->Acquire();
        }
        //Photo done...
        //update money and myLine
        senators[senatorIndex].myMoney = myMoney;
        senators[senatorIndex].myLine = myLine;

        //Now go to Application Clerk
        appClerkLineLock->Acquire();
        //printf("Customer %d just left Photo Clerk, now going to the Application Clerk!\n", senatorIndex);
        for (int i = 0; i < numAppClerk; i++){
            peopleInAppClerk[i] = appClerkLineCount[i];
        }
        myLine = 0;
        leastPeopleInApp = peopleInAppClerk[0];
        for (int i = 0; i < numAppClerk; i++){
            if((peopleInAppClerk[i] < leastPeopleInApp) && (appClerkState[i] != NOTAVAILABLE) && (appClerkState[i] != ONBREAK)){
                myLine = i;
                leastPeopleInApp = peopleInAppClerk[i];
            }
        }

        if (appClerkState[myLine] == AVAILABLE){
            appClerkState[myLine] = BUSY;
        }

        appClerkLineCount[myLine]++;  //customer increment line count it is in
        //printf("Customer_%d is waiting in regular line %d of Application department, with %d people in regular line and %d people in bribe line...$%d left\n", senatorIndex, myLine, appClerkLineCount[myLine], appClerkBribeLineCount[myLine], myMoney);
        if((test_chosen == 8) || (test_chosen == 1) || (test_chosen == 7)) {
            printf("Senator %d has gotten in regular line for ApplicationClerk %d.\n", senatorIndex, myLine);
        }
        appClerkLineCV[myLine]->Wait(appClerkLineLock);   
        appClerkLineCount[myLine]--;  //customer decrement line count after waken up
        
        appClerkLineLock->Release();

        //interaction between customer and application clerk
        appClerkLock[myLine]->Acquire();
        appClerkCustSSN[myLine] = senatorIndex;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Senator %d has given SSN %d to ApplicationClerk %d\n", senatorIndex, senatorIndex, myLine);
        }
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    //customer signals app clerk that customer is ready to be serviced
        appClerkCV[myLine]->Wait(appClerkLock[myLine]);      //wait for app clerk to respond clerk is done processing application
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    //customer signals app clerk that customer is leaving now, telling clerk can get ready for next customer
        appClerkLock[myLine]->Release();
        //app clerk done..
        //update money and myLine
        senators[senatorIndex].myMoney = myMoney;
        senators[senatorIndex].myLine = myLine;

        //printf("Customer_%d is done with Photo and Application, going to Passport Clerk next.\n", senatorIndex);
    }
    //at this point, customer has finished submitting application and taking photos
    //now going to the passport clerk
    //find shortest line of passport clerk
    //if by the time passport clerk is ready to service customer but customer variables hasn't yet been updated by app clerk and photo clerk
    //go back to find shortest line and repeat the whole process again
    //perhaps..
    //while(true){..break}
    while(true){
        passportClerkLineLock->Acquire();
        int peopleInPassportClerk[numPassportClerk];
        for (int i = 0; i < numPassportClerk; ++i){
            peopleInPassportClerk[i] = passportClerkLineCount[i];
        }
        int leastPeopleInPassport = peopleInPassportClerk[0];
        myLine = 0;
        for (int i = 0; i < numPassportClerk; i++){
            if((peopleInPassportClerk[i] < leastPeopleInPassport) && (passportClerkState[i] != NOTAVAILABLE) && (passportClerkState[i] != ONBREAK)){
                myLine = i;
                leastPeopleInPassport = peopleInPassportClerk[i];
            }
        }


        //after finding shortest line, get in line and wait till passport clerk thread signals customer thread
        if (passportClerkState[myLine] == AVAILABLE){
                passportClerkState[myLine] = BUSY;
        }

        passportClerkLineCount[myLine]++;  //customer increment line count it is in
        //printf("Customer_%d is waiting in regular line %d of Passport department, with %d people in regular line and %d people in bribe line...$%d left\n", senatorIndex, myLine, passportClerkLineCount[myLine], passportClerkBribeLineCount[myLine], myMoney);
        if((test_chosen == 8) || (test_chosen == 5) || (test_chosen == 7)) {
            printf("Senator %d has gotten in regular line for PassportClerk %d.\n", senatorIndex, myLine);
            //printf("PassportClerk %d has %d people in line.\n", myLine, passportClerkLineCount[myLine]);
        }
        passportClerkLineCV[myLine]->Wait(passportClerkLineLock);   
        passportClerkLineCount[myLine]--;  //customer decrement line count after waken up
        
        passportClerkLineLock->Release();

        //update money and myLine
        senators[senatorIndex].myMoney = myMoney;
        senators[senatorIndex].myLine = myLine;

        //interaction between customer and passport clerk
        passportClerkLock[myLine]->Acquire();
        passportClerkCustSSN[myLine] = senatorIndex;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Senator %d has given SSN %d to PassportClerk %d\n", senatorIndex, senatorIndex, myLine);
        }
        passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);    //customer signals passport clerk that customer is ready to be serviced
        passportClerkCV[myLine]->Wait(passportClerkLock[myLine]);      //wait for passport clerk to respond clerk is done processing application
        if(!(customer_apps[senatorIndex]==true && customer_photos[senatorIndex]==true)){
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Senator %d has gone to PassportClerk %d too soon. They are going to the back of the line.\n", senatorIndex, myLine);
            }
            int yieldCount = rand()%900+100;      
            for(int i = 0; i < yieldCount; i++){    
                currentThread->Yield();             //being punished to yield 100 to 1000 times
            }
            passportClerkLock[myLine]->Release();
            continue;
        }
        passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);    //customer signals passport clerk that customer is leaving now, telling clerk can get ready for next customer
        passportClerkLock[myLine]->Release();
        //passport clerk done..
        //update money and myLine
        senators[senatorIndex].myMoney = myMoney;
        senators[senatorIndex].myLine = myLine;
        //printf("Customer_%d is done with passport Clerk, now going to cashier to pay fees.\n", senatorIndex);
        break;
    }
    //customer finished visiting passport clerk

    

    //customer finished visiting passport clerk
    //now customer should head to the cashier
    //find the shortest line of the cashier
    //if by the time cashier is ready to service customer but customer variables hasn't yet been updated by the passport clerk
    //go back to find shortest line and repeat the whole process again
    //perhaps....
    //while(true){..break}
    while(true){
        cashierLineLock->Acquire();
        int leastPeopleInCashier = cashierLineCount[0];
        myLine = 0;
        for (int i = 0; i < numCashier; i++){
            if((cashierLineCount[i] < leastPeopleInCashier) && (cashierState[i] != NOTAVAILABLE) && (cashierState[i] != ONBREAK)){
                myLine = i;
                leastPeopleInCashier = cashierLineCount[i];
            }
        }


        //after finding shortest line, get in line and wait till cashier thread signals customer thread
        if (cashierState[myLine] == AVAILABLE){
                cashierState[myLine] = BUSY;
        }
        //no bribe line in casheir
        cashierLineCount[myLine]++;  //customer increment line count it is in
        //printf("Customer_%d is waiting in line %d of Cashier , with %d people in line...$%d left\n", senatorIndex, myLine, cashierLineCount[myLine], myMoney);
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Senator %d has gotten in regular line for Cashier %d.\n", senatorIndex, myLine);
        }
        cashierLineCV[myLine]->Wait(cashierLineLock);   
        cashierLineCount[myLine]--;  //customer decrement line count after waken up
        cashierLineLock->Release();
        
        //update money and myLine
        senators[senatorIndex].myMoney = myMoney;
        senators[senatorIndex].myLine = myLine;

        
    
    
        //interaction between customer and cashier
        cashierLock[myLine]->Acquire();
        cashierCustSSN[myLine] = senatorIndex;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Senator %d has given SSN %d to Cashier %d\n", senatorIndex, senatorIndex, myLine);
        }
        cashierCV[myLine]->Signal(cashierLock[myLine]);    //customer signals cashier that customer is ready to be serviced
        
        
        cashierCV[myLine]->Wait(cashierLock[myLine]);      //wait for cashier to respond cashier is done collecting fees
        if (!(customer_passport[senatorIndex]==true)){
            if((test_chosen == 8) || (test_chosen == 7)) {
                printf("Senator %d has gone to Cashier %d too soon. They are going to the back of the line.\n", senatorIndex, myLine);
            }
            cashierLineLock->Release();
            int yieldCount = rand()%900+100;      
            for(int i = 0; i < yieldCount; i++){    
                currentThread->Yield();             //being punished to yield 100 to 1000 times
            }
            continue;
        }
        myMoney = myMoney - 100;
        if((test_chosen == 8) || (test_chosen == 7)) {
            printf("Senator %d has given Cashier %d $100.\n", senatorIndex, myLine);
        }
        cashierCV[myLine]->Signal(cashierLock[myLine]);    //customer signals cashier that customer is leaving now, telling cashier can get ready for next customer
        cashierLock[myLine]->Release();
        //cashier done..
        //update money and myLine
        senators[senatorIndex].myMoney = myMoney;
        senators[senatorIndex].myLine = myLine;
        //printf("Customer_%d is done with Cashier, whole passport application done\n", senatorIndex);
        //customer finished visiting cashier

        //now customer is finished the whole applying for passport process
        if((test_chosen == 8) || (test_chosen == 3) || (test_chosen == 7)) {
            printf("Senator %d is leaving the Passport Office\n", senatorIndex);
        }
        //leaving the office....
        break;
    }

    senatorLock->Acquire();
    //decrement senator amount
    senatorCount = senatorCount - 1;
    senatorLock->Release();

    p2_done.V();
}

void AppClerk(int appClerkIndex) {
    p2_done.V();
    int customerSSN;
    while(true) {
        appClerkLineLock->Acquire();
        
        if(appClerkBribeLineCount[appClerkIndex] > 0){
            
            appClerkBribeLineCV[appClerkIndex]->Signal(appClerkLineLock);
            if(test_chosen == 8) {
                printf("ApplicationClerk %d has signalled a Customer to come to their counter.\n", appClerkIndex);
                
            }
            appClerkState[appClerkIndex] = BUSY;

            appClerkSSNLock[appClerkIndex]->Acquire();
            appClerkLineLock->Release();

            appClerkSSNCV[appClerkIndex] -> Wait(appClerkSSNLock[appClerkIndex]);
            customerSSN = appClerkCustSSN[appClerkIndex];
            appClerkSSNCV[appClerkIndex] -> Signal(appClerkSSNLock[appClerkIndex]);

            appClerkLineLock->Acquire();
            appClerkSSNLock[appClerkIndex]->Release();


            appClerkMoneyLock[appClerkIndex] -> Acquire();
            appClerkMoney[appClerkIndex] += 500;
            if(test_chosen == 8) {
                printf("ApplicationClerk %d has received $500 from Customer %d\n", appClerkIndex, appClerkCustSSN[appClerkIndex]); 
                
            } 
            appClerkMoneyLock[appClerkIndex] -> Release();
            
        } else if(appClerkLineCount[appClerkIndex] > 0){
            appClerkLineCV[appClerkIndex]->Signal(appClerkLineLock);
            if(test_chosen == 8) {
                if (senatorInOffice){
                    printf("ApplicationClerk %d has signalled a Senator to come to their counter.\n", appClerkIndex);
                }
                else{
                    printf("ApplicationClerk %d has signalled a Customer to come to their counter.\n", appClerkIndex);
                }
                
            }
            appClerkState[appClerkIndex] = BUSY;
        } else {
            /*break code here*/
            //wait on a cv specific to me
            appClerkState[appClerkIndex] = ONBREAK;
            appClerkBreakLock->Acquire();
            appClerkLineLock->Release();
            if((test_chosen == 8) || (test_chosen == 4)) {
                printf("ApplicationClerk %d is going on break\n", appClerkIndex);
            }
            appClerkBreakCV[appClerkIndex]->Wait(appClerkBreakLock);
            if(test_chosen == 8) {
                printf("ApplicationClerk %d is going off break\n", appClerkIndex);
            }
            appClerkBreakLock->Release();

            continue;
            
        }
        //interaction between customer and application clerk
        appClerkLock[appClerkIndex]->Acquire();
        appClerkLineLock->Release();
        //Wait for customer to give me SSN.
        appClerkCV[appClerkIndex]->Wait(appClerkLock[appClerkIndex]);
        customerSSN = appClerkCustSSN[appClerkIndex];
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("ApplicationClerk %d has received SSN %d from Senator %d\n", appClerkIndex,customerSSN,customerSSN);
            }
            else{
                printf("ApplicationClerk %d has received SSN %d from Customer %d\n", appClerkIndex,customerSSN,customerSSN);
            }
            
        }
        //Approve of Customer Application
        appClerkCV[appClerkIndex]->Signal(appClerkLock[appClerkIndex]);
        //Wait for Customer to know their Application is completed.
        appClerkCV[appClerkIndex]->Wait(appClerkLock[appClerkIndex]);
        //and be ready to leave. after this customer does one last signal. this will allow clerk to loop around
        int randYieldTime = rand() % 100 + 20;
            for(int i = 0; i < randYieldTime; i++){
            currentThread->Yield();
        }
        
        customer_apps[customerSSN] = true;
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("ApplicationClerk %d has recorded a completed application for Senator %d\n", appClerkIndex, customerSSN);
            }
            else{
                printf("ApplicationClerk %d has recorded a completed application for Customer %d\n", appClerkIndex, customerSSN);
            }
            
        }
        appClerkLock[appClerkIndex]->Release(); //done with customer interaction
    }

    
}
void PhotoClerk(int photoClerkIndex) {

    //PhotoClerk code goes here
    p2_done.V();
    while(true) {
        photoClerkLineLock->Acquire();
        if(photoClerkBribeLineCount[photoClerkIndex] > 0){
            photoClerkBribeLineCV[photoClerkIndex]->Signal(photoClerkLineLock);
            if(test_chosen == 8) {
                printf("PictureClerk %d has signalled a Customer to come to their counter.\n", photoClerkIndex);
               
            }
            photoClerkState[photoClerkIndex] = BUSY;
            photoClerkMoneyLock[photoClerkIndex] -> Acquire();
            photoClerkMoney[photoClerkIndex] += 500;
            photoClerkMoneyLock[photoClerkIndex] -> Release();
        }else if(photoClerkLineCount[photoClerkIndex] > 0){
            photoClerkLineCV[photoClerkIndex]->Signal(photoClerkLineLock);
            if(test_chosen == 8) {
                if (senatorInOffice){
                     printf("PictureClerk %d has signalled a Senator to come to their counter.\n", photoClerkIndex);
                }
                else{
                     printf("PictureClerk %d has signalled a Customer to come to their counter.\n", photoClerkIndex);
                }
            }
            photoClerkState[photoClerkIndex] = BUSY;
        } else {
            /*break code here*/
            //wait on a cv specific to me
            photoClerkState[photoClerkIndex] = ONBREAK;
            photoClerkBreakLock->Acquire();
            photoClerkLineLock->Release();
            if((test_chosen == 8) || (test_chosen == 4)) {
                printf("PictureClerk %d is going on break\n", photoClerkIndex);
            }
            photoClerkBreakCV[photoClerkIndex]->Wait(photoClerkBreakLock);
            if(test_chosen == 8) {
                printf("PictureClerk %d is going off break\n", photoClerkIndex);
            }
            photoClerkBreakLock->Release();

            continue;

        }
        //interaction between customer and application clerk
        photoClerkLock[photoClerkIndex]->Acquire();
        photoClerkLineLock->Release();
        //Wait for customer to give me SSN.
        photoClerkCV[photoClerkIndex]->Wait(photoClerkLock[photoClerkIndex]);
        int customerSSN = photoClerkCustSSN[photoClerkIndex];
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("PictureClerk %d has received SSN %d from Senator %d\n", photoClerkIndex,customerSSN, customerSSN);
            }
            else{
                 printf("PictureClerk %d has received SSN %d from Customer %d\n", photoClerkIndex,customerSSN, customerSSN);
            }
            
        }
        
        //Approve of Customer Photo

        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("PictureClerk %d has taken a picture of Senator %d \n", photoClerkIndex, photoClerkCustSSN[photoClerkIndex]);
            }
            else{
                printf("PictureClerk %d has taken a picture of Customer %d \n", photoClerkIndex, photoClerkCustSSN[photoClerkIndex]);
            }
            
        }
        int pickyness = customers[customerSSN].pickyness;
        if (senatorInOffice){
            pickyness = senators[customerSSN].pickyness;
        }
        int toRetakePhoto = rand() %99 + 1;

        if (toRetakePhoto >= pickyness){
            customer_likesPhoto[customerSSN] = true;
            if(test_chosen == 8) {
                if (senatorInOffice){
                    printf("PictureClerk %d has been told that Senator %d does like their picture\n", photoClerkIndex, customerSSN);
                }
                else{
                    printf("PictureClerk %d has been told that Customer %d does like their picture\n", photoClerkIndex, customerSSN);
                }
                
            }
        }
        else{
            customer_likesPhoto[customerSSN] = false;
            if(test_chosen == 8) {
                if (senatorInOffice){
                    printf("PictureClerk %d has been told that Senator %d does not like their picture\n", photoClerkIndex, customerSSN);
                }
                else{
                    printf("PictureClerk %d has been told that Customer %d does not like their picture\n", photoClerkIndex, customerSSN);
                }
                
            }

        }
        //signals customer photo has been taken
        photoClerkCV[photoClerkIndex]->Signal(photoClerkLock[photoClerkIndex]);

        //Wait for Customer to see they like picture
        photoClerkCV[photoClerkIndex]->Wait(photoClerkLock[photoClerkIndex]);

        if (customer_likesPhoto[customerSSN]){
            int randYieldTime = rand() % 100 + 20;
                for(int i = 0; i < randYieldTime; i++){
                currentThread->Yield();
            }
            customer_photos[customerSSN] = true;
        }

        
        photoClerkLock[photoClerkIndex]->Release(); //done with customer interaction
    }

}
void PassportClerk(int passportClerkIndex) {
    //PassportClerk code goes here
    p2_done.V();
    while(true) {
        passportClerkLineLock->Acquire();
        
        if(passportClerkBribeLineCount[passportClerkIndex] > 0){
            passportClerkBribeLineCV[passportClerkIndex]->Signal(passportClerkLineLock);
            //take money
            passportClerkMoneyLock[passportClerkIndex] -> Acquire();
            passportClerkMoney[passportClerkIndex] += 500;
            passportClerkMoneyLock[passportClerkIndex] -> Release();

            if(test_chosen == 8) {
                printf("PassportClerk %d has signalled a Customer to come to their counter.\n", passportClerkIndex);
            }
            passportClerkState[passportClerkIndex] = BUSY;
            
        } else if(passportClerkLineCount[passportClerkIndex] > 0){
            passportClerkLineCV[passportClerkIndex]->Signal(passportClerkLineLock);
            if(test_chosen == 8) {
                if (senatorInOffice>0){
                    printf("PassportClerk %d has signalled a Senator to come to their counter.\n", passportClerkIndex);
                }
                else{
                    printf("PassportClerk %d has signalled a Customer to come to their counter.\n", passportClerkIndex);
                }
            }
            passportClerkState[passportClerkIndex] = BUSY;
        } else {
            passportClerkState[passportClerkIndex] = ONBREAK;
            passportClerkBreakLock->Acquire();
            passportClerkLineLock->Release();
            if((test_chosen == 8) || (test_chosen == 4) || (test_chosen == 5)) {
                printf("PassportClerk %d is going on break\n", passportClerkIndex);
            }
            passportClerkBreakCV[passportClerkIndex]->Wait(passportClerkBreakLock);
            if((test_chosen == 8) || (test_chosen == 5)) {
                printf("PassportClerk %d is going off break\n", passportClerkIndex);
            }
            passportClerkBreakLock->Release();
            
            continue;
            
        }


        //interaction between customer and passport clerk
        passportClerkLock[passportClerkIndex]->Acquire();
        passportClerkLineLock->Release();
        //Wait for customer to give me SSN.
        passportClerkCV[passportClerkIndex]->Wait(passportClerkLock[passportClerkIndex]);
        int customerSSN = passportClerkCustSSN[passportClerkIndex];
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("PassportClerk %d has received SSN %d from Senator %d\n", passportClerkIndex,customerSSN,customerSSN);
            }
            else{
                printf("PassportClerk %d has received SSN %d from Customer %d\n", passportClerkIndex,customerSSN,customerSSN);
            }
            
        }
        //Approve of Customer Passport Application
        if (!(customer_apps[customerSSN]==true && customer_photos[customerSSN]==true)){
            //customer came too soon, kick him back in line and get next customer
            if(test_chosen == 8) {
                if (senatorInOffice){
                    printf("PassportClerk %d has determined that Senator %d does not have both their application and picture completed\n", passportClerkIndex, customerSSN);
                }
                else{
                    printf("PassportClerk %d has determined that Customer %d does not have both their application and picture completed\n", passportClerkIndex, customerSSN);
                }
                
            }
            passportClerkCV[passportClerkIndex]->Signal(passportClerkLock[passportClerkIndex]);
            passportClerkLock[passportClerkIndex]->Release();
            continue;
        }
        else {
            if(test_chosen == 8) {
                if (senatorInOffice){
                    printf("PassportClerk %d has determined that Senator %d has both their application and picture completed\n", passportClerkIndex, customerSSN);
                }
                else{
                    printf("PassportClerk %d has determined that Customer %d has both their application and picture completed\n", passportClerkIndex, customerSSN);
                }
                
            }
        }
        // if(test_chosen == 8) {
        //     if (senatorInOffice){
        //         printf("PassportClerk %d has recorded Senator %d passport documentation\n", passportClerkIndex, customerSSN);
        //     }
        //     else{
        //         printf("PassportClerk %d has recorded Customer %d passport documentation\n", passportClerkIndex, customerSSN);
        //     }

        // }
        passportClerkCV[passportClerkIndex]->Signal(passportClerkLock[passportClerkIndex]);
        //Wait for Customer to know their Passport Application is completed.
        passportClerkCV[passportClerkIndex]->Wait(passportClerkLock[passportClerkIndex]);
        //  printf("PassportClerk %d has received $500 from Customer[identifier]", customerIndex);
        //and be ready to leave. after this customer does one last signal. this will allow clerk to loop around
        int randYieldTime = rand() % 100 + 20;
            for(int i = 0; i < randYieldTime; i++){
            currentThread->Yield();
        }
        customer_passport[customerSSN] = true;
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("PassportClerk %d has recorded Customer %d passport documentation\n", passportClerkIndex, customerSSN);
            }
            else{
                printf("PassportClerk %d has recorded Customer %d passport documentation\n", passportClerkIndex, customerSSN);
            }
            
        }

        passportClerkLock[passportClerkIndex]->Release(); //done with customer interaction
    }
}
void Cashier(int cashierIndex) {
    //Cashier code goes here
    p2_done.V();
    while(true) {
        cashierLineLock->Acquire();
        
        if(cashierLineCount[cashierIndex] > 0){
            cashierLineCV[cashierIndex]->Signal(cashierLineLock);
            if(test_chosen == 8) {
                if (senatorInOffice){
                    printf("Cashier %d has signalled a Senator to come to their counter.\n", cashierIndex);
                }
                else{
                    printf("Cashier %d has signalled a Customer to come to their counter.\n", cashierIndex);
                }
                
            }
            cashierState[cashierIndex] = BUSY;
        } else {
            cashierState[cashierIndex] = ONBREAK;
            cashierBreakLock->Acquire();
            cashierLineLock->Release();
            if((test_chosen == 8) || (test_chosen == 4)) {

                printf("Cashier %d is going on break\n", cashierIndex);
            }
            cashierBreakCV[cashierIndex]->Wait(cashierBreakLock);
            if(test_chosen == 8) {
                printf("Cashier %d is going off break\n", cashierIndex);
            }
            cashierBreakLock->Release();
            

            continue;
            
        }
        //interaction between customer and cashier
        cashierLock[cashierIndex]->Acquire();
        cashierLineLock->Release();
        //Wait for customer to give me SSN.
        cashierCV[cashierIndex]->Wait(cashierLock[cashierIndex]);
        int customerSSN = cashierCustSSN[cashierIndex];
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("Cashier %d has received SSN %d from Senator %d\n", cashierIndex,customerSSN,customerSSN);
            }
            else{
                printf("Cashier %d has received SSN %d from Customer %d\n", cashierIndex,customerSSN,customerSSN);
            }
            
        }

        if (!(customer_passport[customerSSN]==true)){
            //customer came too soon, kick him back in line and get next customer
            if((test_chosen == 8) || (test_chosen == 6)) {
                if (senatorInOffice){
                    printf("Cashier %d has received the $100 from Senator %d before certification. They are to go to the back of my line.\n", cashierIndex, customerSSN);
                }
                else{
                    printf("Cashier %d has received the $100 from Customer %d before certification. They are to go to the back of my line.\n", cashierIndex, customerSSN);
                }
                
            }
            cashierCV[cashierIndex]->Signal(cashierLock[cashierIndex]);
            cashierLock[cashierIndex]->Release();
            continue;
        }
        //Approve of Customer Payments
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("Cashier %d has verified that Senator %d has been certified by a PassportClerk\n", cashierIndex, customerSSN);
            }
            else{
                printf("Cashier %d has verified that Customer %d has been certified by a PassportClerk\n", cashierIndex, customerSSN);
            }

        }
        int randYieldTime = rand() % 100 + 20;
        for(int i = 0; i < randYieldTime; i++){
            currentThread->Yield();
        }
        cashierMoneyLock[cashierIndex] -> Acquire();
        cashierMoney[cashierIndex] += 100;
        cashierMoneyLock[cashierIndex] -> Release();
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("Cashier %d has received the $100 from Senator %d after certification\n", cashierIndex, customerSSN);
            }
            else{
                printf("Cashier %d has received the $100 from Customer %d after certification\n", cashierIndex, customerSSN);
            }
            
        }
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("Cashier %d has provided Senator %d their completed passport\n", cashierIndex, customerSSN);
            }
            else{
                printf("Cashier %d has provided Customer %d their completed passport\n", cashierIndex, customerSSN);
            }
            
        }
        if((test_chosen == 8) || (test_chosen == 3)) {
            if (senatorInOffice){
                printf("Cashier %d has provided Senator %d their completed passport\n", cashierIndex, customerSSN);
            }
            else{
                printf("Cashier %d has provided Customer %d their completed passport\n", cashierIndex, customerSSN);
            }
            
        }
        cashierCV[cashierIndex]->Signal(cashierLock[cashierIndex]);
        //Wait for Customer to know their payment is completed.
        cashierCV[cashierIndex]->Wait(cashierLock[cashierIndex]);
        if(test_chosen == 8) {
            if (senatorInOffice){
                printf("Cashier %d has recorded that Senator %d has been given their completed passport\n", cashierIndex, customerSSN);
            }
            else{
                printf("Cashier %d has recorded that Customer %d has been given their completed passport\n", cashierIndex, customerSSN);
            }

        }
        //  printf("Cashier %d has received $100 from Customer[identifier]", customerIndex);
        //and be ready to leave. after this customer does one last signal. this will allow cashier to loop around
        
        cashierLock[cashierIndex]->Release(); //done with customer interaction
    }
}
void Manager() {
    //Manager code goes here
    
    senatorInOffice = false;
    while(true){
        //printf("Manager Thread.\n");//tbd
        //just to run manager thread on a regular basis...
        int yield = rand()%100+1000;
        for(int i = 0; i<yield; i++){
            currentThread->Yield();
        }


        int peopleInLine = 0;

        //checking if there is senator....
        if (senatorCount > 0 && senatorInOffice == false){

            //printf("Manager: there is a senator entering the office, will broadcast senator after all customer go on wait. \n");//tbd
            //printf("WaitingCustomer: %d\n", waitingCustomer);//tbd
            if (senatorInOffice == false && waitingCustomer == numCustomers){
                //printf("Manager: manager has woken up all senators.\n");//tbd
                senatorInOffice = true;
                senatorCV->Broadcast(senatorLock);

            }            
        }
        
      
        
        //printf("\nManager is monitoring the passport office...\n");
        //Check Application Clerks, make them go on break or back to work
        appClerkLineLock->Acquire();
        peopleInLine = 0;
        for (int i = 0; i < numAppClerk; i++){
            peopleInLine = appClerkLineCount[i] + appClerkBribeLineCount[i];
            if(appClerkState[i] == ONBREAK && peopleInLine>0){
                appClerkBreakLock->Acquire();
                if((test_chosen == 8) || (test_chosen == 4)) {
                    printf("Manager has woken up an ApplicationClerk\n");
                }
                appClerkState[i] = AVAILABLE;
                appClerkBreakCV[i]->Signal(appClerkBreakLock);
                appClerkBreakLock->Release();
            }
            if (peopleInLine > 3){
                for (int j = 0; j < numAppClerk; j++){
                    if (appClerkState[j] == ONBREAK){
                        appClerkBreakLock->Acquire();
                        if((test_chosen == 8) || (test_chosen == 4)) {
                            printf("Manager has woken up an ApplicationClerk\n");
                        }
                        appClerkState[j] = AVAILABLE;
                        appClerkBreakCV[j]->Signal(appClerkBreakLock);
                        appClerkBreakLock->Release();
                        break;
                    }
                }
            }
        }
        appClerkLineLock->Release();

        //Check Photo Clerks, make them go on break or back to work
        photoClerkLineLock->Acquire();
        peopleInLine = 0;
        for (int i = 0; i < numPhotoClerk; i++){
            peopleInLine = photoClerkLineCount[i] + photoClerkBribeLineCount[i];
            if(photoClerkState[i] == ONBREAK && peopleInLine>0){
                photoClerkBreakLock->Acquire();
                if((test_chosen == 8) || (test_chosen == 4)) {
                    printf("Manager has woken up a PictureClerk\n");
                }
                photoClerkState[i] = AVAILABLE;
                photoClerkBreakCV[i]->Signal(photoClerkBreakLock);
                photoClerkBreakLock->Release();
            }
            if (peopleInLine > 3){
                for (int j = 0; j < numPhotoClerk; j++){
                    if (photoClerkState[j] == ONBREAK){
                        photoClerkBreakLock->Acquire();
                        if((test_chosen == 8) || (test_chosen == 4)) {
                            printf("Manager has woken up a PictureClerk\n");
                        }
                        photoClerkState[j] = AVAILABLE;
                        photoClerkBreakCV[j]->Signal(photoClerkBreakLock);
                        photoClerkBreakLock->Release();
                        break;
                    }
                }
                
            }
        }
        photoClerkLineLock->Release();

        //Check passport Clerks, make them go on break or back to work
        passportClerkLineLock->Acquire();
        peopleInLine = 0;
        for (int i = 0; i < numPassportClerk; i++){
            peopleInLine = passportClerkLineCount[i] + passportClerkBribeLineCount[i];
            if(passportClerkState[i] == ONBREAK && peopleInLine>0){
                passportClerkBreakLock->Acquire();
                if((test_chosen == 8) || (test_chosen == 4)) {
                    printf("Manager has woken up a PassportClerk\n");
                }
                passportClerkState[i] = AVAILABLE;
                passportClerkBreakCV[i]->Signal(passportClerkBreakLock);
                passportClerkBreakLock->Release();
            }
            if (peopleInLine > 3){
                for (int j = 0; j < numPassportClerk; j++){
                    if (passportClerkState[j] == ONBREAK){
                        passportClerkBreakLock->Acquire();
                        if((test_chosen == 8) || (test_chosen == 4) || (test_chosen == 5)) {
                            printf("Manager has woken up a PassportClerk\n");
                        }
                        passportClerkState[j] = AVAILABLE;
                        passportClerkBreakCV[j]->Signal(passportClerkBreakLock);
                        passportClerkBreakLock->Release();
                        break;
                    }
                }
                
            }
        }
        passportClerkLineLock->Release();

        //Check Cashier, make them go on break or back to work
        cashierLineLock->Acquire();
        peopleInLine = 0;
        for (int i = 0; i < numCashier; i++){
            if(cashierState[i] == ONBREAK && cashierLineCount[i]>0){
                cashierBreakLock->Acquire();
                if((test_chosen == 8) || (test_chosen == 4)) {
                    printf("Manager has woken up a Cashier\n");
                }
                cashierState[i] = AVAILABLE;
                cashierBreakCV[i]->Signal(cashierBreakLock);
                cashierBreakLock->Release();
            }
            if (cashierLineCount[i] > 3){
                for (int j = 0; j < numCashier; j++){
                    if (cashierState[j] == ONBREAK){
                        cashierBreakLock->Acquire();
                        if((test_chosen == 8) || (test_chosen == 4)) {
                            printf("Manager has woken up a Cashier\n");
                        }
                        cashierState[j] = AVAILABLE;
                        cashierBreakCV[j]->Signal(cashierBreakLock);
                        cashierBreakLock->Release();
                        break;
                    }
                }
                
            }
        }
        cashierLineLock->Release();

        int totalAppClerkMoney = 0;
        for (int i = 0; i < numAppClerk; i++){
            appClerkMoneyLock[i]->Acquire();
            totalAppClerkMoney = totalAppClerkMoney + appClerkMoney[i];
            appClerkMoneyLock[i]->Release();
        }
        int totalPhotoClerkMoney = 0;
        for (int i = 0; i < numPhotoClerk; i++){
            photoClerkMoneyLock[i]->Acquire();
            totalPhotoClerkMoney = totalPhotoClerkMoney + photoClerkMoney[i];
            photoClerkMoneyLock[i]->Release();
        }
        int totalPassportClerkMoney = 0;
        for (int i = 0; i < numPassportClerk; i++){
            passportClerkMoneyLock[i]->Acquire();
            totalPassportClerkMoney = totalPassportClerkMoney + passportClerkMoney[i];
            passportClerkMoneyLock[i]->Release();
        }
        int totalCashierMoney = 0;
        for (int i = 0; i < numCashier; i++){
            cashierMoneyLock[i]->Acquire();
            totalCashierMoney = totalCashierMoney + cashierMoney[i];
            cashierMoneyLock[i]->Release();
        }

        //printing money collected from each types of clerk
        int totalMoneyMade = totalAppClerkMoney + totalPhotoClerkMoney + totalPassportClerkMoney + totalCashierMoney;
        if((test_chosen == 8) || (test_chosen == 2) || (test_chosen == 6)) {
            printf("Manager has counted a total of $%d for ApplicationClerks\n", totalAppClerkMoney);
            printf("Manager has counted a total of $%d for PictureClerks\n", totalPhotoClerkMoney);
            printf("Manager has counted a total of $%d for PassportClerks\n", totalPassportClerkMoney);
            printf("Manager has counted a total of $%d for Cashiers\n", totalCashierMoney);
            printf("Manager has counted a total of $%d for the passport office\n", totalMoneyMade);
        }

        if (senatorCount == 0 && senatorInOffice == true){
         //   printf("Manager: All senators have left gotten their passport and left the office.\n");
            customerWaitSenatorCV->Broadcast(senatorLock);
            waitingCustomer = 0;
            senatorInOffice=false;
        }

        //printf("totalCustomerMoney: $%d \n", totalCustomerMoney);
        if (totalMoneyMade == (totalCustomerMoney + totalSenatorMoney)){
            //printf("\nALL CUSTOMERS DONE!!!!\n\n");
            p2_done.V();
            break;
        }

    }




    
    
}
/*void Senator(int senatorIndex) {


}*/

//initilize threads for part 2
void initializeThreads(){
    Thread *t;
    char* name;
    init_customer_data();
    init_senator_data();
    init_locks_and_CV();
    for (int i = 0; i < numCustomers; ++i){
        name = new char[20];
        sprintf(name,"Customer_%d",i);
        //printf("Creating customer thread: %s\n", name);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr) Customer, i);
    }
    for (int i = 0; i < numAppClerk; ++i){
        name = new char[20];
        sprintf(name,"AppClerk_%d",i);
        //printf("Creating App Clerk thread: %s\n", name);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr) AppClerk, i);
    }
    for (int i = 0; i < numPhotoClerk; ++i){
        name = new char[20];
        sprintf(name,"PhotoClerk_%d",i);
        //printf("Creating Photo Clerk thread: %s\n", name);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr) PhotoClerk, i);
    }
    for (int i = 0; i < numPassportClerk; ++i){
        name = new char[20];
        sprintf(name,"passportClerk_%d",i);
        //printf("Creating Passport Clerk thread: %s\n", name);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr) PassportClerk, i);
    }
    for (int i = 0; i < numCashier; ++i){
        name = new char[20];
        sprintf(name,"Cashier_%d",i);
        //printf("Creating Cashier thread: %s\n", name);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr) Cashier, i);
    }
    for (int i = 0; i < numSenator; ++i){
        name = new char[20];
        sprintf(name,"Senator_%d",i);
        //printf("Creating Cashier thread: %s\n", name);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr) Senator, i);
    }
    t = new Thread("Manager");
    //printf("Creating Manager thread.\n");
    t->Fork((VoidFunctionPtr) Manager, 1);
    //printf("Creating Manager thread: Manager\n");

    
}

// --------------------------------------------------
void Problem2(){

    int i;
    Thread *t;
    srand(time(NULL));
    //printf("Starting Problem 2: \n\n");
   
    int menu_option = -1;
    bool all_well = true;
    
    printf("Please choose an option to test: \n");
    printf("1: Customer takes shortest line \n2: Managers only read from one clerk's money\n3: Customer doesn't leave without passport\n4: Clerks go on break appropriately\n5: Managers get clerk off break\n6: Total sales don't suffer from race condition\n7: Behaviour of customers when senators arrive\n8: System Test\n");
    scanf("%d", &menu_option);
    
    if(menu_option == 1) {
        printf("\nTest 1 Description: 'Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time'\n");
        test_chosen = 1;
        numCustomers = 10;
        numAppClerk = 2;
        numPhotoClerk = 2;
        numCashier = 2;
        numPassportClerk = 2;
        numCashier = 2;
        numSenator = 0;
    }
    else if(menu_option == 2) {
        printf("\nTest 2 Description: 'Managers only read one from one Clerk's total money received, at a time'\n");
        test_chosen = 2;
        numCustomers = 5;
        numAppClerk = 2;
        numPhotoClerk = 2;
        numCashier = 2;
        numPassportClerk = 2;
        numCashier = 2;
        numSenator = 0;
    }
    else if(menu_option == 3) {
        printf("\nTest 3 Description: 'Customers do not leave until they are given their passport by the Cashier. The Cashier does not start on another customer until they know that the last Customer has left their area'\n");
        test_chosen = 3;
        numCustomers = 10;
        numAppClerk = 2;
        numPhotoClerk = 2;
        numCashier = 2;
        numPassportClerk = 2;
        numCashier = 2;
        numSenator = 2;
    }
    else if(menu_option == 4) {
        printf("\nTest 4 Description: 'Clerks go on break when they have no one waiting in their line'\n");
        test_chosen = 4;
        numCustomers = 10;
        numAppClerk = 2;
        numPhotoClerk = 2;
        numCashier = 2;
        numPassportClerk = 2;
        numCashier = 2;
        numSenator = 0;
    }
    else if(menu_option == 5) {
        printf("\nTest 5 Description: 'Managers get Clerks off their break when lines get too long'\n");
        test_chosen = 5;
        numCustomers = 10;
        numAppClerk = 2;
        numPhotoClerk = 2;
        numCashier = 2;
        numPassportClerk = 2;
        numCashier = 2;
        numSenator = 0;
    }
    else if(menu_option == 6) {
        printf("\nTest 6 Description: 'Total sales never suffers from a race condition'\n");
        test_chosen = 6;
        numCustomers = 5;
        numAppClerk = 2;
        numPhotoClerk = 2;
        numCashier = 2;
        numPassportClerk = 2;
        numCashier = 2;
        numSenator = 0;
    }
    else if(menu_option == 7) {
        printf("\nTest 7 Description: 'The behavior of Customers is proper when Senators arrive. This is before, during, and after'\n");
        test_chosen = 7;
        numCustomers = 2;
        numAppClerk = 2;
        numPhotoClerk = 2;
        numCashier = 2;
        numPassportClerk = 2;
        numCashier = 2;
        numSenator = 2;
    }
    else if(menu_option == 8) {
        test_chosen = 8;
        printf("\nTest 8 Description: 'Select number of customers, clerks, manager, senators - and run everything'\n");
        printf("Enter number of Customers: ");
        scanf("%d", &numCustomers);
        printf("Enter number of ApplicationClerks: ");
        scanf("%d", &numAppClerk);
        printf("Enter number of PictureClerks: ");
        scanf("%d", &numPhotoClerk);
        printf("Enter number of PassportClerks: ");
        scanf("%d", &numPassportClerk);
        printf("Enter number of Cashiers: ");
        scanf("%d", &numCashier);
        printf("Enter number of Senators: ");
        scanf("%d", &numSenator);
        
        //bound checks
        if((numCustomers < 1) || (numCustomers > 50)) {
            printf("\nError: Check number of Customers entered - must be between 1 and 50 - restart Nachos\n");
            all_well = false;
        }
        if((numAppClerk < 1) || (numAppClerk > 5) || (numPhotoClerk < 1) || (numPhotoClerk > 5) || (numPassportClerk < 1) || (numPassportClerk > 5) || (numCashier < 1) || (numCashier > 5)) {
            printf("\nError: Check number of ApplicationClerk, PictureClerk, PassportClerk and/or Cashier - must be between 1 and 5 - restart Nachos\n");
            all_well = false;
        }
        if((numSenator < 0) || (numSenator > 10)) {
            printf("\nError: Check number of Senators entered - must be between 1 and 10 - restart Nachos\n");
            all_well = false;
        }
    }
    
    else {
        printf("\nError: Select appropriate menu option - restart Nachos\n");
        all_well = false;
    }

    if((test_chosen > 0) && (test_chosen < 8)) { //i.e. if menu_option != 8 (system test)
        //default values
        /*numCustomers = 10;
        numAppClerk = 2;
        numPhotoClerk = 2;
        numCashier = 2;
        numPassportClerk = 2;
        numCashier = 2;
        numSenator = 2;
        */
    }
    
    if(all_well) {
        printf("Number of Customers = %d\n", numCustomers);
        printf("Number of ApplicationClerks = %d\n", numAppClerk);
        printf("Number of PictureClerks = %d\n", numPhotoClerk);
        printf("Number of PassportClerks = %d\n", numPassportClerk);
        printf("Number of Cashiers = %d\n", numCashier);
        printf("Number of Senators = %d\n", numSenator);

        printf("\n//////////////////////////////////////////////////////////////////\n");
        printf("////////////////////Passport Office Simulation////////////////////\n");
        printf("//////////////////////////////////////////////////////////////////\n\n");

        initializeThreads();
        // wait for problem 2 to complete
        for (i = 0; i < numCustomers+numAppClerk+numPhotoClerk+numPassportClerk+numCashier+numSenator+1; i++){ //+1 Manager
            p2_done.P();
        }
    
        printf("\n//////////////////////////////////////////////////////////////////\n");
        printf("////////////////////Passport Office Simulation////////////////////\n");
        printf("//////////////////////////////////////////////////////////////////\n");
        printf("\n\n\n");
    }
}



// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//   4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    srand(time(NULL));
    Thread *t;
    char *name;
    int i;
    
    //  Passport Office Test - Problem 2
    //printf("\n\n\n");
    //printf("Starting Problem 2: \n\n");
    //Problem2();
    
    
    
    
    // Test 1
    
    printf("Starting Test 1\n");
    
    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);
    
    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);
    
    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);
    
    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
        t1_done.P();
    
    // Test 2
    
    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");
    
    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);
    
    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);
    
    // Wait for Test 2 to complete
    t2_done.P();
    
    // Test 3
    
    printf("Starting Test 3\n");
    
    for (  i = 0 ; i < 5 ; i++ ) {
        name = new char [20];
        sprintf(name,"t3_waiter%d",i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);
    
    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
        t3_done.P();
    
    // Test 4
    
    printf("Starting Test 4\n");
    
    for (  i = 0 ; i < 5 ; i++ ) {
        name = new char [20];
        sprintf(name,"t4_waiter%d",i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);
    
    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
        t4_done.P();
    
    // Test 5
    
    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");
    
    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);
    
    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);
    
}
#endif



