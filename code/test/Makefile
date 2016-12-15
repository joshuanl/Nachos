# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult matmult_fork sort testfiles exectest1 exectest2 exectest3 forktest2 forktest1 locktest1 locktest2 CVtest1 testmatmult testmatmultfork testmatmult2 passport network1 network2 network3

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult
	
matmult_fork.o: matmult_fork.c
	$(CC) $(CFLAGS) -c matmult_fork.c
matmult_fork: matmult_fork.o start.o
	$(LD) $(LDFLAGS) start.o matmult_fork.o -o matmult_fork.coff
	../bin/coff2noff matmult_fork.coff matmult_fork

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

exectest1.o: exectest1.c
	$(CC) $(CFLAGS) -c exectest1.c
exectest1: exectest1.o start.o
	$(LD) $(LDFLAGS) start.o exectest1.o -o exectest1.coff
	../bin/coff2noff exectest1.coff exectest1

exectest2.o: exectest2.c
	$(CC) $(CFLAGS) -c exectest2.c
exectest2: exectest2.o start.o
	$(LD) $(LDFLAGS) start.o exectest2.o -o exectest2.coff
	../bin/coff2noff exectest2.coff exectest2

exectest3.o: exectest3.c
	$(CC) $(CFLAGS) -c exectest3.c
exectest3: exectest3.o start.o
	$(LD) $(LDFLAGS) start.o exectest3.o -o exectest3.coff
	../bin/coff2noff exectest3.coff exectest3

forktest2.o: forktest2.c
	$(CC) $(CFLAGS) -c forktest2.c
forktest2: forktest2.o start.o
	$(LD) $(LDFLAGS) start.o forktest2.o -o forktest2.coff
	../bin/coff2noff forktest2.coff forktest2

forktest1.o: forktest1.c
	$(CC) $(CFLAGS) -c forktest1.c
forktest1: forktest1.o start.o
	$(LD) $(LDFLAGS) start.o forktest1.o -o forktest1.coff
	../bin/coff2noff forktest1.coff forktest1

locktest1.o: locktest1.c
	$(CC) $(CFLAGS) -c locktest1.c
locktest1: locktest1.o start.o
	$(LD) $(LDFLAGS) start.o locktest1.o -o locktest1.coff
	../bin/coff2noff locktest1.coff locktest1

locktest2.o: locktest2.c
	$(CC) $(CFLAGS) -c locktest2.c
locktest2: locktest2.o start.o
	$(LD) $(LDFLAGS) start.o locktest2.o -o locktest2.coff
	../bin/coff2noff locktest2.coff locktest2

CVtest1.o: CVtest1.c
	$(CC) $(CFLAGS) -c CVtest1.c
CVtest1: CVtest1.o start.o
	$(LD) $(LDFLAGS) start.o CVtest1.o -o CVtest1.coff
	../bin/coff2noff CVtest1.coff CVtest1

testmatmult.o: testmatmult.c
	$(CC) $(CFLAGS) -c testmatmult.c
testmatmult: testmatmult.o start.o
	$(LD) $(LDFLAGS) start.o testmatmult.o -o testmatmult.coff
	../bin/coff2noff testmatmult.coff testmatmult

testmatmultfork.o: testmatmultfork.c
	$(CC) $(CFLAGS) -c testmatmultfork.c
testmatmultfork: testmatmultfork.o start.o
	$(LD) $(LDFLAGS) start.o testmatmultfork.o -o testmatmultfork.coff
	../bin/coff2noff testmatmultfork.coff testmatmultfork

testmatmult2.o: testmatmult2.c
	$(CC) $(CFLAGS) -c testmatmult2.c
testmatmult2: testmatmult2.o start.o
	$(LD) $(LDFLAGS) start.o testmatmult2.o -o testmatmult2.coff
	../bin/coff2noff testmatmult2.coff testmatmult2

passport.o: passport.c
	$(CC) $(CFLAGS) -c passport.c
passport: passport.o start.o
	$(LD) $(LDFLAGS) start.o passport.o -o passport.coff
	../bin/coff2noff passport.coff passport

network1.o: network1.c
	$(CC) $(CFLAGS) -c network1.c
network1: network1.o start.o
	$(LD) $(LDFLAGS) start.o network1.o -o network1.coff
	../bin/coff2noff network1.coff network1

network2.o: network2.c
	$(CC) $(CFLAGS) -c network2.c
network2: network2.o start.o
	$(LD) $(LDFLAGS) start.o network2.o -o network2.coff
	../bin/coff2noff network2.coff network2

network3.o: network3.c
	$(CC) $(CFLAGS) -c network3.c
network3: network3.o start.o
	$(LD) $(LDFLAGS) start.o network3.o -o network3.coff
	../bin/coff2noff network3.coff network3


clean:
	rm -f *.o *.coff
