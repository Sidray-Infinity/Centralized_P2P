#include <stdio.h>
#include <stdlib.h>

struct threeNum
{
    int n1, n2, n3;
};

void show_DHT(struct file_node DHT[])
{
    printf("---------------------------------------------\n");
    printf("BLOCK DISTRIBUTION TABLE\n\n");
    for (int i = 0; i < 100; i++)
    {
        if (DHT[i].block_arr != NULL)
        {
            printf("FILENAME: %s \n", DHT[i].filename);
            for (struct block *q = DHT[i].block_arr; q != NULL; q = q->next)
                printf("\tBLOCKNAME: %s IP: %s PORT: %d\n",
                       q->block_name,
                       q->loc.ip,
                       q->loc.port);
            printf("\n");
        }
    }
    printf("---------------------------------------------\n");
}

int main()
{
    int n;
    struct threeNum num;
    FILE *fptr;

    if ((fptr = fopen("test.c", "rb")) == NULL)
    {
        printf("Error! opening file");

        // Program exits if the file pointer returns NULL.
        exit(1);
    }

    while (1)
    {
        fread(&n, sizeof(int), 1, fptr);
        printf("%d\n", n);

        getchar();
    }

    fclose(fptr);

    return 0;
}