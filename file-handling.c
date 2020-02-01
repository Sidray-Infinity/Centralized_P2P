#include <stdio.h>
#include <string.h>
#include <stdlib.h>


long file_size(char *name) {
    FILE *fp = fopen(name, "rb"); //must be binary read to get bytes

    long size = -1;
    if(fp) {
        fseek (fp, 0, SEEK_END);
        size = ftell(fp)+1;
        fclose(fp);
    }
    return size;
}

void split_file(char * file_name, int segment) {
    int segments = 0, i, len, accum;
    FILE *fp1, *fp2;
    char filename[260] = {"smallFileName_"}; //base name for small files.
    char largeFileName[] = {"test-file.txt"}; //change to your path
    char smallFileName[260];
    char line[1080];

    long sizeFile = file_size(largeFileName);
    segments = sizeFile/segment + 1; //ensure end of file
    
    fp1 = fopen(largeFileName, "r");
    if(fp1) {
        for(i=0; i < segments; i++) {
            accum = 0;
            sprintf(smallFileName, "%s%d.txt", filename, i);
            fp2 = fopen(smallFileName, "w");
            if(fp2) {
                while(fgets(line, 1080, fp1) && accum <= segment) {
                    accum += strlen(line); //track size of growing file
                    fputs(line, fp2);
                }
                fclose(fp2);
            }
        }
        fclose(fp1);
    }
}

/*
FOR DEBUGGING PURPOSE ONLY
int main(void) {
    split_file("test-file.txt", 1500);
    return 0;
}
*/