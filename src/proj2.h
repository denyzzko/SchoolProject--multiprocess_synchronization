#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/mman.h>

// input arguments
int skier_count;
int busstop_count;
int bus_capacity;
int skier_wait;
int travel_time;

// variable to create random starting stop for skier
int random_busstop;

//file proj2.out
FILE *file;

// shared variables and semaphores
int *line_count;
int *skier_id;
int *current_busstop;
int *waitingskier_count;
bool *boarding;
int *busstop_waiting_count;
int *current_buscapacity;
sem_t *line_count_m;
sem_t *skier_id_m;
sem_t *boarding_m;
sem_t *capacity_m;
sem_t *current_busstop_m;
sem_t *waitingskier_count_m;
sem_t *busstop_waiting_count_m;
sem_t *current_buscapacity_m;

// child processes
pid_t process_bus;
pid_t process_skier;

// function to destroy semaphores, clean allocated resources and close file proj2.out
void clean_up(int busstop_count);

// function to validate correct ranges of input arguments
void validate_parameters(int skier_count, int busstop_count, int bus_capacity, int skier_wait, int travel_time);

// function to handle process for skibus
void handlingBus(int * line_count, int * current_busstop, int busstop_count, int travel_time, int *waitingskier_count, bool *boarding, int *busstop_waiting_count, int *current_buscapacity, int bus_capacity);

// function to handle process for skier
void handlingSkier(int * line_count, int * skier_id, int random_busstop, int * current_busstop, int *waitingskier_count, bool *boarding, int busstop_count, int *busstop_waiting_count, int *current_buscapacity);