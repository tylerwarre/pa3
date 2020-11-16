#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

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
    int optimal_freq;
    int optimal_time;
    int start_deadline;
    int stop_deadline;
};

struct Simulation {
    int min;
    int currProcess;
    int pastProcess;
    int isIdle;
    int startTime;
    double currEnergy;
    double totalEnergy;
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


//returns energy used in a certain time period, returns -1 if schedule is impossible
double simulateEnergy(struct Process* procs, int freq[processes], int isEDF){

    //copy processes
    struct Process procsCopy[processes];
    memcpy(procsCopy, procs, sizeof(struct Process)*processes);

    //assign frequency and exec time for each process
    for(int i = 0; i <processes; i++){
        switch(freq[i]){
            case 0:
                procsCopy[i].optimal_time = procsCopy[i].exec_1188;
                procsCopy[i].optimal_freq = 1188;
                break;
            case 1:
                procsCopy[i].optimal_time = procsCopy[i].exec_918;
                procsCopy[i].optimal_freq = 918;
                break;
            case 2:
                procsCopy[i].optimal_time = procsCopy[i].exec_648;
                procsCopy[i].optimal_freq = 648;
                break;
            case 3:
                procsCopy[i].optimal_time = procsCopy[i].exec_384;
                procsCopy[i].optimal_freq = 384;
                break;
        }
        procsCopy[i].exec_time = procsCopy[i].optimal_time;
    }

    //simulate schedule
    struct Simulation sim;
    sim.totalEnergy = 0.0;
    sim.currEnergy = 0.0;
    sim.startTime = 1;
    sim.pastProcess = 0;

    for (int time = 1; time < simTime; time++) {
        sim.currProcess = 0;
        sim.min = simTime * 2;
        sim.isIdle = 1;

        for (int i = 0; i < processes; i++) {

            //return error if any deadline was missed
            if(procsCopy[i].stop_deadline < time) return -1;
            if(isEDF) {
                if (procsCopy[i].stop_deadline < sim.min && time >= procsCopy[i].start_deadline) {
                    sim.currProcess = i;
                    sim.min = procsCopy[i].stop_deadline;
                    sim.isIdle = 0;
                }
            }
            else{
                if (procsCopy[i].period < sim.min && time >= procsCopy[i].start_deadline) {
                    sim.currProcess = i;
                    sim.min = procsCopy[i].period;
                    sim.isIdle = 0;
                }
            }
        }

        if (sim.isIdle) {
            sim.currProcess = processes + 1;
        }


        if (sim.currProcess != sim.pastProcess && time != 1) {
            sim.startTime = time;
            sim.totalEnergy += sim.currEnergy;
            sim.currEnergy = 0;
        }

        //execute process and add energy used during execution
        if(sim.isIdle){
            sim.currEnergy += w_idle*0.001;
        }
        else {
            procsCopy[sim.currProcess].exec_time--;
            switch (procsCopy[sim.currProcess].optimal_freq) {
                case 1188:
                    sim.currEnergy += w_1188*0.001;
                    break;
                case 918:
                    sim.currEnergy += w_918*0.001;
                    break;
                case 648:
                    sim.currEnergy += w_648*0.001;
                    break;
                case 384:
                    sim.currEnergy += w_384*0.001;
                    break;
            }
        }

        if (procsCopy[sim.currProcess].exec_time <= 0) {
            procsCopy[sim.currProcess].exec_time = procsCopy[sim.currProcess].optimal_time;
            procsCopy[sim.currProcess].start_deadline += procsCopy[sim.currProcess].period;
            procsCopy[sim.currProcess].stop_deadline += procsCopy[sim.currProcess].period;
        }

        sim.pastProcess = sim.currProcess;
    }
    sim.totalEnergy += sim.currEnergy;
    return sim.totalEnergy;
}


// brute-force function to determine optimal frequency for each process
// Returns 1 if a schedule is impossible with no missed deadlines, 0 otherwise
int findOptimalFreq(struct Process *procs, int isEDF, int isEE) {

    if(!isEE){
        //assign all frequencies to max
        for(int i = 0; i < processes; i++) {
            procs[i].optimal_freq = 1188;
            procs[i].optimal_time = procs[i].exec_1188;
        }

        //check is schedule is possible with no missed deadlines
        double utilization = 0.0;
        for (int i = 0; i < processes; i++) {
            utilization += (double)procs[i].exec_time / procs[i].period;
        }

        int missedDeadline = isEDF?(utilization > 1)
                                     :(utilization > 0.7435);
        return missedDeadline;

    }

    int energy[processes][4];
    int time[processes][4];
    //flag to check if any possible schedule is found
    int noPossibleSchedule = 1;
    for(int i = 0; i < processes; i++) {
        energy[i][0] = w_1188*procs[i].exec_1188;
        energy[i][1] = w_918*procs[i].exec_918;
        energy[i][2] = w_648*procs[i].exec_648;
        energy[i][3] = w_384*procs[i].exec_384;

        time[i][0] = procs[i].exec_1188;
        time[i][1] = procs[i].exec_918;
        time[i][2] = procs[i].exec_648;
        time[i][3] = procs[i].exec_384;
    }

    double lowestEnergy = DBL_MAX;
    //arrays for frequency, energy use, and utilization for each process
    int freq[5];
    double utilization[5];
    double totalEnergy;
    double totalUtilization;
    int missedDeadlines;
    //nested loops check every possible combination of frequencies
    for(freq[0] = 0; freq[0] < 4; freq[0]++){
        for(freq[1] = 0; freq[1] < 4; freq[1]++){
            for(freq[2] = 0; freq[2] < 4; freq[2]++){
                for(freq[3] = 0; freq[3] < 4; freq[3]++){
                    for(freq[4] = 0; freq[4] < 4; freq[4]++) {

                        totalEnergy = 0;
                        totalUtilization = 0;
                        for(int i = 0; i < 5; i++){
                            //energy use for all process
                            //utilization for all process
                            utilization[i] = (double)time[i][freq[i]] / procs[i].period;

                            totalEnergy += energy[i][freq[i]];
                            totalUtilization +=utilization[i];
                        }

                        if(totalUtilization > 1)break;
                        totalEnergy = simulateEnergy(procs, freq, isEDF);

                        if(totalEnergy == -1) break;
                        if(totalEnergy < lowestEnergy) {
                            noPossibleSchedule = 0;
                            lowestEnergy = totalEnergy;

                            for (int i = 0; i < 5; i++) {
                                switch(freq[i]){
                                    case 0:
                                        procs[i].optimal_freq = 1188;
                                        break;
                                    case 1:
                                        procs[i].optimal_freq = 918;
                                        break;
                                    case 2:
                                        procs[i].optimal_freq = 648;
                                        break;
                                    case 3:
                                        procs[i].optimal_freq = 384;
                                        break;
                                }
                                procs[i].optimal_time = time[i][freq[i]];
                                procs[i].exec_time = procs[i].optimal_time;
                            }
                        }
                    }
                }
            }
        }
    }
    return noPossibleSchedule;
}


void edf(int isEE, int isEDF, struct Process* procs) {
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

    int error = findOptimalFreq(procs, isEDF, isEE);

    if(error){
        printf("No feasible schedule could be found\n");
        return;
    }

    struct Simulation sim;
    sim.totalEnergy = 0.0;
    sim.currEnergy = 0.0;
    sim.startTime = 1;
    sim.pastProcess = 0;

    for (int time = 1; time < simTime; time++) {
        sim.currProcess = 0;
        sim.min = simTime * 2;
        sim.isIdle = 1;

        if (isEDF) {
            for (int i = 0; i < processes; i++) {
                if (procs[i].stop_deadline < sim.min && time >= procs[i].start_deadline) {
                    sim.currProcess = i;
                    sim.min = procs[i].stop_deadline;
                    sim.isIdle = 0;
                }
            }
        }
        else {
            for (int i = 0; i < processes; i++) {
                if (procs[i].period < sim.min && time >= procs[i].start_deadline) {
                    sim.currProcess = i;
                    sim.min = procs[i].period;
                    sim.isIdle = 0;
                }
            }
        }

        if (sim.isIdle) {
            sim.currProcess = processes + 1;
        }


        if (sim.currProcess != sim.pastProcess && time != 1) {
            if (sim.pastProcess == processes + 1) {
                printf("%-4d %-4s %-4s %-4d %-4f \n", sim.startTime, "IDLE", "IDLE", time - sim.startTime, sim.currEnergy);
            }
            else {
                printf("%-4d %-4s %-4d %-4d %-4f \n", sim.startTime, procs[sim.pastProcess].name, procs[sim.pastProcess].optimal_freq, time - sim.startTime, sim.currEnergy);
            }
            sim.startTime = time;
            sim.totalEnergy += sim.currEnergy;
            sim.currEnergy = 0;
        }

        //execute process and add energy used during execution
        if(sim.isIdle){
            sim.currEnergy += w_idle*0.001;
        }
        else {
            switch (procs[sim.currProcess].optimal_freq) {
                case 1188:
                    sim.currEnergy += w_1188*0.001;
                    break;
                case 918:
                    sim.currEnergy += w_918*0.001;
                    break;
                case 648:
                    sim.currEnergy += w_648*0.001;
                    break;
                case 384:
                    sim.currEnergy += w_384*0.001;
                    break;
            }
        }

        if (!sim.isIdle) {
            procs[sim.currProcess].exec_time--;
        }

        if (procs[sim.currProcess].exec_time <= 0) {
            procs[sim.currProcess].exec_time = procs[sim.currProcess].optimal_time;
            procs[sim.currProcess].start_deadline += procs[sim.currProcess].period;
            procs[sim.currProcess].stop_deadline += procs[sim.currProcess].period;
        }

        sim.pastProcess = sim.currProcess;
    }
    
    if (sim.isIdle) {
        printf("%-4d %-4s %-4s %-4d %-4f \n", sim.startTime, "IDLE", "IDLE", (simTime + 1) - sim.startTime, sim.currEnergy);
    }
    else {
        printf("%-4d %-4s %-4d %-4d %-4f \n", sim.startTime, procs[sim.pastProcess].name, procs[sim.pastProcess].optimal_freq, (simTime + 1) - sim.startTime, sim.currEnergy);
    }

    printf("Total Energy: %-6f\n", sim.totalEnergy);
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
        if (strcmp(argv[3], "EE") == 0) {
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
        edf(isEE, 1, procs);
    }
    else if (strcmp(argv[2],"RM") == 0) {
        edf(isEE, 0, procs);
    }
    else {
        printf("Please select EDF or RM for the scheduling mode\n");
    }

    // Deallocate memory for structs
    free(procs);

    return 0;
}