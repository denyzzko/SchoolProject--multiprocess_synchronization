/**
 * @file proj2.c
 * @brief Multiprocess synchronization.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 22.4.2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "proj2.h"

void clean_up(int stage, int busstop_count){
    if (stage >= 0) munmap(line_count,sizeof(int));
	if (stage >= 1) munmap(line_count_m,sizeof(sem_t));
    if (stage >= 2) munmap(skier_id,sizeof(int));
	if (stage >= 3) munmap(skier_id_m,sizeof(sem_t));
    if (stage >= 4) munmap(current_busstop,sizeof(int));
    if (stage >= 5) munmap(boarding,sizeof(bool));
    if (stage >= 6) munmap(boarding_m,sizeof(sem_t));
    if (stage >= 7) munmap(capacity_m,sizeof(sem_t));
    if (stage >= 8) munmap(current_busstop_m,sizeof(sem_t));
    if (stage >= 9) munmap(waitingskier_count,sizeof(int));
    if (stage >= 10) munmap(waitingskier_count_m,sizeof(sem_t));
    if (stage >= 11) munmap(busstop_waiting_count,sizeof(int)*busstop_count);
    if (stage >= 12) munmap(busstop_waiting_count_m,sizeof(sem_t));
    if (stage >= 13) munmap(current_buscapacity,sizeof(int));
    if (stage >= 14) munmap(current_buscapacity_m,sizeof(sem_t));
    if (stage >= 15) sem_destroy(line_count_m);
    if (stage >= 16) sem_destroy(skier_id_m);
    if (stage >= 17) sem_destroy(boarding_m);
    if (stage >= 18) sem_destroy(capacity_m);
    if (stage >= 19) sem_destroy(current_busstop_m);
    if (stage >= 20) sem_destroy(waitingskier_count_m);
    if (stage >= 21) sem_destroy(busstop_waiting_count_m);
    if (stage >= 22) sem_destroy(current_buscapacity_m);

    fclose(file);
}

void validate_parameters(int skier_count, int busstop_count, int bus_capacity, int skier_wait, int travel_time) {
    if (skier_count < 0 || skier_count >= 20000) {
        fprintf(stderr, "Error: Number of skiers 'L' must be between 0 and 19999.\n");
        exit(1);
    }
    if (busstop_count <= 0 || busstop_count > 10) {
        fprintf(stderr, "Error: Number of stops 'Z' must be between 1 and 10.\n");
        exit(1);
    }
    if (bus_capacity < 10 || bus_capacity > 100) {
        fprintf(stderr, "Error: Capacity of skibus 'K' must be between 10 and 100.\n");
        exit(1);
    }
    if (skier_wait < 0 || skier_wait > 10000) {
        fprintf(stderr, "Error: Maximum wait time 'TL' must be between 0 and 10000 microseconds.\n");
        exit(1);
    }
    if (travel_time < 0 || travel_time > 1000) {
        fprintf(stderr, "Error: Maximum travel time 'TB' between stops must be between 0 and 1000.\n");
        exit(1);
    }
}

void handlingBus(int * line_count, int * current_busstop, int busstop_count, int travel_time, int *waitingskier_count, bool *boarding, int *busstop_waiting_count, int *current_buscapacity, int bus_capacity){
    //starting bus
    sem_wait(line_count_m);
    fprintf(file, "%d: BUS: started\n", *line_count);
    (*line_count)++;
    sem_post(line_count_m);

    sleep(2);

    while(true){
        if (*current_busstop <= busstop_count ){
            //going to stop
            srand(time(NULL));
            usleep(rand() % (travel_time + 1));
            sem_wait(line_count_m);
            fprintf(file, "%d: BUS: arrived to %d\n", *line_count, *current_busstop);
            (*line_count)++;
            sem_post(line_count_m);
            
            //allowing skiers to board
            sem_wait(boarding_m);
            *boarding = true;
            sem_post(boarding_m);
            
            //waiting for skiers to board
            while(true){
                sem_wait(busstop_waiting_count_m);
                sem_wait(current_buscapacity_m);
                if (busstop_waiting_count[(*current_busstop) - 1] == 0 || *current_buscapacity == 0){
                    sem_post(current_buscapacity_m);
                    sem_post(busstop_waiting_count_m);
                    break;
                }
                else {
                    sem_post(current_buscapacity_m);
                    sem_post(busstop_waiting_count_m);
                    continue;
                }
            }

            //leaving stop
            sem_wait(boarding_m);
            *boarding = false;

            sem_wait(current_busstop_m);
            sem_wait(line_count_m);
            fprintf(file, "%d: BUS: leaving %d\n", *line_count, *current_busstop);
            (*line_count)++;
            sem_post(line_count_m);
            (*current_busstop)++;
            sem_post(current_busstop_m);

            sem_post(boarding_m);
        }
        else{
            //going to final
            srand(time(NULL));
            usleep(rand() % (travel_time + 1));
            sem_wait(line_count_m);
            fprintf(file, "%d: BUS: arrived to final\n", *line_count);
            (*line_count)++;
            sem_post(line_count_m);

            //allowing skiers to aboard
            sem_wait(boarding_m);
            *boarding = true;
            sem_post(boarding_m);
            
            //waiting for skiers to aboard
            while(true){
                sem_wait(current_buscapacity_m);
                if (*current_buscapacity == bus_capacity){
                    sem_post(current_buscapacity_m);
                    break;
                }
                else {
                    sem_post(current_buscapacity_m);
                    continue;
                }
            }

            //permitting skiers to aboard
            sem_wait(boarding_m);
            *boarding = false;

            //leaving final
            sem_wait(line_count_m);
            fprintf(file, "%d: BUS: leaving final\n", *line_count);
            (*line_count)++;
            sem_post(line_count_m);

            sem_post(boarding_m);

            //check if any skier is still waiting at stop
            sem_wait(waitingskier_count_m);
            if (*waitingskier_count > 0){
                sem_post(waitingskier_count_m);
                sem_wait(current_busstop_m);
                *current_busstop = 1;
                sem_post(current_busstop_m);
                continue;
            }
            else{
                sem_post(waitingskier_count_m);
                break;

            }
        }
    }
    //finishing bus
    sem_wait(line_count_m);
    fprintf(file, "%d: BUS: finish\n", *line_count);
    (*line_count)++;
    sem_post(line_count_m);
}

void handlingSkier(int * line_count, int * skier_id, int random_busstop, int * current_busstop, int *waitingskier_count, bool *boarding, int busstop_count, int *busstop_waiting_count, int *current_buscapacity){
    int currentskier_id = *skier_id;
    (*skier_id)++;
    sem_post(skier_id_m);

    //starting skier
    sem_wait(line_count_m);
    fprintf(file, "%d: L %d: started\n", *line_count, currentskier_id);
    (*line_count)++;
    sem_post(line_count_m);

    //finishing breakfast
    srand(time(NULL));
    usleep(rand() % (skier_wait + 1));
    
    //arriving to stop
    sem_wait(line_count_m);
    fprintf(file, "%d: L %d: arrived to %d\n", *line_count, currentskier_id, random_busstop);
    (*line_count)++;
    
    sem_wait(busstop_waiting_count_m);
    busstop_waiting_count[(random_busstop) - 1]++;
    sem_post(busstop_waiting_count_m);

    sem_post(line_count_m);


    //waiting for bus to arrive
    while (true){
        sem_wait(current_busstop_m);
        if(*current_busstop == random_busstop){
            sem_post(current_busstop_m);
            sem_wait(boarding_m);
            if (*boarding == true){
                //skier can try to board
                sem_post(boarding_m);
                sem_wait(capacity_m);

                //check if bus is still at same busstop and available to board (case where bus capacity is available for next bus run)
                sem_wait(current_busstop_m);
                if (*current_busstop == random_busstop){
                    sem_post(current_busstop_m);
                    sem_wait(boarding_m);
                    if (*boarding == true){
                        //skier can board
                        sem_post(boarding_m);
                        sem_wait(line_count_m);
                        fprintf(file, "%d: L %d: boarding\n", *line_count, currentskier_id);
                        (*line_count)++;

                        sem_wait(waitingskier_count_m);
                        (*waitingskier_count)--;
                        sem_post(waitingskier_count_m);

                        sem_wait(busstop_waiting_count_m);
                        busstop_waiting_count[(random_busstop) - 1]--;
                        sem_post(busstop_waiting_count_m);

                        sem_wait(current_buscapacity_m);
                        (*current_buscapacity)--;
                        sem_post(current_buscapacity_m);

                        sem_post(line_count_m);

                        break;   
                    }
                    else{
                        //skier didnt make it to bus and has to wait for next arrival
                        sem_post(boarding_m);
                        sem_post(capacity_m);
                        continue;
                    }
                }
                else{
                    //skier didnt make it to bus and has to wait for next arrival
                    sem_post(current_busstop_m);
                    sem_post(capacity_m);
                    continue;
                }
            }
            else{
                //boarding is not allowed
                sem_post(boarding_m);
                continue;
            }
        }
        else{
            //bus is not at skiers stop
            sem_post(current_busstop_m);
            continue;
        }
    }
    
    //waiting to arrive to final stop
    while (true){
        sem_wait(current_busstop_m);
        if(*current_busstop == (busstop_count) + 1){
            //skier has arrived to final stop
            sem_post(current_busstop_m);
            sem_wait(boarding_m);
            if (*boarding == true){
                //skier can aboard and go to ski
                sem_post(boarding_m);
                sem_wait(line_count_m);
                fprintf(file, "%d: L %d: going to ski\n", *line_count, currentskier_id);
                (*line_count)++;

                sem_wait(current_buscapacity_m);
                (*current_buscapacity)++;
                sem_post(current_buscapacity_m);

                sem_post(capacity_m);

                sem_post(line_count_m);
            
                break;
            }
            else{
                //aboarding is not allowed
                sem_post(boarding_m);
                continue;
            }
        }
        else{
            //bus has not arrived to final stop yet
            sem_post(current_busstop_m);
            continue;
        }
    }
}

int main(int argc, char *argv[]) {
    // check if the correct number of arguments is passed
    if (argc != 6) {
        fprintf(stderr, "Error: Format should be: ./proj2 L Z K TL TB\n");
        exit(1);
    }

    // parse arguments
    skier_count = atoi(argv[1]);
    busstop_count = atoi(argv[2]);
    bus_capacity = atoi(argv[3]);
    skier_wait = atoi(argv[4]);
    travel_time = atoi(argv[5]);
    validate_parameters(skier_count, busstop_count, bus_capacity, skier_wait, travel_time);
    
    // open file
    file = fopen("proj2.out", "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file proj2.out.\n");
        exit(1);
    }

    setbuf(file, NULL);

    //initialize shared variables
    line_count = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (line_count == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for line_count\n");
        clean_up(0, busstop_count);
        exit(1);
    }
    line_count_m = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (line_count_m == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for line_count_m\n");
        clean_up(1, busstop_count);
        exit(1);
    }
    skier_id = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (skier_id == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for skier_id\n");
        clean_up(2, busstop_count);
        exit(1);
    }
    skier_id_m = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (skier_id_m == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for skier_id_m\n");
        clean_up(3, busstop_count);
        exit(1);
    }
    current_busstop = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (current_busstop == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for current_busstop\n");
        clean_up(4, busstop_count);
        exit(1);
    }
    boarding = mmap(NULL, sizeof(bool), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (boarding == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for boarding\n");
        clean_up(5, busstop_count);
        exit(1);
    }
    boarding_m = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (boarding_m == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for boarding_m\n");
        clean_up(6, busstop_count);
        exit(1);
    }
    capacity_m = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (capacity_m == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for capacity_m\n");
        clean_up(7, busstop_count);
        exit(1);
    }
    current_busstop_m = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (line_count == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for current_busstop_m\n");
        clean_up(8, busstop_count);
        exit(1);
    }
    waitingskier_count = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (waitingskier_count == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for waitingskier_count\n");
        clean_up(9, busstop_count);
        exit(1);
    }
    waitingskier_count_m = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (waitingskier_count_m == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for waitingskier_count_m\n");
        clean_up(10, busstop_count);
        exit(1);
    }
    busstop_waiting_count = mmap(NULL, sizeof(int)*busstop_count, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (line_count == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for busstop_waiting_count\n");
        clean_up(11, busstop_count);
        exit(1);
    }
    busstop_waiting_count_m = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (busstop_waiting_count_m == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for busstop_waiting_count_m\n");
        clean_up(12, busstop_count);
        exit(1);
    }
    current_buscapacity = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (current_buscapacity == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for current_buscapacity\n");
        clean_up(13, busstop_count);
        exit(1);
    }
    current_buscapacity_m = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    if (current_buscapacity_m == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() failed for current_buscapacity_m\n");
        clean_up(14, busstop_count);
        exit(1);
    }

    line_count[0] = 1;
    skier_id[0] = 1;
    current_busstop[0] = 1;
    boarding[0] = false;
    waitingskier_count[0] = skier_count;
    current_buscapacity[0] = bus_capacity;

    //initialize semaphores
    if (sem_init(line_count_m, 1, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed.\n");
        clean_up(15, busstop_count);
	    exit(1);
    }
    if (sem_init(skier_id_m, 1, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed.\n");
        clean_up(16, busstop_count);
	    exit(1);
    }
    if (sem_init(boarding_m, 1, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed.\n");
        clean_up(17, busstop_count);
	    exit(1);
    }
    if (sem_init(capacity_m, 1, bus_capacity) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed.\n");
        clean_up(18, busstop_count);
	    exit(1);
    }
    if (sem_init(current_busstop_m, 1, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed.\n");
        clean_up(19, busstop_count);
	    exit(1);
    }
    if (sem_init(waitingskier_count_m, 1, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed.\n");
        clean_up(20, busstop_count);
	    exit(1);
    }
    if (sem_init(busstop_waiting_count_m, 1, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed.\n");
        clean_up(21, busstop_count);
	    exit(1);
    }
    if (sem_init(current_buscapacity_m, 1, 1) != 0) {
        fprintf(stderr, "Error: Semaphore initialization failed.\n");
        clean_up(22, busstop_count);
	    exit(1);
    }

    // create process for skibus
    process_bus = fork();
    if (process_bus < 0){
        fprintf(stderr, "Error: Failed to create process bus.\n");
        clean_up(22, busstop_count);
        exit(1);
    }
    else if (process_bus == 0){
        handlingBus(line_count, current_busstop, busstop_count, travel_time, waitingskier_count, boarding, busstop_waiting_count, current_buscapacity, bus_capacity);
        exit(0);
    }
    // create processes for skiers
    for (int i = 0; i < skier_count; i++){
        process_skier = fork();
        if (process_skier < 0){
            fprintf(stderr, "Error: Failed to create process skier.\n");
            clean_up(22, busstop_count);
            exit(1);
        }
        else if (process_skier == 0){
            sem_wait(skier_id_m);
            //does this count as: "Hlavni proces každému lyžaři při jeho spuštění náhodně přidělí nástupní zastávku"
            // seed the random number
            srand(time(NULL) + getpid());
            random_busstop = (rand() % busstop_count) + 1;
            handlingSkier(line_count, skier_id, random_busstop, current_busstop, waitingskier_count, boarding, busstop_count, busstop_waiting_count, current_buscapacity);
            sem_post(skier_id_m);
            exit(0);
        }
    }

    // wait for child processes
    while(wait(NULL) > 0);

    clean_up(22, busstop_count);

    return 0;
}