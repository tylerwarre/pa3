#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

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
    //execution times for different frequencies from highest to lowest
    int time[4];
    int exec_1188;
    int exec_918;
    int exec_648;
    int exec_384;
    //energy use for different frequencies from highest to lowest
    int energy[4];
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
                                     :(utilization > processes * (pow(2, 1 / processes) - 1));
        return missedDeadline;

    }

    //flag to check if any possible schedule is found
    int noPossibleSchedule = 1;
    for(int i = 0; i < processes; i++) {
        procs[i].energy[0] = w_1188*procs[i].exec_1188;
        procs[i].energy[1] = w_918*procs[i].exec_918;
        procs[i].energy[2] = w_648*procs[i].exec_648;
        procs[i].energy[3] = w_384*procs[i].exec_384;

        procs[i].time[0] = procs[i].exec_1188;
        procs[i].time[1] = procs[i].exec_918;
        procs[i].time[2] = procs[i].exec_648;
        procs[i].time[3] = procs[i].exec_384;
    }

    int lowestEnergy = INT_MAX;
    //arrays for frequency, energy use, and utilization for each process
    int freq[5];
    int energy[5];
    double utilization[5];
    int totalEnergy;
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
                            energy[i] = procs[i].energy[freq[i]];
                            //utilization for all process
                            utilization[i] = (double)procs[i].time[freq[i]] / procs[i].period;

                            totalEnergy += energy[i];
                            totalUtilization +=utilization[i];
                        }

                        //check if schedule misses any deadlines by checking utilization ratio
                        missedDeadlines = isEDF ? (totalUtilization > 1)
                                                : (totalUtilization > 0.7435);

                        if(missedDeadlines) break;
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
                                procs[i].optimal_time = procs[i].time[freq[i]];
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
        sim.min = simTime * 2;
        sim.isIdle = 1;

        // for (int i = 0; i < processes; i++) {
        //     if (time > procs[i].stop_deadline) {
        //         printf("Deadline missed for %s\n", procs[i].name);
        //     }
        // }

        for (int i = 0; i < processes; i++) {
            if (procs[i].stop_deadline < sim.min && time >= procs[i].start_deadline) {
                sim.currProcess = i;
                sim.min = procs[i].stop_deadline;
                sim.isIdle = 0;
            }
        }

        // if (time > 332 && time < 343) {
        //     printf("\nDEBUG\n");
        //     for (int y = 0; y < processes; y++) {
        //         printf("%-4d: %-2s, %-4d, %-4d, %-4d\n", time, procs[y].name, procs[y].exec_time, procs[y].start_deadline, procs[y].stop_deadline);
        //     }
        //     printf("DEBUG\n");
        // }

        if (sim.isIdle) {
            sim.currProcess = processes + 1;
        }


        if (sim.currProcess != sim.pastProcess) {
            if (sim.isIdle) {
                printf("%-4d: IDLE\n", time);
            }
            else {
                printf("%-4d: Context switch to %s\n", time, procs[sim.currProcess].name);
            }
            
        }

        procs[sim.currProcess].exec_time--;

        if (procs[sim.currProcess].exec_time <= 0) {
            if (isEE) {
                // EE magic here
            }
            else {
                procs[sim.currProcess].exec_time = procs[sim.currProcess].exec_1188;
                procs[sim.currProcess].start_deadline += procs[sim.currProcess].period;
                procs[sim.currProcess].stop_deadline += procs[sim.currProcess].period;
            }
        }

        sim.pastProcess = sim.currProcess;
    }
}

void rm(int isEE, struct Process* procs) {
    //set a cpu frequency for each process
    int error = findOptimalFreq(procs, 0, isEE);

    if(error){
        printf("no feasible schedule could be found");
        return;
    }


    struct Simulation sim;
    sim.currProcess = 0;
    sim.pastProcess = 0;
    double energy = 0.0;
    int startTime = 1;
    int idle = 0;
    sim.min = INT_MAX;
    for(int time = 1; time < simTime; time++){

        //find process with shortest period to run
        for (int i = 0; i < processes; i++) {
            if (procs[i].period < sim.min && time >= procs[i].start_deadline && i != sim.currProcess) {
                sim.currProcess = i;
                sim.min = procs[i].period;
                idle = 0;
            }
        }
        if(idle) sim.currProcess = -1;



        //switch context if new process is running
        if (sim.currProcess != sim.pastProcess && time !=1) {
            if(sim.pastProcess == -1){
                printf("%-4d:   IDLE   IDLE   %d   %lf\n", startTime, time - 1, energy);
            }
            else {
                printf("%-4d:   %s   %d   %d   %lf\n", startTime, procs[sim.pastProcess].name,
                       procs[sim.pastProcess].optimal_freq, time-startTime, energy);
                startTime = time;
                energy = 0;
            }
        }

        //execute process and add energy used during execution
        if(idle){
            energy += w_idle*0.001;
        }
        else {
            procs[sim.currProcess].exec_time--;
            switch (procs[sim.currProcess].optimal_freq) {
                case 1188:
                    energy += w_1188*0.001;
                    break;
                case 918:
                    energy += w_918*0.001;
                    break;
                case 648:
                    energy += w_648*0.001;
                    break;
                case 384:
                    energy += w_384*0.001;
                    break;
            }
        }

        //reschedule process if finished
        if(!idle && procs[sim.currProcess].exec_time <= 0){
            procs[sim.currProcess].exec_time = procs[sim.currProcess].optimal_time;
            procs[sim.currProcess].start_deadline += procs[sim.currProcess].period;
            procs[sim.currProcess].stop_deadline += procs[sim.currProcess].period;
            sim.min = INT_MAX;
            idle = 1;
        }

        sim.pastProcess = sim.currProcess;

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