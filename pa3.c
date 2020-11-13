#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int simTime;
int processes;
int w_1188;
int w_918;
int w_648;
int w_384;
int w_idle;

struct Poccess {
    char name[5];
    int period;
    int exec_1188;
    int exec_918;
    int exec_648;
    int exec_384;
    int start_dealine;
    int stop_dealine;
};

struct Poccess* init(char file[]) {
    FILE *fp;
    fp = fopen(file, "r");
    char line[255];
    char attribute[12];
    int col, row;

    if (fp == NULL) {
        exit(-1);
    }
    
    col = 0;
    while (col < 7) {
        fscanf(fp, "%s", attribute);
        
        switch(col) {
            case 0:
                processes = atoi(attribute);
                break;
            case 1:
                simTime = atoi(attribute);
                break;
            case 2:
                w_1188 = atoi(attribute);
                break;
            case 3:
                w_918 = atoi(attribute);
                break;
            case 4:
                w_648 = atoi(attribute);
                break;
            case 5:
                w_384 = atoi(attribute);
                break;
            case 6:
                w_idle = atoi(attribute);
                break;
        }
        col++;
    }

    printf("%d proccess detected with a simulation time %d\n", processes, simTime);

    // Allocate memory for structs on heap
    struct Poccess *procs = malloc(sizeof(struct Poccess) * processes);

    row = 0;
    while (row < processes) {
        col = 0;
        while (col < 6) {
            fscanf(fp, "%s", attribute);
        
            switch(col) {
                case 0:
                    strcpy(procs[row].name, attribute);
                    break;
                case 1:
                    procs[row].period = atoi(attribute);
                    procs[row].start_dealine = 0;
                    procs[row].stop_dealine = atoi(attribute);
                    break;
                case 2:
                    procs[row].exec_1188 = atoi(attribute);
                    break;
                case 3:
                    procs[row].exec_918 = atoi(attribute);
                    break;
                case 4:
                    procs[row].exec_648 = atoi(attribute);
                    break;
                case 5:
                    procs[row].exec_384 = atoi(attribute);
                    break;
            }
            col++;
        }
        row++;
    }
    printf("Init completed\n");

    return procs;
}

void edf(int isEE, struct Poccess* procs) {
    if (isEE) {
        printf("Starting EDF simulation in EE mode\n");
    }
    else {
        printf("Starting EDF simulation in standard mode\n");
    }
}

void rm(int isEE, struct Poccess* procs) {
    if (isEE) {
        printf("Starting RM simulation in EE mode\n");
    }
    else {
        printf("Starting RM simulation in standard mode\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        printf("Invalid number of arguments supplied\n");
        return 0;
    }

    struct Poccess* procs = init(argv[1]);

    int isEE;
    // Checks if in EE mode
    if (argc == 4) {
        if (argv[3] == "EE") {
            isEE = 1;
        }
        else {
            isEE = 0;
        }
    }
    else {
        isEE = 0;
    }

    // Checks scheduling type and launches simulation
    if (strcmp(argv[2],"EDF") == 0) {
        edf(isEE, procs);
    }
    else if (strcmp(argv[2],"RM") == 0) {
        rm(isEE, procs);
    }
    else {
        printf("Please select EDF or RM for the scheduling mode\n");
    }

    // Deallocate memory for structs
    free(procs);

    return 0;
}