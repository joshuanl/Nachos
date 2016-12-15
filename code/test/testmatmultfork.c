/* matmult.c
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in
* physical memory
*/

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int A2[Dim][Dim];
int B2[Dim][Dim];
int C2[Dim][Dim];

void matmultfork1(){
    int i, j, k;
    
    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
        for (j = 0; j < Dim; j++) {
            A[i][j] = i;
            B[i][j] = j;
            C[i][j] = 0;
        }
    
    for (i = 0; i < Dim; i++)		/* then multiply them together */
        for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
                C[i][j] += A[i][k] * B[k][j];
    
    Exit(C[Dim-1][Dim-1]);		/* and then we're done */

}

void matmultfork2(){
    int i, j, k;
    
    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
        for (j = 0; j < Dim; j++) {
            A2[i][j] = i;
            B2[i][j] = j;
            C2[i][j] = 0;
        }
    
    for (i = 0; i < Dim; i++)		/* then multiply them together */
        for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
                C2[i][j] += A2[i][k] * B2[k][j];
    
    Exit(C[Dim-1][Dim-1]);		/* and then we're done */
    
}

int
main()
{
    Fork(matmultfork1);
    Fork(matmultfork2);
}
