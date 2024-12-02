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

#define N 100    // Maksymalna liczba pasazerow na statku
#define K 20     // Maksymalna liczba pasazerow na mostku
#define T1 60     // Czas miedzy rejsami
#define T2 120    // Czas trwania rejsu
#define R 2     // Maksymalna liczba rejsow
#define NUM_PASSENGERS 82 // Liczba pasazerow

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

// Deklaracje funkcji
void init_shared_memory();
void init_semaphores();
void enter_bridge(void);
void enter_ship(void);
void exit_ship(void);
void exit_bridge(void);
void *Passenger(void *arg);
void *PortCaptain(void *arg);
void *ShipCaptain(void *arg);

#endif // COMMON_H