// init.c

#include "common.h"

SharedData *shared_data = NULL;
int shm_id;
sem_t bridge_sem;
sem_t ship_sem;

void init_shared_memory() {
    key_t key = ftok("rejs", 'R');
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(key, sizeof(SharedData), IPC_CREAT | 0600);
    if (shm_id < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *) -1) {
        perror("smhat");
        exit(EXIT_FAILURE);
    }

    shared_data->passengers_on_board = 0;
    shared_data->passengers_on_bridge = 0;
    shared_data->voyage_number = 0;
    shared_data->loading = 1;
}

void init_semaphores() {
    if (sem_init(&bridge_sem, 1, K) != 0) {
        perror("sem_init bridge_sem");
        exit(1);
    }

    if (sem_init(&ship_sem, 1, N) != 0) {
        perror("sem_init ship_sem");
        exit(1);
    }
}

