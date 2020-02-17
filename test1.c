#include <stdio.h>
#include <stdlib.h>

int main() {

    FILE *f = fopen("a.out", "rb");
    if(f == NULL) {
        printf("File doesn't exist!\n");
        exit(1);
    }
    char c;


    return 0;
}