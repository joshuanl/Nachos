/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void DoIt(){
    Write("Hi.\n", sizeof("Hi.\n"), ConsoleOutput);
    Write("There\n", sizeof("There\n"), ConsoleOutput);
    Write("Grader. This is being forked. \n", sizeof("Grader. This is being forked. \n"), ConsoleOutput);
    Exit(0);
}

int main() {
  OpenFileId fd;
  int bytesread;
  char buf[20];
    
    Fork(DoIt);
    Fork(DoIt);
    Fork(DoIt);

    Create("testfile", 8);
    fd = Open("testfile", 8);
    Write("Testing a write.\n", 17, fd ); 
    Close(fd);


    fd = Open("testfile", 8);
    bytesread = Read( buf, 100, fd );
    Write( buf, bytesread, ConsoleOutput );
    Close(fd);
}
