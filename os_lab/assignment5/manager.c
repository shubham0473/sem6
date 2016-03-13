#include <stdio.h>
#include <string.h>

#define MAX_TRAINS 100;

int main(int argc, char* argv[]){
    FILE *fp;
    char sequence[MAX_TRAINS]

    if(argc < 2){
        printf("Please provide the input sequence\n");
        exit(-1);
    }

    fp = fopen(argv[1], 'r');
    if(fp == NULL)
    {
        perror("Error: ");
        return(-1);
    }



    return 0;
}
