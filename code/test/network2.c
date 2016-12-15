#include "syscall.h"

int main(){
	int lock0Index;
	int cv0Index;
	
	/* MV testing start */

	SetMV(0,0,100);
	SetMV(0,1,200);
	SetMV(0,2,300);

	/* MV testing complete */

	/* Lock and CV testing start */

	lock0Index = CreateLock("network2_lock0",sizeof("network2_lock0"));
	cv0Index = CreateCondition("network2_cv0",sizeof("network2_cv0"));
	Write("lock0 acquired\n", sizeof("lock0 acquired\n"), ConsoleOutput);
	Acquire(lock0Index);
	
	Write("Wait on cv0 with lock0\n", sizeof("Wait on cv0 with lock0\n"), ConsoleOutput);
	Wait(cv0Index, lock0Index);
	Write("Freed by network 1 nachos\n", sizeof("Freed by network 1 nachos\n"), ConsoleOutput);
	Write("Releasing lock now\n", sizeof("Releasing lock now\n"), ConsoleOutput);
	Release(lock0Index);

	/* Lock and CV testing complete */
	Exit(0);
}