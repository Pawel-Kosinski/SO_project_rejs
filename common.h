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

#define N 100    // Maksymalna liczba pasazerow na statku
#define K 10     // Maksymalna liczba pasazerow na mostku
#define T1 5     // Czas miedzy rejsami
#define T2 10    // Czas trwania rejsu
#define R 3      // Maksymalna liczba rejsow

// Struktura pamieci dzielonej
typedef struct {
    int passengers_on_board;
    int passengers_on_bridge;
    int voyage_number;
    int loading;
} SharedData;

// Deklaracje globalnych zmiennych
extern SharedData *shared_data;
extern int shm_id;
extern sem_t bridge_sem;
extern sem_t ship_sem;
extern pthread_mutex_t mutex;

// Deklaracje funkcji
void init_shared_memory();
void init_semaphores();

#endif // COMMON_H