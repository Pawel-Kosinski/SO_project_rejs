// init.c

#include "common.h"

SharedData *shared_data = NULL;
int shm_id;
int semid;
int msgid;

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

    shared_data->loading = 0;
    shared_data->boarding_allowed = 0;
    shared_data->unloading_allowed = 0;
    shared_data->loading_finished = 0;
    shared_data->unloading_finished = 0;
    shared_data->passengers_on_bridge = 0;
    shared_data->passengers_on_board = 0;
    shared_data->voyage_number = 0;
    shared_data->bridge_empty = 0;
    
}

int create_semaphore(key_t key) {
    semid = semget(key, 5, 0600 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    unsigned short initial_values[5] = {1, K, N, 0, 0}; // MUTEX_SEM=1, BRIDGE_SEM=K, SHIP_SEM=N, BOARDING_SEM=0, UNLOADING_SEM=0

    union semun arg;
    arg.array = initial_values;
    if (semctl(semid, 0, SETALL, arg) == -1) {
        perror("semctl SETALL");
        exit(EXIT_FAILURE);
    }
    return semid;
}

int main() {
    printf("Inicjalizacja zasobow. \n");

    init_shared_memory();

    key_t key_s = ftok("rejs", 'S');
    if (key_s == -1) {
        perror("ftok key_s");
        exit(EXIT_FAILURE);
    }

    semid = create_semaphore(key_s);

    key_t msg_key = ftok("rejs", 'M');
    if (msg_key == -1) {
        perror("ftok msg_key");
        exit(EXIT_FAILURE);
    }

    msgid = msgget(msg_key, IPC_CREAT | 0600);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    printf("Inicjalizacja zakonczona.\n");
    return 0;
}