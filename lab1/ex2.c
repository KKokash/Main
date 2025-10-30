#include <stdio.h>

int main(void) {
    printf("Size of int: %zu bytes\n", sizeof(int));
    printf("Size of float: %zu bytes\n", sizeof(float));
    printf("Size of double: %zu bytes\n", sizeof(double));
    printf("Size of void: %zu bytes (not allowed directly)\n", sizeof(void*));
    printf("Size of pointer: %zu bytes\n", sizeof(void*));

    printf("Size of short: %zu bytes\n", sizeof(short));
    printf("Size of long: %zu bytes\n", sizeof(long));


    if ((char)-1 < 0)
        printf("char is signed on this system.\n");
    else
        printf("char is unsigned on this system.\n");

    return 0;
}
