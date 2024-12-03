// common.h

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <time.h> 

#define N 29    // Maksymalna liczba pasazerow na statku
#define K 15    // Maksymalna liczba pasazerow na mostku
#define T1 30     // Czas miedzy rejsami
#define T2 20    // Czas trwania rejsu
#define R 5     // Maksymalna liczba rejsow
#define NUM_PASSENGERS 30 // Liczba pasazerow

// Struktura pamieci dzielonej
typedef struct {
    int passengers_on_board;
    int passengers_on_bridge;
    int voyage_number;
    int loading; // 0 - brak zaladunku, 1 - zaladunek, 2 - rozladunek
    int boarding_allowed; // 0 - nie mozna wchodzic, 1 - mozna wchodzic
    int unloading_allowed; // 0 - nie mozna schodzic, 1 - mozna schodzic
    int loading_finished; // Flaga informujaca o zakonczeniu zaladunku
    int unloading_finished; // Flaga informujaca o zakonczeniu rozladunku
    int bridge_empty; // Flaga informujaca o zakonczeniu schodzenia z mostku
} SharedData;

// Deklaracje globalnych zmiennych
extern SharedData *shared_data;
extern int shm_id;
extern sem_t bridge_sem;
extern sem_t ship_sem;
extern pthread_mutex_t mutex;

extern pthread_cond_t queue_cond; 
extern pthread_mutex_t queue_mutex;
extern pthread_mutex_t voyage_mutex;
extern pthread_cond_t voyage_cond;
extern pthread_cond_t port_cond;
extern pthread_mutex_t port_mutex;
extern pthread_cond_t ship_cond;
extern pthread_mutex_t ship_mutex;
extern pthread_cond_t bridge_empty_cond;
extern pthread_mutex_t bridge_empty_mutex;

// Deklaracje funkcji
void init_shared_memory();
void init_semaphores();
void init_cond();
void enter_bridge(int passenger_id);
void enter_ship(int passenger_id);
void exit_ship(int passenger_id);
void exit_bridge(int passenger_id);
void *Passenger(void *arg);
void *PortCaptain(void *arg);
void *ShipCaptain(void *arg);

#endif // COMMON_H