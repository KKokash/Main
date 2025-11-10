#include <stdio.h>
#include <stdlib.h>



void swap_pointers(int **p1, int **p2){
int *temp = *p1;
*p1 = *p2;
*p2 = temp;}
int main(void){
    int a = 1;
    int b = 2;
    // for testing we use pointers to integers
    int *p = &a;
    int *q = &b;
    printf("address of p = %p and  and q = %p\n", (int*)p, (int*)q);
    // prints p = &a and q = &b
    swap_pointers( (int**)&p , (int**)&q );
    printf("address of p = %p and q = %p\n", (int*)p, (int*)q);
    // prints p = &b and q = &a
    return 0;
}