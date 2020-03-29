#include <stdio.h>

unsigned long hash_filename(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++) {
        printf("%ld\n", hash);
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash%100;
}

int main() {
    int a = 3;
    printf("Test\n");
    // printf("%ld", hash_filename("test.c"));
    printf("%d\n", a);
    return 0;
}