#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main() {
    int n;
	char c;

    FILE* fptr;

    if ((fptr = fopen("test-data.hex", "r")) == NULL) {
		printf("Error! opening file");

		// Program exits if the file pointer returns NULL.
		exit(1);
    }

    while (fread(&c, sizeof(char), 1, fptr)) {	
        // printf("%d", n);
		if (c == '\n')
			continue;
		printf("%c", c);
        //getchar();
    }

    fclose(fptr);

    return 0;
}