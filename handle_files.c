#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long hash_filename(unsigned char *str) {
    // Creates a hash key for a filename
    unsigned long hash = 5381;
    int c;

    while (c = *str++) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash%100;
}

void split_file(char *addrs, int num_blocks, int str_len) {
    int sprint_stat;
    char cmnd[1024];
    bzero(cmnd, 1024);

    char orig_name[str_len];
    bzero(orig_name, str_len);

    // for(int i=0; i<str_len; i++) 
    //     orig_name[i] = addrs[i];

    strcpy(orig_name, addrs);

    // char *fileformat = strtok(addrs, ".");
    // fileformat = strtok(NULL, ".");

    sprint_stat = snprintf(cmnd, sizeof(cmnd), 
        "split %s %s -n %d -d", orig_name, addrs, num_blocks);
    if(sprint_stat == -1) {
        printf("Snprintf error!\n");
        exit(1);
    }
    // printf("CMND: %s\n", cmnd);
    int sys_stat = system(cmnd);
    if(sys_stat == -1) {
        printf("Cannot execute split-file command!\n");
        exit(1);
    }
}

void merge_files(char *addrs, int numblocks, char *target_file) {
    int sprint_stat;
    char cmnd[2048], files[2048], file[20];
    bzero(files, 2048);

    for(int i=0; i<numblocks; i++) {
        // Iterating through each block, and forming the string
        bzero(file, 20);
        if(i < 10)
            snprintf(file, sizeof(file), "%s0%d ", addrs, i);
        else    
            snprintf(file, sizeof(file), "%s%d ", addrs, i);
        strcat(files, file);
    }

    bzero(cmnd, 2048);
    sprint_stat = snprintf(cmnd, sizeof(cmnd), 
        "cat %s > %s", files, target_file);
    if(sprint_stat == -1) {
        printf("Snprintf error!\n");
        exit(1);
    }
    int sys_stat = system(cmnd);
    if(sys_stat == -1) {
        printf("Cannot execute merge-file command!\n");
        exit(1);
    }

    bzero(cmnd, 2048);
    sprint_stat = snprintf(cmnd, sizeof(cmnd), 
        "chmod 777 %s", target_file);
    if(sprint_stat == -1) {
        printf("Snprintf error!\n");
        exit(1);
    }

    sys_stat = system(cmnd);
    if(sys_stat == -1) {
        printf("Cannot execute give-permission command!\n");
        exit(1);
    }

    bzero(cmnd, 2048);
    sprint_stat = snprintf(cmnd, sizeof(cmnd), 
        "rm %s", files);
    if(sprint_stat == -1) {
        printf("Snprintf error!\n");
        exit(1);
    }   
    sys_stat = system(cmnd);
    if(sys_stat == -1) {
        printf("Cannot execute remove-blocks command!\n");
        exit(1);
    }
}

/*
FOR DEBUGGING PURPOSE ONLY
int main() {
    //split_file("test-file.txt", 5000);
    merge_files("test-file.txt", 3, "new_file.txt");
    return 0;
}
*/