#include "syscall.h"

int main(){
	int lock0Index = 0;
	int cv0Index = 0;
	int mv0Index;
	int mvValue0=0;
	int mvValue1=50;
	int mvValue2=100;
	int i;
	
	mv0Index = CreateMV("network1_mv0",sizeof("network1_mv0"),3);

	/* MV testing Start */
	
	SetMV(0,0,24);
	SetMV(0,1,46);
	SetMV(0,2,68);

	for (i = 0; i < 100000; ++i); /* busy waiting */

	mvValue0 = GetMV(0,0);
	mvValue1 = GetMV(0,1);
	mvValue2 = GetMV(0,2);

	PrintInt(mvValue0);
	PrintInt(mvValue1);
	PrintInt(mvValue2);

	/* MV testing complete */

	/* Lock and CV testing start */

	Acquire(lock0Index);
	Write("lock0 acquired\n", sizeof("lock0 acquired\n"), ConsoleOutput);
	Signal(cv0Index,lock0Index);
	Write("Signalled CV 0 with lock 0\n", sizeof("Signalled CV 0 with lock 0\n"), ConsoleOutput);
	Write("Releasing lock now\n", sizeof("Releasing lock now\n"), ConsoleOutput);
	Release(lock0Index);

	/* Lock and CV testing complete */

	DestroyMV(mv0Index);

	Exit(0);

}