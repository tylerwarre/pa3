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

struct Process {
    char name[5];
    int period;
    int exec_1188;
    int exec_918;
    int exec_648;
    int exec_384;
    int exec_time; 
    int start_deadline;
    int stop_deadline;
};

struct Simulation {
    int min;
    int currProcess;
    int pastProcess;
};

struct Process* init(char file[]) {
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
    struct Process *procs = malloc(sizeof(struct Process) * processes);

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
                    procs[row].start_deadline = 1;
                    procs[row].stop_deadline = atoi(attribute);
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

void edf(int isEE, struct Process* procs) {
    if (isEE) {
        printf("Starting EDF simulation in EE mode\n");
    }
    else {
        printf("Starting EDF simulation in standard mode\n");
        // Set max CPU frequency for all processes
        for (int i = 0; i < processes; i++) {
            procs[i].exec_time = procs[i].exec_1188;
        }
    }

    double edfCalc = 0.0;
    for (int i = 0; i < processes; i++) {
        edfCalc += procs[i].exec_time / procs[i].period;
    }

    if (edfCalc >= 1) {
        printf("No schedule can be found with EDF\n");
        exit(0);
    }

    struct Simulation sim;

    for (int time = 1; time < simTime; time++) {
        sim.currProcess = 0;
        sim.min = procs[0].stop_deadline;
        for (int i = 1; i < processes; i++) {
            if (procs[i].stop_deadline < sim.min && time >= procs[i].start_deadline) {
                sim.currProcess = i;
                sim.min = procs[i].stop_deadline;
            }
        }

        if (sim.currProcess != sim.pastProcess) {
            printf("%-4d: Context switch to %s\n", time, procs[sim.currProcess].name);
        }

        procs[sim.currProcess].exec_time--;

        if (procs[sim.currProcess].exec_time <= 0) {
            if (isEE) {
                // EE magic here
            }
            else {
                procs[sim.currProcess].exec_time = procs[sim.currProcess].exec_1188;
                procs[sim.currProcess].start_deadline += procs[sim.currProcess].period + 1;
                procs[sim.currProcess].stop_deadline += procs[sim.currProcess].period + 1;
            }
        }

        sim.pastProcess = sim.currProcess;
    }
}

void rm(int isEE, struct Process* procs) {
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

    struct Process* procs = init(argv[1]);

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