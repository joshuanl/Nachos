/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"
#define NUM 5
#define NUM_CUST 50
#define NUM_SENATOR 10
#define TRUE 1
#define FALSE 0

int test_chosen = -1;
int clerkStates[4] = {1, 2, 3, 4};
/*1:BUSY 2:AVAILABLE 3:ONBREAK 4:NOTAVAILABLE
/*Semaphore customer_waiting("customer_waiting",0);*/
int numCustomers = 0; 
int customerIndex = 0;
int customer_apps[NUM_CUST]= {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int customer_likesPhoto[NUM_CUST] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int customer_photos[NUM_CUST] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int customer_passport[NUM_CUST] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int customerPicky[NUM_CUST] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int totalCustomerMoney = 0;
int waitingCustomer = 0;
int customerMoney[NUM_CUST]; /* = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};*/
int customerLine[NUM_CUST]; /* = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};*/
int customerIndexLock;
/*Condition* customerWaitSenatorCV;*/

/*Application Clerk Variables*/
int appClerkState[NUM] = {2,2,2,2,2};
int numAppClerk = 0;
int appClerkNum = 0;
int appClerkLineCount[NUM] = {0,0,0,0,0};       
int appClerkBribeLineCount[NUM] = {0,0,0,0,0};  
int appClerkMoney[NUM] = {0,0,0,0,0};
int appClerkCustSSN[NUM]={-1,-1,-1,-1,-1};
int appClerkCustPhoto[NUM];
int appClerkCustApplication[NUM];
int appClerkIndexLock;
int appClerkLock[NUM];
int appClerkLineLock; 
int appClerkSSNLock[NUM];
int appClerkMoneyLock[NUM];
int appClerkSSNCV[NUM];
int appClerkLineCV[NUM];
int appClerkBribeLineCV[NUM];
int appClerkCV[NUM];


/*Photo Clerk Variables*/
int photoClerkState[NUM] = {2,2,2,2,2};
int numPhotoClerk = 0;
int photoClerkNum = 0;
int photoClerkLineCount[NUM] = {0,0,0,0,0};    
int photoClerkBribeLineCount[NUM] = {0,0,0,0,0};
int photoClerkMoney[NUM] = {0,0,0,0,0};
int photoClerkCustSSN[NUM]={-1,-1,-1,-1,-1};
int photoClerkCustPhoto[NUM];
int photoClerkCustApplication[NUM];
int photoClerkIndexLock;
int photoClerkLock[NUM];
int photoClerkLineLock; 
int photoClerkMoneyLock[NUM];
int photoClerkLineCV[NUM];
int photoClerkBribeLineCV[NUM];
int photoClerkCV[NUM];


/*Passport Clerk Variables*/
int passportClerkState[NUM] = {2,2,2,2,2};
int numPassportClerk = 0;
int passportClerkNum = 0;
int passportClerkMoney[NUM] = {0,0,0,0,0};
int passportClerkLineCount[NUM] = {0,0,0,0,0};
int passportClerkBribeLineCount[NUM] = {0,0,0,0,0};
int passportClerkCustSSN[NUM]={-1,-1,-1,-1,-1};
int passportClerkCustApplication[NUM];
int passportClerkCustPhoto[NUM];
int passportClerkIndexLock;
int passportClerkLock[NUM];
int passportClerkLineLock;
int passportClerkMoneyLock[NUM];
int passportClerkLineCV[NUM];
int passportClerkBribeLineCV[NUM];
int passportClerkCV[NUM];


/*Cashier Variables*/
int cashierState[NUM] = {2,2,2,2,2};
int numCashier = 0;
int cashierNum = 0;
int cashierLineCount[NUM] = {0,0,0,0,0};
int cashierMoney[NUM] = {0,0,0,0,0};
int cashierCustSSN[NUM]={-1,-1,-1,-1,-1};
int cashierCustPhoto[NUM];
int cashierCustApplication[NUM];
int cashierIndexLock;
int cashierLock[NUM];
int cashierLineLock; 
int cashierMoneyLock[NUM];
int cashierCV[NUM];
int cashierLineCV[NUM];


/*Manager variables – timer thread*/
int senatorInOffice = 0;
int appClerkBreakLock;
int photoClerkBreakLock;
int passportClerkBreakLock;
int cashierBreakLock;
int appClerkBreakCV[NUM];
int photoClerkBreakCV[NUM];
int passportClerkBreakCV[NUM];
int cashierBreakCV[NUM];


/*Senator variables*/
int numSenator = 0; 
int totalSenatorMoney = 0;
int senatorCount;
int senator_count = 0;
int senator_apps[NUM_SENATOR];
int senator_likesPhoto[NUM_SENATOR];
int senator_photos[NUM_SENATOR];
int senator_passport[NUM_SENATOR];
int senatorPicky[NUM_SENATOR] = {1,1,1,1,1,1,1,1,1,1};
/*Lock* senatorLock;
Condition* senatorCV;*/


/* --------------------------------------------------*/
/*Thread Code*/
/*Customer variables*/
struct customer {
    int myLine;
    int myMoney;
    int SSN;
    int pickyness;
    int isDone; 
} customers[NUM_CUST];

/*Senator variables*/
struct senator {
    int myLine;
    int myMoney;
    int SSN;
    int pickyness;
    int isDone; 
} senators[NUM_SENATOR];

void Customer() {
	
    int myLine = -1;        
    int myMoney = 1600;
    int pickyness = 50;
    int customerSSN = 0;
    int isDone = 0;
    int likePhoto = 0;
    int doneApp = 0;
    int donePhoto = 0;
    int donePassport = 0;
    int likePicture;

    int i;
    int peopleInAppClerk[numAppClerk];
    int peopleInPhotoClerk[numPhotoClerk];
    int peopleInPassportClerk[numPassportClerk];
    int leastPeopleInApp = 0;
    int leastPeopleInPhoto = 0;
    int leastPeopleInPassport = 0;
    int leastPeopleInCashier = 0;
    int myAppLine = 0;
    int myPhotoLine = 0;
    int leastPeople;
    
    Acquire(customerIndexLock);
    customerSSN = customerIndex;
    customerIndex++;
    customerMoney[customerSSN] = myMoney;
    customerLine[customerSSN] = myLine;
    Write("Making customer: ",sizeof("Making customer: "),ConsoleOutput);
    PrintInt(customerSSN);
    Write("\n",sizeof("\n"), ConsoleOutput);
    Release(customerIndexLock);

    Acquire(appClerkLineLock);
    Acquire(photoClerkLineLock);
	for (i = 0; i < numAppClerk; i++){
        peopleInAppClerk[i] = appClerkLineCount[i] + appClerkBribeLineCount[i];
    }
    for (i = 0; i < numPhotoClerk; i++){
        peopleInPhotoClerk[i] = photoClerkLineCount[i] + photoClerkBribeLineCount[i];
    }
    leastPeopleInApp = peopleInAppClerk[0];
    for (i = 0; i < numAppClerk; i++){
        if((peopleInAppClerk[i] < leastPeopleInApp) && (appClerkState[i] != 4) && (appClerkState[i] != 3)){
            myAppLine = i;
            leastPeopleInApp = peopleInAppClerk[i];
        }
    }
    leastPeopleInPhoto = peopleInPhotoClerk[0];
    for (i = 0; i < numPhotoClerk; i++){
        if((peopleInPhotoClerk[i] < leastPeopleInPhoto) && (photoClerkState[i] != 4) && (photoClerkState[i] != 3)){
            myPhotoLine = i;
            leastPeopleInPhoto = peopleInPhotoClerk[i];
        }
    }
    if (leastPeopleInPhoto >= leastPeopleInApp){
    	leastPeople = leastPeopleInApp;
    }
    else{
    	leastPeople = leastPeopleInPhoto;
    }
    myLine = (leastPeopleInApp > leastPeopleInPhoto)? myPhotoLine:myAppLine;

    if(leastPeopleInPhoto >= leastPeopleInApp){
    	Release(photoClerkLineLock);
    	if (appClerkState[myLine] == 2){
            appClerkState[myLine] = 1;
        }	
        if (myMoney > 500){
        	myMoney = myMoney - 500;
        	appClerkBribeLineCount[myLine]++;
        	Write("Customer ", sizeof("Customer "), ConsoleOutput);
        	PrintInt(customerSSN);
        	Write(" has gotten in bribe line for ApplicationClerk ", sizeof(" has gotten in bribe line for ApplicationClerk "), ConsoleOutput);
        	PrintInt(myLine);
    		Write("\n",sizeof("\n"), ConsoleOutput);
        	/*
        	appClerkBribeLineCV[myLine]->Wait(appClerkLineLock);
            */
            Wait(appClerkBribeLineCV[myLine], appClerkLineLock);

            appClerkBribeLineCount[myLine]--;   

            Acquire(appClerkSSNLock[myLine]);
            Release(appClerkLineLock);

            appClerkCustSSN[myLine] = customerSSN;

            /*
            appClerkSSNCV[myLine]->Signal(appClerkSSNLock[myLine]);
            appClerkSSNCV[myLine]->Wait(appClerkSSNLock[myLine]);
			*/
			Signal(appClerkSSNCV[myLine], appClerkSSNLock[myLine]);
			Wait(appClerkSSNCV[myLine], appClerkSSNLock[myLine]);
			
            Acquire(appClerkLineLock);
            Release(appClerkSSNLock[myLine]);
            

        }

    
	    else{
	    	appClerkLineCount[myLine]++;
	    	Write("Customer ", sizeof("Customer "), ConsoleOutput);
        	PrintInt(customerSSN);
        	Write(" has gotten in regular line for ApplicationClerk ", sizeof(" has gotten in regular line for ApplicationClerk "), ConsoleOutput);
        	PrintInt(myLine);
    		Write("\n",sizeof("\n"), ConsoleOutput);
	    	
	    	/*
	    	appClerkLineCV[myLine]->Wait(appClerkLineLock);
	        */
	        Wait(appClerkLineCV[myLine], appClerkLineLock);
	        appClerkLineCount[myLine]--;
	    } 
	    Release(appClerkLineLock);

	    Acquire(appClerkLock[myLine]);

	    appClerkCustSSN[myLine] = customerSSN;
	  
	    Write("Customer ", sizeof("Customer "), ConsoleOutput);
    	PrintInt(customerSSN);
    	Write(" has given SSN to ApplicationClerk ", sizeof(" has given SSN to ApplicationClerk "), ConsoleOutput);
    	PrintInt(myLine);
		Write("\n",sizeof("\n"), ConsoleOutput);

	    /*
	    appClerkCV[myLine]->Signal(appClerkLock[myLine]);    
	    appClerkCV[myLine]->Wait(appClerkLock[myLine]);      
	    appClerkCV[myLine]->Signal(appClerkLock[myLine]);    
	    */
	    Signal(appClerkCV[myLine],appClerkLock[myLine]);
	    Wait(appClerkCV[myLine],appClerkLock[myLine]);
	    Signal(appClerkCV[myLine],appClerkLock[myLine]);
	    
	    

	    customerMoney[customerSSN] = myMoney;
	    customerLine[customerSSN] = myLine;

	    Release(appClerkLock[myLine]);

	    while(1){
	    	Acquire(photoClerkLineLock);
	    	for (i = 0; i < numPhotoClerk; i++){
                peopleInPhotoClerk[i] = photoClerkLineCount[i] + photoClerkBribeLineCount[i];
            }
            myLine = 0;
            leastPeopleInPhoto = peopleInPhotoClerk[0];
            for (i = 0; i < numPhotoClerk; i++){
                if((peopleInPhotoClerk[i] < leastPeopleInPhoto) && (photoClerkState[i] != 4) && (photoClerkState[i] != 3)){
                    myLine = i;
                    leastPeopleInPhoto = peopleInPhotoClerk[i];
                }
            }
            if (photoClerkState[myLine] == 2){
                photoClerkState[myLine] = 1;
            }
            if (myMoney > 500){
                myMoney = myMoney - 500;
                photoClerkBribeLineCount[myLine]++;  
                Write("Customer ", sizeof("Customer "), ConsoleOutput);
	        	PrintInt(customerSSN);
	        	Write(" has gotten in bribe line for PhotoClerk ", sizeof(" has gotten in bribe line for PhotoClerk "), ConsoleOutput);
	        	PrintInt(myLine);
	    		Write("\n",sizeof("\n"), ConsoleOutput);
                /*
                photoClerkBribeLineCV[myLine]->Wait(photoClerkLineLock);
                */
                Wait(photoClerkBribeLineCV[myLine], photoClerkLineLock);
                photoClerkBribeLineCount[myLine]--;
                
            }
            else{
                photoClerkLineCount[myLine]++; 
                Write("Customer ", sizeof("Customer "), ConsoleOutput);
	        	PrintInt(customerSSN);
	        	Write(" has gotten in regular line for PhotoClerk ", sizeof(" has gotten in regular line for PhotoClerk "), ConsoleOutput);
	        	PrintInt(myLine);
	    		Write("\n",sizeof("\n"), ConsoleOutput);
                
                /*
                photoClerkLineCV[myLine]->Wait(photoClerkLineLock);   
                */
                Wait(photoClerkLineCV[myLine],photoClerkLineLock);
                photoClerkLineCount[myLine]--;  /*customer decrement line count after waken up*/
            }
            Release(photoClerkLineLock);

            Acquire(photoClerkLock[myLine]);
            photoClerkCustSSN[myLine] = customerSSN;
            
            Write("Customer ", sizeof("Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" has given SSN to PhotoClerk ", sizeof(" has given SSN to PhotoClerk "), ConsoleOutput);
	    	PrintInt(myLine);
			Write("\n",sizeof("\n"), ConsoleOutput);

            /*
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]); 
            photoClerkCV[myLine]->Wait(photoClerkLock[myLine]);
            */
            Signal(photoClerkCV[myLine], photoClerkLock[myLine]);
            Wait(photoClerkCV[myLine], photoClerkLock[myLine]);

            
            if (customer_likesPhoto[customerSSN] == 1){
                Write("Customer ", sizeof("Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does like their picture from PictureClerk ", sizeof(" does like their picture from PictureClerk "), ConsoleOutput);
		    	PrintInt(myLine);
				Write("\n",sizeof("\n"), ConsoleOutput);
                likePicture = 1;
            }
            else{
                Write("Customer ", sizeof("Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does NOT like their picture from PictureClerk ", sizeof(" does NOT like their picture from PictureClerk "), ConsoleOutput);
		    	PrintInt(myLine);
				Write("\n",sizeof("\n"), ConsoleOutput);
                likePicture = 0;
            }
            /*
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);
            */
            Signal(photoClerkCV[myLine],photoClerkLock[myLine]);
            Release(photoClerkLock[myLine]);

            if (likePicture == 1){
                break;
            }
	    }
	    customerMoney[customerSSN] = myMoney;
	    customerLine[customerSSN] = myLine;
	}
	else{
		Release(appClerkLineLock);
		while(1){
            if (photoClerkState[myLine] == 2){
                photoClerkState[myLine] = 1;
            }
            if (myMoney > 500){
                myMoney = myMoney - 500;
                photoClerkBribeLineCount[myLine]++;     
                Write("Customer ", sizeof("Customer "), ConsoleOutput);
	        	PrintInt(customerSSN);
	        	Write(" has gotten in bribe line for PhotoClerk ", sizeof(" has gotten in bribe line for PhotoClerk "), ConsoleOutput);
	        	PrintInt(myLine);
	    		Write("\n",sizeof("\n"), ConsoleOutput);
                /*
                photoClerkBribeLineCV[myLine]->Wait(photoClerkLineLock);
                */
                Wait(photoClerkBribeLineCV[myLine],photoClerkLineLock);
                photoClerkBribeLineCount[myLine]--;
            }
            else{
                photoClerkLineCount[myLine]++; 
                Write("Customer ", sizeof("Customer "), ConsoleOutput);
	        	PrintInt(customerSSN);
	        	Write(" has gotten in regular line for PhotoClerk ", sizeof(" has gotten in regular line for PhotoClerk "), ConsoleOutput);
	        	PrintInt(myLine);
	    		Write("\n",sizeof("\n"), ConsoleOutput);
                /*
                photoClerkLineCV[myLine]->Wait(photoClerkLineLock);   
                */
                Wait(photoClerkLineCV[myLine],photoClerkLineLock);
                photoClerkLineCount[myLine]--;
            }
            Release(photoClerkLineLock);

            Acquire(photoClerkLock[myLine]);
            photoClerkCustSSN[myLine] = customerSSN;
            Write("Customer ", sizeof("Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" has given SSN to PhotoClerk ", sizeof(" has given SSN to PhotoClerk "), ConsoleOutput);
	    	PrintInt(myLine);
			Write("\n",sizeof("\n"), ConsoleOutput);
            /*
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);   
            photoClerkCV[myLine]->Wait(photoClerkLock[myLine]);     
            */
            Signal(photoClerkCV[myLine],photoClerkLock[myLine]);
            Wait(photoClerkCV[myLine],photoClerkLock[myLine]);

            if (customer_likesPhoto[customerSSN] == 1){
                Write("Customer ", sizeof("Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does like their picture from PictureClerk ", sizeof(" does like their picture from PictureClerk "), ConsoleOutput);
		    	PrintInt(myLine);
				Write("\n",sizeof("\n"), ConsoleOutput);
                likePicture = 1;
            }
            else{
                Write("Customer ", sizeof("Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does NOT like their picture from PictureClerk ", sizeof(" does NOT like their picture from PictureClerk "), ConsoleOutput);
		    	PrintInt(myLine);
				Write("\n",sizeof("\n"), ConsoleOutput);
                likePicture = 0;
            }
            /*
            photoClerkCV[myLine]->Signal(photoClerkLock[myLine]);
            */
            Signal(photoClerkCV[myLine],photoClerkLock[myLine]);

            Release(photoClerkLock[myLine]);

            if (likePicture == 1){
                break;
            }
            Acquire(photoClerkLineLock);
        }
        customerMoney[customerSSN] = myMoney;
        customerLine[customerSSN] = myLine;

        Acquire(appClerkLineLock);
        for (i = 0; i < numAppClerk; i++){
            peopleInAppClerk[i] = appClerkLineCount[i] + appClerkBribeLineCount[i];
        }
        myLine = 0;
        leastPeopleInApp = peopleInAppClerk[0];
        for (i = 0; i < numAppClerk; i++){
            if((peopleInAppClerk[i] < leastPeopleInApp) && (appClerkState[i] != 4) && (appClerkState[i] != 3)){
                myLine = i;
                leastPeopleInApp = peopleInAppClerk[i];
            }
        }

        if (appClerkState[myLine] == 2){
            appClerkState[myLine] = 1;
        }
        if (myMoney > 500){
            myMoney = myMoney - 500;
            appClerkBribeLineCount[myLine]++; 
            Write("Customer ", sizeof("Customer "), ConsoleOutput);
        	PrintInt(customerSSN);
        	Write(" has gotten in bribe line for ApplicationClerk ", sizeof(" has gotten in bribe line for ApplicationClerk "), ConsoleOutput);
        	PrintInt(myLine);
    		Write("\n",sizeof("\n"), ConsoleOutput);
            /*
            appClerkBribeLineCV[myLine]->Wait(appClerkLineLock);
            */
            Wait(appClerkBribeLineCV[myLine],appClerkLineLock);
            appClerkBribeLineCount[myLine]--;   /*customer decrement line count after waken up*/

            Acquire(appClerkSSNLock[myLine]);
            Release(appClerkLineLock);

            appClerkCustSSN[myLine] = customerSSN;
            /*
            appClerkSSNCV[myLine]->Signal(appClerkSSNLock[myLine]);
            appClerkSSNCV[myLine]->Wait(appClerkSSNLock[myLine]);
			*/
			Signal(appClerkSSNCV[myLine],appClerkSSNLock[myLine]);
			Wait(appClerkSSNCV[myLine],appClerkSSNLock[myLine]);

            Acquire(appClerkLineLock);
            Release(appClerkSSNLock[myLine]);
        }
        else{
            appClerkLineCount[myLine]++;  /*customer increment line count it is in*/
            Write("Customer ", sizeof("Customer "), ConsoleOutput);
        	PrintInt(customerSSN);
        	Write(" has gotten in regular line for ApplicationClerk ", sizeof(" has gotten in regular line for ApplicationClerk "), ConsoleOutput);
        	PrintInt(myLine);
    		Write("\n",sizeof("\n"), ConsoleOutput);
            /*
            appClerkLineCV[myLine]->Wait(appClerkLineLock);   
            */
            Wait(appClerkLineCV[myLine],appClerkLineLock);

            appClerkLineCount[myLine]--;  /*customer decrement line count after waken up*/
        }
        Release(appClerkLineLock);

        Acquire(appClerkLock[myLine]);
        appClerkCustSSN[myLine] = customerSSN;
        Write("Customer ", sizeof("Customer "), ConsoleOutput);
    	PrintInt(customerSSN);
    	Write(" has given SSN to ApplicationClerk ", sizeof(" has given SSN to ApplicationClerk "), ConsoleOutput);
    	PrintInt(myLine);
		Write("\n",sizeof("\n"), ConsoleOutput);
        /*
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    
        appClerkCV[myLine]->Wait(appClerkLock[myLine]);      
        appClerkCV[myLine]->Signal(appClerkLock[myLine]);    
        */
        Signal(appClerkCV[myLine],appClerkLock[myLine]);
        Wait(appClerkCV[myLine],appClerkLock[myLine]);
        Signal(appClerkCV[myLine],appClerkLock[myLine]);

        customerMoney[customerSSN] = myMoney;
        customerLine[customerSSN] = myLine;
		Release(appClerkLock[myLine]);
	}

	while(1){

        Acquire(passportClerkLineLock);
        
        for (i = 0; i < numPassportClerk; ++i){
            peopleInPassportClerk[i] = passportClerkLineCount[i] + passportClerkBribeLineCount[i];
        }
        leastPeopleInPassport = peopleInPassportClerk[0];
        myLine = 0;
        for (i = 0; i < numPassportClerk; i++){
            if((peopleInPassportClerk[i] < leastPeopleInPassport) && (passportClerkState[i] != 4) && (passportClerkState[i] != 3)){
                myLine = i;
                leastPeopleInPassport = peopleInPassportClerk[i];
            }
        }


        /*after finding shortest line, get in line and wait till passport clerk thread signals customer thread*/
        if (passportClerkState[myLine] == 2){
                passportClerkState[myLine] = 1;
        }
        if (myMoney > 500){
            myMoney = myMoney - 500;
            passportClerkBribeLineCount[myLine]++;     /*customer increment line count it is in*/
            
            
            Write("Customer ", sizeof("Customer "), ConsoleOutput);
        	PrintInt(customerSSN);
        	Write(" has gotten in bribe line for PassportClerk ", sizeof(" has gotten in bribe line for PassportClerk "), ConsoleOutput);
        	PrintInt(myLine);
    		Write("\n",sizeof("\n"), ConsoleOutput);
            /*
            passportClerkBribeLineCV[myLine]->Wait(passportClerkLineLock);
            */
            Wait(passportClerkBribeLineCV[myLine],passportClerkLineLock);

            passportClerkBribeLineCount[myLine]--; 
        }
        else{
            passportClerkLineCount[myLine]++;  /*customer increment line count it is in*/
            Write("Customer ", sizeof("Customer "), ConsoleOutput);
        	PrintInt(customerSSN);
        	Write(" has gotten in regular line for PassportClerk ", sizeof(" has gotten in regular line for PassportClerk "), ConsoleOutput);
        	PrintInt(myLine);
    		Write("\n",sizeof("\n"), ConsoleOutput);
            /*
            passportClerkLineCV[myLine]->Wait(passportClerkLineLock);   
            */
            Wait(passportClerkLineCV[myLine],passportClerkLineLock);

            passportClerkLineCount[myLine]--;  /*customer decrement line count after waken up*/
        }
        Release(passportClerkLineLock);

        /*update money and myLine*/
        customerMoney[customerSSN] = myMoney;
        customerLine[customerSSN] = myLine;

        /*interaction between customer and passport clerk*/
        Acquire(passportClerkLock[myLine]);
        passportClerkCustSSN[myLine] = customerSSN;
        
        Write("Customer ", sizeof("Customer "), ConsoleOutput);
    	PrintInt(customerSSN);
    	Write(" has given SSN to PassportClerk ", sizeof(" has given SSN to PassportClerk "), ConsoleOutput);
    	PrintInt(myLine);
		Write("\n",sizeof("\n"), ConsoleOutput);
        /*
        passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);    
        passportClerkCV[myLine]->Wait(passportClerkLock[myLine]);      
        */
        Signal(passportClerkCV[myLine],passportClerkLock[myLine]);
        Wait(passportClerkCV[myLine],passportClerkLock[myLine]);

        if(!(customer_apps[customerSSN]==1 && customer_photos[customerSSN]==1)){
            Write("Customer ", sizeof("Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" has gone to PassportClerk ", sizeof(" has gone to PassportClerk "), ConsoleOutput);
	    	PrintInt(myLine);
			Write(" too soon. They are going to the back of the line.\n",sizeof(" too soon. They are going to the back of the line.\n"), ConsoleOutput);
            /*
            int yieldCount = rand()%900+100;      
            for(i = 0; i < yieldCount; i++){    
                currentThread->Yield();             
            }
            */
            Release(passportClerkLock[myLine]);
            continue;
        }
        /*
        passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);    
        
        */

        Release(passportClerkLock[myLine]);
        
        customerMoney[customerSSN] = myMoney;
        customerLine[customerSSN] = myLine;
        break;
    }

    while(1){

        Acquire(cashierLineLock);
        leastPeopleInCashier = cashierLineCount[0];
        myLine = 0;
        for (i = 0; i < numCashier; i++){
            if((cashierLineCount[i] < leastPeopleInCashier) && (cashierState[i] != 4) && (cashierState[i] != 3)){
                myLine = i;
                leastPeopleInCashier = cashierLineCount[i];
            }
        }


        /*after finding shortest line, get in line and wait till cashier thread signals customer thread*/
        if (cashierState[myLine] == 2){
                cashierState[myLine] = 1;
        }
        /*no bribe line in casheir*/
        cashierLineCount[myLine]++;  /*customer increment line count it is in*/
        /*printf("Customer_%d is waiting in line %d of Cashier , with %d people in line...$%d left\n", customerIndex, myLine, cashierLineCount[myLine], myMoney);*/
        
        Write("Customer ", sizeof("Customer "), ConsoleOutput);
    	PrintInt(customerSSN);
    	Write(" has gotten in regular line for Cashier ", sizeof(" has gotten in regular line for Cashier "), ConsoleOutput);
    	PrintInt(myLine);
		Write("\n",sizeof("\n"), ConsoleOutput);
        /*
        cashierLineCV[myLine]->Wait(cashierLineLock);   
        */
        Wait(cashierLineCV[myLine],cashierLineLock);
        cashierLineCount[myLine]--;  /*customer decrement line count after waken up*/
        Release(cashierLineLock);
        
        /*update money and myLine*/
        customerMoney[customerSSN] = myMoney;
        customerLine[customerSSN] = myLine;

        /*interaction between customer and cashier*/
        Acquire(cashierLock[myLine]);
        cashierCustSSN[myLine] = customerSSN;
        
        Write("Customer ", sizeof("Customer "), ConsoleOutput);
    	PrintInt(customerSSN);
    	Write(" has given SSN to Cashier ", sizeof(" has given SSN to Cashier "), ConsoleOutput);
    	PrintInt(myLine);
		Write("\n",sizeof("\n"), ConsoleOutput);
        /*
        cashierCV[myLine]->Signal(cashierLock[myLine]);
        cashierCV[myLine]->Wait(cashierLock[myLine]);  
        */
        if (!(customer_passport[customerSSN]==1)){
            
            Write("Customer ", sizeof("Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" has gone to Cashier ", sizeof(" has gone to Cashier "), ConsoleOutput);
	    	PrintInt(myLine);
			Write(" too soon. They are going to the back of the line.\n",sizeof(" too soon. They are going to the back of the line.\n"), ConsoleOutput);
            
            Release(cashierLineLock);
            /*
            int yieldCount = rand()%900+100;      
            for(i = 0; i < yieldCount; i++){    
                currentThread->Yield();
            }
            */
            continue;
        }
        myMoney = myMoney - 100;
       
        Write("Customer ", sizeof("Customer "), ConsoleOutput);
    	PrintInt(customerSSN);
    	Write(" has given Cashier ", sizeof(" has given Cashier "), ConsoleOutput);
    	PrintInt(myLine);
		Write(" $100.\n",sizeof(" $100.\n"), ConsoleOutput);
        /*
        cashierCV[myLine]->Signal(cashierLock[myLine]);
        */
        Release(cashierLock[myLine]);
        /*cashier done..*/
        /*update money and myLine*/
        customerMoney[customerSSN] = myMoney;
        customerLine[customerSSN] = myLine;

        /*customer finished visiting cashier*/

        /*now customer is finished the whole applying for passport process*/
     
        Write("Customer ", sizeof("Customer "), ConsoleOutput);
    	PrintInt(customerSSN);
		Write(" is leaving the Passport Office\n",sizeof(" is leaving the Passport Office\n"), ConsoleOutput);

        /*leaving the office....*/
        break;
    }

	Exit(0);
}
void AppClerk() {

	int i;
    int customerSSN;
    int appClerkIndex;

    Acquire(appClerkIndexLock);
    appClerkIndex = appClerkNum;
    appClerkNum++;
    Write("Making AppClerk: ",sizeof("Making AppClerk: "),ConsoleOutput);
    PrintInt(appClerkIndex);
    Write("\n",sizeof("\n"), ConsoleOutput);
    Release(appClerkIndexLock);

    while(1) {
        Acquire(appClerkLineLock);
        
        if(appClerkBribeLineCount[appClerkIndex] > 0){
            /*
            appClerkBribeLineCV[appClerkIndex]->Signal(appClerkLineLock);
            */
            Signal(appClerkBribeLineCV[appClerkIndex], appClerkLineLock);
            
            
            Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
	    	PrintInt(appClerkIndex);
	    	Write(" has signalled a Customer to come to their counter.\n", sizeof(" has signalled a Customer to come to their counter.\n"), ConsoleOutput);
	    	
            appClerkState[appClerkIndex] = 1;

            Acquire(appClerkSSNLock[appClerkIndex]);
            Release(appClerkLineLock);
            /*
            appClerkSSNCV[appClerkIndex] -> Wait(appClerkSSNLock[appClerkIndex]);
            */
            Wait(appClerkSSNCV[appClerkIndex], appClerkSSNLock[appClerkIndex]);
            customerSSN = appClerkCustSSN[appClerkIndex];
            /*
            appClerkSSNCV[appClerkIndex] -> Signal(appClerkSSNLock[appClerkIndex]);
			*/
			Signal(appClerkSSNCV[appClerkIndex],appClerkSSNLock[appClerkIndex]);

            Acquire(appClerkLineLock);
            Release(appClerkSSNLock[appClerkIndex]);


            Acquire(appClerkMoneyLock[appClerkIndex]);
            appClerkMoney[appClerkIndex] += 500;
            
            
            Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
	    	PrintInt(appClerkIndex);
	    	Write(" has received $500 from Customer ", sizeof(" has received $500 from Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);

            Release(appClerkMoneyLock[appClerkIndex]);
            
        } else if(appClerkLineCount[appClerkIndex] > 0){
        	/*
            appClerkLineCV[appClerkIndex]->Signal(appClerkLineLock);
            */
            Signal(appClerkLineCV[appClerkIndexLock],appClerkLineLock);
           
            if (senatorInOffice){
                Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
		    	PrintInt(appClerkIndex);
		    	Write(" has signalled a Senator to come to their counter.\n", sizeof(" has signalled a Senator to come to their counter.\n"), ConsoleOutput);
            }
            else{
                Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
		    	PrintInt(appClerkIndex);
		    	Write(" has signalled a Customer to come to their counter.\n", sizeof(" has signalled a Customer to come to their counter.\n"), ConsoleOutput);
            }
                
            
            appClerkState[appClerkIndex] = 1;
        } else {
            /*break code here*/
            /*wait on a cv specific to me*/
            appClerkState[appClerkIndex] = 3;
            Acquire(appClerkBreakLock);
            Release(appClerkLineLock);
           
            Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
	    	PrintInt(appClerkIndex);
	    	Write(" is going on break.\n", sizeof(" is going on break.\n"), ConsoleOutput);
            /*
            appClerkBreakCV[appClerkIndex]->Wait(appClerkBreakLock);
            */
            Wait(appClerkBreakCV[appClerkIndex],appClerkBreakLock);
            
            Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
	    	PrintInt(appClerkIndex);
	    	Write(" is going off break.\n", sizeof(" is going off break.\n"), ConsoleOutput);
            
            Release(appClerkBreakLock);

            /*break;/*TODO:remove*/
            continue;
            
        }
        /*interaction between customer and application clerk*/
        Acquire(appClerkLock[appClerkIndex]);
        Release(appClerkLineLock);
        /*Wait for customer to give me SSN.*/
        /*
        appClerkCV[appClerkIndex]->Wait(appClerkLock[appClerkIndex]);
        */
        Wait(appClerkCV[appClerkIndex],appClerkLock[appClerkIndex]);
        customerSSN = appClerkCustSSN[appClerkIndex];
        
        if (senatorInOffice){
        	Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
	    	PrintInt(appClerkIndex);
	    	Write(" has received SSN from Senator ", sizeof(" has received SSN from Senator "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        else{
            Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
	    	PrintInt(appClerkIndex);
	    	Write(" has received SSN from Customer ", sizeof(" has received SSN from Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        
        
        /*Approve of Customer Application*/
        /*
        appClerkCV[appClerkIndex]->Signal(appClerkLock[appClerkIndex]);
        */
        Signal(appClerkCV[appClerkIndex],appClerkLock[appClerkIndex]);
        /*Wait for Customer to know their Application is completed.*/
        /*
        appClerkCV[appClerkIndex]->Wait(appClerkLock[appClerkIndex]);
        */
        Wait(appClerkCV[appClerkIndex],appClerkLock[appClerkIndex]);
        /*and be ready to leave. after this customer does one last signal. this will allow clerk to loop around*/
        /*
        int randYieldTime = rand() % 100 + 20;
            for(i = 0; i < randYieldTime; i++){
            currentThread->Yield();
        }
        */
        
        customer_apps[customerSSN] = 1;
        
        if (senatorInOffice){
        	Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
	    	PrintInt(appClerkIndex);
	    	Write(" has recorded a completed application for Senator ", sizeof(" has recorded a completed application for Senator "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        else{
            Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
	    	PrintInt(appClerkIndex);
	    	Write(" has recorded a completed application for Customer ", sizeof(" has recorded a completed application for Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
            
        
        Release(appClerkLock[appClerkIndex]); /*done with customer interaction*/
        
    }

	Exit(0);
}
void PhotoClerk() {

	int i;
    int customerSSN;
    int photoClerkIndex;
    int toRetakePhoto;
    int pickyness;

    Acquire(photoClerkIndexLock);
    photoClerkIndex = photoClerkNum;
    photoClerkNum++;
    Write("Making PhotoClerk: ",sizeof("Making PhotoClerk: "),ConsoleOutput);
    PrintInt(photoClerkIndex);
    Write("\n",sizeof("\n"), ConsoleOutput);
    Release(photoClerkIndexLock);

	while(1) {
        Acquire(photoClerkLineLock);
        if(photoClerkBribeLineCount[photoClerkIndex] > 0){
        	/*
            photoClerkBribeLineCV[photoClerkIndex]->Signal(photoClerkLineLock);
            */
            Signal(photoClerkBribeLineCV[photoClerkIndex],photoClerkLineLock);

            Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
	    	PrintInt(photoClerkIndex);
	    	Write(" has signalled a Customer to come to their counter.\n", sizeof(" has signalled a Customer to come to their counter.\n"), ConsoleOutput);
            photoClerkState[photoClerkIndex] = 1;
            Acquire(photoClerkMoneyLock[photoClerkIndex]);
            photoClerkMoney[photoClerkIndex] += 500;
            Release(photoClerkMoneyLock[photoClerkIndex]);
        }else if(photoClerkLineCount[photoClerkIndex] > 0){
        	/*
            photoClerkLineCV[photoClerkIndex]->Signal(photoClerkLineLock);
            */
            Signal(photoClerkLineCV[photoClerkIndex],photoClerkLineLock);
            
            if (senatorInOffice){
                Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
		    	PrintInt(photoClerkIndex);
		    	Write(" has signalled a Senator to come to their counter.\n", sizeof(" has signalled a Senator to come to their counter.\n"), ConsoleOutput);
            }
            else{
                Write("ApplicationClerk ", sizeof("ApplicationClerk "), ConsoleOutput);
		    	PrintInt(photoClerkIndex);
		    	Write(" has signalled a Customer to come to their counter.\n", sizeof(" has signalled a Customer to come to their counter.\n"), ConsoleOutput);
            }
            
            photoClerkState[photoClerkIndex] = 1;
        } else {
            /*break code here*/
            /*wait on a cv specific to me*/
            photoClerkState[photoClerkIndex] = 3;
            Acquire(photoClerkBreakLock);
            Release(photoClerkLineLock);

            Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
	    	PrintInt(photoClerkIndex);
	    	Write(" is going on break.\n", sizeof(" is going on break.\n"), ConsoleOutput);

            /*
            photoClerkBreakCV[photoClerkIndex]->Wait(photoClerkBreakLock);
            */
            Wait(photoClerkBreakCV[photoClerkIndex],photoClerkBreakLock);
            
            Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
	    	PrintInt(photoClerkIndex);
	    	Write(" is going off break.\n", sizeof(" is going off break.\n"), ConsoleOutput);
            
            Release(photoClerkBreakLock);

            /*break;/*TODO:remove*/
            continue;

        }
        /*interaction between customer and application clerk*/
        Acquire(photoClerkLock[photoClerkIndex]);
        Release(photoClerkLineLock);
        /*Wait for customer to give me SSN.*/
        /*
        photoClerkCV[photoClerkIndex]->Wait(photoClerkLock[photoClerkIndex]);
        */
        Wait(photoClerkCV[photoClerkIndex],photoClerkLock[photoClerkIndex]);

        customerSSN = photoClerkCustSSN[photoClerkIndex];
        
        if (senatorInOffice){
        	Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
	    	PrintInt(photoClerkIndex);
	    	Write(" has received SSN from Senator ", sizeof(" has received SSN from Senator "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        else{
            Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
	    	PrintInt(photoClerkIndex);
	    	Write(" has received SSN from Customer ", sizeof(" has received SSN from Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        
        
        
        /*Approve of Customer Photo*/

        if (senatorInOffice){
        	Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
	    	PrintInt(photoClerkIndex);
	    	Write(" has taken a picture of Senator ", sizeof(" has taken a picture of Senator "), ConsoleOutput);
	    	PrintInt(photoClerkIndex);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        else{
            Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
	    	PrintInt(photoClerkIndex);
	    	Write(" has taken a picture of Customer ", sizeof(" has taken a picture of Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
            
        
        pickyness = customerPicky[customerSSN];
        if (senatorInOffice){
            pickyness = senatorPicky[customerSSN];
        }
        toRetakePhoto = 100; /*rand() %99 + 1;*/

        if (toRetakePhoto >= pickyness){
            
            if (senatorInOffice){
            	senator_likesPhoto[customerSSN] = 1;
            	Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
		    	PrintInt(photoClerkIndex);
		    	Write(" has been told that Senator ", sizeof(" has been told that Senator "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does like their picture\n", sizeof(" does like their picture\n"), ConsoleOutput);
            }
            else{
            	customer_likesPhoto[customerSSN] = 1;
                Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
		    	PrintInt(photoClerkIndex);
		    	Write(" has been told that Customer ", sizeof(" has been told that Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does like their picture\n", sizeof(" does like their picture\n"), ConsoleOutput);
            }
            
            
        }
        else{
            
            if (senatorInOffice){
            	senator_likesPhoto[customerSSN] = 0;
                Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
		    	PrintInt(photoClerkIndex);
		    	Write(" has been told that Senator ", sizeof(" has been told that Senator "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does NOT like their picture\n", sizeof(" does NOT like their picture\n"), ConsoleOutput);
            }
            else{
            	customer_likesPhoto[customerSSN] = 0;
                Write("PhotoClerk ", sizeof("PhotoClerk "), ConsoleOutput);
		    	PrintInt(photoClerkIndex);
		    	Write(" has been told that Customer ", sizeof(" has been told that Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does NOT like their picture\n", sizeof(" does NOT like their picture\n"), ConsoleOutput);
            }
            
            

        }
        /*
        photoClerkCV[photoClerkIndex]->Signal(photoClerkLock[photoClerkIndex]);
        photoClerkCV[photoClerkIndex]->Wait(photoClerkLock[photoClerkIndex]);
		*/
        Signal(photoClerkCV[photoClerkIndex],photoClerkLock[photoClerkIndex]);
        Wait(photoClerkCV[photoClerkIndex],photoClerkLock[photoClerkIndex]);


        if (customer_likesPhoto[customerSSN]){
            /*
            int randYieldTime = rand() % 100 + 20;
                for(i = 0; i < randYieldTime; i++){
                currentThread->Yield();
            }
            */
            customer_photos[customerSSN] = 1;
        }

        
        Release(photoClerkLock[photoClerkIndex]); /*done with customer interaction*/
        
    }
	Exit(0);
}
void PassportClerk() {
	int customerSSN;
	int passportClerkIndex;

	Acquire(passportClerkIndexLock);
    passportClerkIndex = passportClerkNum;
    passportClerkNum++;
    Write("Making PassportClerk: ",sizeof("Making PassportClerk: "),ConsoleOutput);
    PrintInt(passportClerkIndex);
    Write("\n",sizeof("\n"), ConsoleOutput);
    Release(passportClerkIndexLock);

	while(1) {
        Acquire(passportClerkLineLock);
        
        if(passportClerkBribeLineCount[passportClerkIndex] > 0){
        	/*
            passportClerkBribeLineCV[passportClerkIndex]->Signal(passportClerkLineLock);
            */
            /*take money*/
            Acquire(passportClerkMoneyLock[passportClerkIndex]);
            passportClerkMoney[passportClerkIndex] += 500;
            Release(passportClerkMoneyLock[passportClerkIndex]);


            Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
	    	PrintInt(passportClerkIndex);
	    	Write(" has signalled a Customer to come to their counter.\n", sizeof(" has signalled a Customer to come to their counter.\n"), ConsoleOutput);

            passportClerkState[passportClerkIndex] = 1;
            
        } else if(passportClerkLineCount[passportClerkIndex] > 0){
        	/*
            passportClerkLineCV[passportClerkIndex]->Signal(passportClerkLineLock);
            */
            
            if (senatorInOffice){
                Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
		    	PrintInt(passportClerkIndex);
		    	Write(" has signalled a Senator to come to their counter.\n", sizeof(" has signalled a Senator to come to their counter.\n"), ConsoleOutput);
            }
            else{
                Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
		    	PrintInt(passportClerkIndex);
		    	Write(" has signalled a Customer to come to their counter.\n", sizeof(" has signalled a Customer to come to their counter.\n"), ConsoleOutput);
            }
            
            passportClerkState[passportClerkIndex] = 1;
        } else {
            passportClerkState[passportClerkIndex] = 3;
            Acquire(passportClerkBreakLock);
            Release(passportClerkLineLock);
            
            Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
	    	PrintInt(passportClerkIndex);
	    	Write(" is going on break.\n", sizeof(" is going on break.\n"), ConsoleOutput);

            /*
            passportClerkBreakCV[passportClerkIndex]->Wait(passportClerkBreakLock);
            */
            Wait(passportClerkBreakCV[passportClerkIndex],passportClerkBreakLock);
            
            Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
	    	PrintInt(passportClerkIndex);
	    	Write(" is going off break.\n", sizeof(" is going off break.\n"), ConsoleOutput);
            
            Release(passportClerkBreakLock);
            
            /*break;/*TODO:remove*/
            continue;
        }


        /*interaction between customer and passport clerk*/
        Acquire(passportClerkLock[passportClerkIndex]);
        Release(passportClerkLineLock);
        /*Wait for customer to give me SSN.*/
        /*
        passportClerkCV[passportClerkIndex]->Wait(passportClerkLock[passportClerkIndex]);
        */
        customerSSN = passportClerkCustSSN[passportClerkIndex];
        
        if (senatorInOffice){
        	Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
	    	PrintInt(passportClerkIndex);
	    	Write(" has received SSN from Senator ", sizeof(" has received SSN from Senator "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        else{
            Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
	    	PrintInt(passportClerkIndex);
	    	Write(" has received SSN from Customer ", sizeof(" has received SSN from Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        
        
        /*Approve of Customer Passport Application*/
        if (!(customer_apps[customerSSN]==1 && customer_photos[customerSSN]==1)){
            /*customer came too soon, kick him back in line and get next customer*/
            
            if (senatorInOffice){
            	Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
		    	PrintInt(passportClerkIndex);
		    	Write(" has determined that Senator ", sizeof(" has determined that Senator "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does not have both their application and picture competed\n", sizeof(" does not have both their application and picture competed\n"), ConsoleOutput);
            }
            else{
                Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
		    	PrintInt(passportClerkIndex);
		    	Write(" has determined that Customer ", sizeof(" has determined that Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does NOT have both their application and picture competed\n", sizeof(" does NOT have both their application and picture competed\n"), ConsoleOutput);
            }
            /*
            passportClerkCV[passportClerkIndex]->Signal(passportClerkLock[passportClerkIndex]);
            */
            Release(passportClerkLock[passportClerkIndex]);
            continue;
        }
        else {
            
            if (senatorInOffice){
            	Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
		    	PrintInt(passportClerkIndex);
		    	Write(" has determined that Senator ", sizeof(" has determined that Senator "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does have both their application and picture competed\n", sizeof(" does have both their application and picture competed\n"), ConsoleOutput);
            }
            else{
                Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
		    	PrintInt(passportClerkIndex);
		    	Write(" has determined that Customer ", sizeof(" has determined that Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" does have both their application and picture competed\n", sizeof(" does have both their application and picture competed\n"), ConsoleOutput);
            }
        }
        /*
        passportClerkCV[passportClerkIndex]->Signal(passportClerkLock[passportClerkIndex]);
        */
        /*Wait for Customer to know their Passport Application is completed.*/
        /*
        passportClerkCV[passportClerkIndex]->Wait(passportClerkLock[passportClerkIndex]);
        */
        
        /*
        int randYieldTime = rand() % 100 + 20;
            for(i = 0; i < randYieldTime; i++){
            currentThread->Yield();
        }
        */
        customer_passport[customerSSN] = 1;
        
        if (senatorInOffice){
            Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
	    	PrintInt(passportClerkIndex);
	    	Write(" has recorded Senator ", sizeof(" has recorded Senator "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" passport documentation\n", sizeof(" passport documentation\n"), ConsoleOutput);
        }
        else{
            Write("PassportClerk ", sizeof("PassportClerk "), ConsoleOutput);
	    	PrintInt(passportClerkIndex);
	    	Write(" has recorded Customer ", sizeof(" has recorded Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" passport documentation\n", sizeof(" passport documentation\n"), ConsoleOutput);
        }
        
        

        Release(passportClerkLock[passportClerkIndex]); /*done with customer interaction*/
        /*break; /*TODO:remove*/
    }
	Exit(0);
}
void Cashier() {
	
	/*Cashier code goes here*/
    int i;
    int cashierIndex;
    int customerSSN;

    Acquire(cashierIndexLock);
    cashierIndex = cashierNum;
    cashierNum++;
    Write("Making Cashier: ",sizeof("Making Cashier: "),ConsoleOutput);
    PrintInt(cashierIndex);
    Write("\n",sizeof("\n"), ConsoleOutput);
    Release(cashierIndexLock);

    while(1) {
        Acquire(cashierLineLock);
        
        if(cashierLineCount[cashierIndex] > 0){
        	/*
            cashierLineCV[cashierIndex]->Signal(cashierLineLock);
            */

            if (senatorInOffice){
                Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    		PrintInt(cashierIndex);
	    		Write(" has signalled a Senator to come to their counter.\n", sizeof(" has signalled a Customer to come to their counter.\n"), ConsoleOutput);
            }
            else{
                Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
		    	PrintInt(cashierIndex);
		    	Write(" has signalled a Customer to come to their counter.\n", sizeof(" has signalled a Customer to come to their counter.\n"), ConsoleOutput);
            }
            
            
            cashierState[cashierIndex] = 1;
        } else {
            cashierState[cashierIndex] = 3;
            Acquire(cashierBreakLock);
            Release(cashierLineLock);
            

            Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" is going on break.\n", sizeof(" is going on break.\n"), ConsoleOutput);
            /*
            cashierBreakCV[cashierIndex]->Wait(cashierBreakLock);
            */
            Wait(cashierBreakCV[cashierIndex],cashierBreakLock);
            
            Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" is going off break.\n", sizeof(" is going off break.\n"), ConsoleOutput);
            
            Release(cashierBreakLock);
            

            /*break;/*TODO:remove*/
            continue;
            
        }
        /*interaction between customer and cashier*/
        Acquire(cashierLock[cashierIndex]);
        Release(cashierLineLock);
        /*Wait for customer to give me SSN.*/
        /*
        cashierCV[cashierIndex]->Wait(cashierLock[cashierIndex]);
        */
        customerSSN = cashierCustSSN[cashierIndex];
        
        if (senatorInOffice){
        	Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" has received SSN from Senator ", sizeof(" has received SSN from Senator "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        else{
            Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" has received SSN from Customer ", sizeof(" has received SSN from Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write("\n", sizeof("\n"), ConsoleOutput);
        }
        
        

        if (!(customer_passport[customerSSN]==1)){
            /*customer came too soon, kick him back in line and get next customer*/
            
            if (senatorInOffice){
            	Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
		    	PrintInt(cashierIndex);
		    	Write(" has determined that Senator ", sizeof(" has determined that Senator "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" has NOT been certified by a PassportClerk\n", sizeof(" has NOT been certified by a PassportClerk\n"), ConsoleOutput);
            }
            else{
                Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
		    	PrintInt(cashierIndex);
		    	Write(" has determined that Customer ", sizeof(" has determined that Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" has NOT been certified by a PassportClerk\n", sizeof(" has NOT been certified by a PassportClerk\n"), ConsoleOutput);
            }
                
            /*
            cashierCV[cashierIndex]->Signal(cashierLock[cashierIndex]);
            */
            Release(cashierLock[cashierIndex]);
            continue;
        }
        /*Approve of Customer Payments*/
        
        if (senatorInOffice){
            	Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
		    	PrintInt(cashierIndex);
		    	Write(" has determined that Senator ", sizeof(" has determined that Senator "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" has been certified by a PassportClerk\n", sizeof(" has been certified by a PassportClerk\n"), ConsoleOutput);
            }
            else{
                Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
		    	PrintInt(cashierIndex);
		    	Write(" has determined that Customer ", sizeof(" has determined that Customer "), ConsoleOutput);
		    	PrintInt(customerSSN);
		    	Write(" has been certified by a PassportClerk\n", sizeof(" has been certified by a PassportClerk\n"), ConsoleOutput);
            }

        /*
        int randYieldTime = rand() % 100 + 20;
        for(i = 0; i < randYieldTime; i++){
            currentThread->Yield();
        }
        */
        Acquire(cashierMoneyLock[cashierIndex]);
        cashierMoney[cashierIndex] += 100;
        Release(cashierMoneyLock[cashierIndex]);
        
        if (senatorInOffice){
        	Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" has received the $100 from Senator ", sizeof(" has received the $100 from Senator "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" after certification\n", sizeof(" after certification\n"), ConsoleOutput);
        }
        else{
            Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" has received the $100 from Customer ", sizeof(" has received the $100 from Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" after certification\n", sizeof(" after certification\n"), ConsoleOutput);
        }
        
        
        
        if (senatorInOffice){
            Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" has provided Senator ", sizeof(" has provided Senator "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" their completed passport\n", sizeof(" their completed passport\n"), ConsoleOutput);
        }
        else{
            Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" has provided Customer ", sizeof(" has provided Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" their completed passport\n", sizeof(" their completed passport\n"), ConsoleOutput);
        }
        
        /*
        cashierCV[cashierIndex]->Signal(cashierLock[cashierIndex]);
        */
        /*Wait for Customer to know their payment is completed.*/
        /*
        cashierCV[cashierIndex]->Wait(cashierLock[cashierIndex]);
        */
        
        if (senatorInOffice){
        	Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" has recorded that Senator ", sizeof(" has recorded that Senator "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" has been given their completed passport\n", sizeof(" has been given their completed passport\n"), ConsoleOutput);
        }
        else{
            Write("Cashier ", sizeof("Cashier "), ConsoleOutput);
	    	PrintInt(cashierIndex);
	    	Write(" has recorded that Customer ", sizeof(" has recorded that Customer "), ConsoleOutput);
	    	PrintInt(customerSSN);
	    	Write(" has been given their completed passport\n", sizeof(" has been given their completed passport\n"), ConsoleOutput);
        }

        Release(cashierLock[cashierIndex]); /*done with customer interaction*/
        /*break; /*TODO:remove*/
    }
	Exit(0);
}
void Senator() {
	Write("Making senator\n",sizeof("Making senator\n"),ConsoleOutput);
	Exit(0);
}
void Manager() {
	int i,j;
	int peopleInLine;
	int totalAppClerkMoney = 0;
	int totalPhotoClerkMoney = 0;
	int totalCashierMoney = 0;
	int totalPassportClerkMoney = 0;
	int totalMoneyMade = 0;
	
	Write("Making manager\n",sizeof("Making manager\n"),ConsoleOutput);

    senatorInOffice = 0;
    while(1){
        /*printf("Manager Thread.\n");/*tbd*/
        /*just to run manager thread on a regular basis...*/
        /*
        int yield = rand()%100+1000;
        */
        for(i = 0; i<1000; i++){
            Yield();
        }
        


        peopleInLine = 0;

        /*checking if there is senator....*/
        if (senatorCount > 0 && senatorInOffice == 0){

            /*printf("Manager: there is a senator entering the office, will broadcast senator after all customer go on wait. \n");/*tbd*/
            /*printf("WaitingCustomer: %d\n", waitingCustomer);/*tbd*/
            if (senatorInOffice == 0 && waitingCustomer == numCustomers){
                /*printf("Manager: manager has woken up all senators.\n");/*tbd*/
                senatorInOffice = 1;
                /*
                senatorCV->Broadcast(senatorLock);
				*/
            }            
        }
        
      
        
        /*printf("\nManager is monitoring the passport office...\n");*/
        /*Check Application Clerks, make them go on break or back to work*/
        Acquire(appClerkLineLock);
        peopleInLine = 0;
        for (i = 0; i < numAppClerk; i++){
            peopleInLine = appClerkLineCount[i] + appClerkBribeLineCount[i];
            if(appClerkState[i] == 3 && peopleInLine>0){
                Acquire(appClerkBreakLock);
                
                Write("Manager has woken up an Application Ckerk\n", sizeof("Manager has woken up an Application Ckerk\n"), ConsoleOutput);
                
                appClerkState[i] = 2;
                /*
                appClerkBreakCV[i]->Signal(appClerkBreakLock);
                */
                Signal(appClerkBreakCV[i],appClerkBreakLock);
                Release(appClerkBreakLock);
            }
            if (peopleInLine > 3){
                for (j = 0; j < numAppClerk; j++){
                    if (appClerkState[j] == 3){
                        Acquire(appClerkBreakLock);
                        
                        Write("Manager has woken up an Application Ckerk\n", sizeof("Manager has woken up an Application Ckerk\n"), ConsoleOutput);
                        
                        appClerkState[j] = 2;
                        /*
                        appClerkBreakCV[j]->Signal(appClerkBreakLock);
                        */
                        Signal(appClerkBreakCV[i],appClerkBreakLock);
                        Release(appClerkBreakLock);
                        break;
                    }
                }
            }
        }
        Release(appClerkLineLock);

        /*Check Photo Clerks, make them go on break or back to work*/
        Acquire(photoClerkLineLock);
        peopleInLine = 0;
        for (i = 0; i < numPhotoClerk; i++){
            peopleInLine = photoClerkLineCount[i] + photoClerkBribeLineCount[i];
            if(photoClerkState[i] == 3 && peopleInLine>0){
                Acquire(photoClerkBreakLock);
                
                Write("Manager has woken up an Photo Ckerk\n", sizeof("Manager has woken up an Photo Ckerk\n"), ConsoleOutput);
                
                photoClerkState[i] = 2;
                /*
                photoClerkBreakCV[i]->Signal(photoClerkBreakLock);
                */
                Signal(photoClerkBreakCV[i],photoClerkBreakLock);
                Release(photoClerkBreakLock);
            }
            if (peopleInLine > 3){
                for (j = 0; j < numPhotoClerk; j++){
                    if (photoClerkState[j] == 3){
                        Acquire(photoClerkBreakLock);
                        
                        Write("Manager has woken up an Photo Ckerk\n", sizeof("Manager has woken up an Photo Ckerk\n"), ConsoleOutput);
                        
                        photoClerkState[j] = 2;
                        /*
                        photoClerkBreakCV[j]->Signal(photoClerkBreakLock);
                        */
                        Signal(photoClerkBreakCV[i],photoClerkBreakLock);
                        Release(photoClerkBreakLock);
                        break;
                    }
                }
                
            }
        }
        Release(photoClerkLineLock);

        /*Check passport Clerks, make them go on break or back to work*/
        Acquire(passportClerkLineLock);
        peopleInLine = 0;
        for (i = 0; i < numPassportClerk; i++){
            peopleInLine = passportClerkLineCount[i] + passportClerkBribeLineCount[i];
            if(passportClerkState[i] == 3 && peopleInLine>0){
                Acquire(passportClerkBreakLock);
                
                Write("Manager has woken up an Passport Ckerk\n", sizeof("Manager has woken up an PassportClerk Ckerk\n"), ConsoleOutput);
                
                passportClerkState[i] = 2;
                /*
                passportClerkBreakCV[i]->Signal(passportClerkBreakLock);
                */
                Release(passportClerkBreakLock);
            }
            if (peopleInLine > 3){
                for (j = 0; j < numPassportClerk; j++){
                    if (passportClerkState[j] == 3){
                        Acquire(passportClerkBreakLock);
                        
                        Write("Manager has woken up an Passport Ckerk\n", sizeof("Manager has woken up an PassportClerk Ckerk\n"), ConsoleOutput);
                        
                        passportClerkState[j] = 2;
                        /*
                        passportClerkBreakCV[j]->Signal(passportClerkBreakLock);
                        */
                        Release(passportClerkBreakLock);
                        break;
                    }
                }
                
            }
        }
        Release(passportClerkLineLock);

        /*Check Cashier, make them go on break or back to work*/
        Acquire(cashierLineLock);
        peopleInLine = 0;
        for (i = 0; i < numCashier; i++){
            if(cashierState[i] == 3 && cashierLineCount[i]>0){
                Acquire(cashierBreakLock);
                
                Write("Manager has woken up an Cashier\n", sizeof("Manager has woken up an Cashier\n"), ConsoleOutput);
                
                cashierState[i] = 2;
                /*
                cashierBreakCV[i]->Signal(cashierBreakLock);
                */
                Release(cashierBreakLock);
            }
            if (cashierLineCount[i] > 3){
                for (j = 0; j < numCashier; j++){
                    if (cashierState[j] == 3){
                        Acquire(cashierBreakLock);
                        
                        Write("Manager has woken up an Cashier\n", sizeof("Manager has woken up an Cashier\n"), ConsoleOutput);
                        
                        cashierState[j] = 2;
                        /*
                        cashierBreakCV[j]->Signal(cashierBreakLock);
                        */
                        Release(cashierBreakLock);
                        break;
                    }
                }
                
            }
        }
        Release(cashierLineLock);

        totalAppClerkMoney = 0;
        for (i = 0; i < numAppClerk; i++){
            Acquire(appClerkMoneyLock[i]);
            totalAppClerkMoney = totalAppClerkMoney + appClerkMoney[i];
            Release(appClerkMoneyLock[i]);
        }
        totalPhotoClerkMoney = 0;
        for (i = 0; i < numPhotoClerk; i++){
            Acquire(photoClerkMoneyLock[i]);
            totalPhotoClerkMoney = totalPhotoClerkMoney + photoClerkMoney[i];
            Release(photoClerkMoneyLock[i]);
        }
        totalPassportClerkMoney = 0;
        for (i = 0; i < numPassportClerk; i++){
            Acquire(passportClerkMoneyLock[i]);
            totalPassportClerkMoney = totalPassportClerkMoney + passportClerkMoney[i];
            Release(passportClerkMoneyLock[i]);
        }
        totalCashierMoney = 0;
        for (i = 0; i < numCashier; i++){
            Acquire(cashierMoneyLock[i]);
            totalCashierMoney = totalCashierMoney + cashierMoney[i];
            Release(cashierMoneyLock[i]);
        }

        /*printing money collected from each types of clerk*/
        totalMoneyMade = totalAppClerkMoney + totalPhotoClerkMoney + totalPassportClerkMoney + totalCashierMoney;
        
        Write("Manager has counted a total of ", sizeof("Manager has counted a total of "), ConsoleOutput);
        PrintInt(totalAppClerkMoney);
        Write(" for Application Clerk\n", sizeof(" for Application Clerk\n"), ConsoleOutput);

        Write("Manager has counted a total of ", sizeof("Manager has counted a total of "), ConsoleOutput);
        PrintInt(totalPhotoClerkMoney);
        Write(" for Photo Clerk\n", sizeof(" for Photo Clerk\n"), ConsoleOutput);

        Write("Manager has counted a total of ", sizeof("Manager has counted a total of "), ConsoleOutput);
        PrintInt(totalPassportClerkMoney);
        Write(" for Passport Clerk\n", sizeof(" for Passport Clerk\n"), ConsoleOutput);

        Write("Manager has counted a total of ", sizeof("Manager has counted a total of "), ConsoleOutput);
        PrintInt(totalCashierMoney);
        Write(" for Cashier\n", sizeof(" for Cashier\n"), ConsoleOutput);

        Write("Manager has counted a total of ", sizeof("Manager has counted a total of "), ConsoleOutput);
        PrintInt(totalMoneyMade);
        Write(" for Passport Office\n", sizeof(" for Passport Office\n"), ConsoleOutput);
        
    

        if (senatorCount == 0 && senatorInOffice == 1){
         /*   printf("Manager: All senators have left gotten their passport and left the office.\n");*/
        	/*
            customerWaitSenatorCV->Broadcast(senatorLock);
            */
            waitingCustomer = 0;
            senatorInOffice = 0;
        }

        /*printf("totalCustomerMoney: $%d \n", totalCustomerMoney);*/

        /*if (totalMoneyMade == (totalCustomerMoney + totalSenatorMoney)){*/
        if (totalMoneyMade == 1000){
            /*printf("\nALL CUSTOMERS DONE!!!!\n\n");*/
            
            break;
        }


        /*break; /*TODO:remove*/
    }
    
	Exit(0);
}

void initialize(){
    int i;
    char* name;

    customerIndexLock = CreateLock("customerIndexLock", sizeof("customerIndexLock"));
    appClerkIndexLock = CreateLock("appClerkIndexLock", sizeof("appClerkIndexLock"));
    appClerkLineLock = CreateLock("appClerkLineLock", sizeof("appClerkLineLock"));
    for (i = 0; i < numAppClerk; ++i){
        appClerkLock[i] = CreateLock("appClerkLock", sizeof("appClerkLock"));
        appClerkSSNLock[i] = CreateLock("appClerkSSNLock", sizeof("appClerkSSNLock"));
        appClerkMoneyLock[i] = CreateLock("appClerkMoneyLock", sizeof("appClerkMoneyLock"));
        appClerkSSNCV[i] = CreateCondition("appClerkSSNCV", sizeof("appClerkSSNCV"));
        appClerkLineCV[i] = CreateCondition("appClerkLineCV", sizeof("appClerkLineCV"));
        appClerkBribeLineCV[i] = CreateCondition("appClerkBribeLineCV", sizeof("appClerkBribeLineCV"));
        appClerkCV[i] = CreateCondition("appClerkCV", sizeof("appClerkCV"));
        appClerkBreakCV[i] = CreateCondition("appClerkBreakCV", sizeof("appClerkBreakCV"));
    }
    photoClerkIndexLock = CreateLock("photoClerkIndexLock", sizeof("photoClerkIndexLock"));
    photoClerkLineLock = CreateLock("photoClerkLineLock", sizeof("photoClerkLineLock"));
    for (i = 0; i < numPhotoClerk; ++i){
        photoClerkLock[i] = CreateLock("photoClerkLock", sizeof("photoClerkLock"));
        photoClerkMoneyLock[i] = CreateLock("photoClerkMoneyLock", sizeof("photoClerkMoneyLock"));
        photoClerkLineCV[i] = CreateCondition("photoClerkLineCV", sizeof("photoClerkLineCV"));
        photoClerkBribeLineCV[i] = CreateCondition("photoClerkBribeLineCV", sizeof("photoClerkBribeLineCV"));
        photoClerkCV[i] = CreateCondition("photoClerkCV", sizeof("photoClerkCV"));
        photoClerkBreakCV[i] = CreateCondition("photoClerkBreakCV", sizeof("photoClerkBreakCV"));
    }
    passportClerkIndexLock = CreateLock("passportClerkIndexLock", sizeof("passportClerkIndexLock"));
    passportClerkLineLock = CreateLock("passportClerkLineLock", sizeof("passportClerkLineLock"));
    for (i = 0; i < numPassportClerk; ++i){
        passportClerkLock[i] = CreateLock("passportClerkLock", sizeof("passportClerkLock"));
        passportClerkMoneyLock[i] = CreateLock("passportClerkMoneyLock", sizeof("passportClerkMoneyLock"));
        passportClerkLineCV[i] = CreateLock("passportClerkLineCV", sizeof("passportClerkLineCV"));
        passportClerkBribeLineCV[i] = CreateCondition("passportClerkBribeLineCV", sizeof("passportClerkBribeLineCV"));
        passportClerkCV[i] = CreateCondition("passportClerkCV", sizeof("passportClerkCV"));
        passportClerkBreakCV[i] = CreateCondition("passportClerkBreakCV", sizeof("passportClerkBreakCV"));
    }
    cashierIndexLock = CreateLock("cashierIndexLock", sizeof("cashierIndexLock"));
    cashierLineLock = CreateLock("cashierLineLock", sizeof("cashierLineLock"));
    for (i = 0; i < numCashier; ++i){
        cashierLock[i] = CreateLock("cashierLock", sizeof("casheirLock"));
        cashierMoneyLock[i] = CreateLock("cashierMoneyLock", sizeof("cashierMoneyLock"));
        cashierCV[i] = CreateCondition("cashierCV", sizeof("cashierCV"));
        cashierLineCV[i] = CreateCondition("cashierLineCV", sizeof("cashierLineCV"));
        cashierBreakCV[i] = CreateCondition("cashierBreakCV", sizeof("cashierBreakCV"));
    }

    appClerkBreakLock = CreateLock("appClerkBreakLock", sizeof("appClerkBreakLock"));
    photoClerkBreakLock = CreateLock("photoClerkBreakLock", sizeof("photoClerkBreakLock"));
    passportClerkBreakLock = CreateLock("passportClerkBreakLock", sizeof("passportClerkBreakLock"));
    cashierBreakLock = CreateLock("cashierBreakLock", sizeof("cashierBreakLock"));



    for (i = 0; i < numCustomers; ++i){
        Fork(Customer);
    }
    for (i = 0; i < numAppClerk; ++i){
        Fork(AppClerk);
    }
    for (i = 0; i < numPhotoClerk; ++i){
        Fork(PhotoClerk);
    }
    for (i = 0; i < numPassportClerk; ++i){
        Fork(PassportClerk);
    }
    for (i = 0; i < numCashier; ++i){
        Fork(Cashier);
    }
    for (i = 0; i < numSenator; ++i){
        Fork(Senator);
    }

    Fork(Manager);

    /*t = new Thread("Manager");*/
    /*t->Fork((VoidFunctionPtr) Manager, 1);*/

    
}

int main()
{
    int i;
    /*Thread *t;*/
    /*srand(time(NULL));*/
    /*printf("Starting Problem 2: \n\n");*/
   
    /*fixed default values*/
    numCustomers = 1;
    numAppClerk = 1;
    numPhotoClerk = 1;
    numPassportClerk = 1;
    numCashier = 1;
    numSenator = 0;
    
    /*printf("Number of Customers = %d\n", numCustomers);*/
    /*printf("Number of ApplicationClerks = %d\n", numAppClerk);*/
    /*printf("Number of PictureClerks = %d\n", numPhotoClerk);*/
    /*printf("Number of PassportClerks = %d\n", numPassportClerk);*/
    /*printf("Number of Cashiers = %d\n", numCashier);*/
    /*printf("Number of Senators = %d\n\n", numSenator);*/

    Write("\n//////////////////////////////////////////////////////////////////\n", sizeof("\n//////////////////////////////////////////////////////////////////\n"), ConsoleOutput);
    Write("////////////////////Passport Office Simulation////////////////////\n", sizeof("////////////////////Passport Office Simulation////////////////////\n"), ConsoleOutput);
    Write("//////////////////////////////////////////////////////////////////\n\n", sizeof("//////////////////////////////////////////////////////////////////\n\n"), ConsoleOutput);

    initialize();
    
    Write("\n//////////////////////////////////////////////////////////////////\n", sizeof("\n//////////////////////////////////////////////////////////////////\n"), ConsoleOutput);
    Write("////////////////////Passport Office Simulation////////////////////\n", sizeof("////////////////////Passport Office Simulation////////////////////\n"), ConsoleOutput);
    Write("//////////////////////////////////////////////////////////////////\n\n", sizeof("//////////////////////////////////////////////////////////////////\n\n"), ConsoleOutput);
    Write("\n\n\n", sizeof("\n\n\n"), ConsoleOutput);
    /*Halt();*/
    /* not reached */
}
