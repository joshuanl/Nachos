#include "syscall.h"
int main(){
	/* This networking test code is to test exception cases for syscalls */

	int nonExistentLock = 100;
	int nonExistentCV = 100;
	int mvIndex;

	CreateLock("network3_lock0",sizeof("network3_lock0"));
	CreateLock("network3_lock0",sizeof("network3_lock0"));
	CreateLock("network3_lock0",sizeof("network3_lock0"));
	CreateLock("network3_lock0",sizeof("network3_lock0"));
	CreateLock("network3_lock0",sizeof("network3_lock0"));
	CreateLock("network3_lock0",sizeof("network3_lock0"));
	CreateLock("network3_lock0",sizeof("network3_lock0"));

	Write("Testing out of bound exceptions on Lock/CV/MV.\n", sizeof("Testing out of bound exceptions on Lock/CV/MV.\n"), ConsoleOutput);
	Acquire(500);
	Release(500);
	Signal(500,0);
	Wait(0,500);
	DestroyMV(500);

	Write("Acquiring non existent Lock\n", sizeof("Acquiring non existent Lock\n"), ConsoleOutput);
	Acquire(nonExistentLock);
	Write("Releasing non existent Lock\n", sizeof("Releasing non existent Lock\n"), ConsoleOutput);
	Release(nonExistentLock);
	Write("Destroying non existent Lock\n", sizeof("Destroying non existent Lock\n"), ConsoleOutput);
	DestroyLock(nonExistentLock);

	Write("Waiting non existent CV\n", sizeof("Waiting non existent CV\n"), ConsoleOutput);
	Wait(nonExistentCV, nonExistentLock);
	Write("Signalling non existent CV\n", sizeof("Signalling non existent CV\n"), ConsoleOutput);
	Signal(nonExistentCV, nonExistentLock);
	DestroyCondition(nonExistentCV);

	Write("Creating MV with size 10\n", sizeof("Creating MV with size 10\n"), ConsoleOutput);
	mvIndex = CreateMV("newMV",sizeof("newMV"),10);
	Write("Setting MV in index that exceed sizes\n", sizeof("Setting MV in index that exceed sizes\n"), ConsoleOutput);
	SetMV(mvIndex, 11, 5);
	Write("Getting MV in index that exceed sizes\n", sizeof("Getting MV in index that exceed sizes\n"), ConsoleOutput);
	GetMV(mvIndex, 11);
	Write("Destroying MV\n", sizeof("Destroying MV\n"), ConsoleOutput);
	DestroyMV(mvIndex);


}