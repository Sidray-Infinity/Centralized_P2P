#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int isNumber(char *buff) {
    // Determines if the string is a single digit.
    buff[strlen(buff)-1] = '\0';
    int num, flag = 0;
    for(int j=0; j<strlen(buff); j++) {
        if(buff[j] <= '9' && buff[j] >= '0')
            flag = 1;
        else {
            flag = 0;
            break;
        }
    }
    if(flag == 1) {
        sscanf(buff, "%d", &num);
        return num;
    }
    return -1;
}

int main() {
    char buff[1024];
    while(1) {
        bzero(buff, 1024);
        printf("Enter a stirng\n");
        fgets(buff, 1024, stdin);
        printf("OP: %d\n", isNumber(buff));
    }

    return 0;
}