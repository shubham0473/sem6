#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM 0
#define TRAIN 1
#define AC 2
#define QTY 3
#define PREF 4
#define AGE 5

char* field[6];

const char* getfield(char* line)
{
    const char* tok;
	int i;
    for (tok = strtok(line, ","), i = 0;
            tok && *tok;
            tok = strtok(NULL, ",\n"), i++)
    {
        field[i] = (char *) tok;
    }
    return NULL;
}

int main()
{
    FILE* stream = fopen("Booking.csv", "r");

    char line[1024];
    while (fgets(line, 1024, stream))
    {	
        char* tmp = strdup(line);
		getfield(tmp);
        printf("Number: %s, Train: %s, AC: %s, QTY: %s, PREF: %s, AGE: %s\n", field[NUM], field[TRAIN], field[AC], field[QTY], field[PREF], field[AGE]);
        // NOTE strtok clobbers tmp
        free(tmp);
    }
}
