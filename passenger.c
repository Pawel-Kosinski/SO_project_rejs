#include "common.h"

SharedData *shared_data = NULL;
int shm_id;
int semid;
int msgid;

void sem_p(int semid, unsigned short semnum) {
    struct sembuf op;
    op.sem_num = semnum;   
    op.sem_op = -1;        
    op.sem_flg = 0;        

    int result = semop(semid, &op, 1);
        if (result == -1) {
            if(errno == EINTR){
                semop(semid, &op, 1);
            }
            else {
            perror("semop P");
            exit(EXIT_FAILURE);
            }
    }
}

void sem_v(int semid, unsigned short semnum) {
    struct sembuf op;
    op.sem_num = semnum;   
    op.sem_op = 1;        
    op.sem_flg = 0;        

    int result = semop(semid, &op, 1);
        if (result == -1) {
            if(errno == EINTR){
                semop(semid, &op, 1);
            }
            else {
            perror("semop P");
            exit(EXIT_FAILURE);
            }
    }
}

void enter_bridge(int passenger_id, int semid, SharedData *shared_data) {

    sem_p(semid, BRIDGE_SEM);
    sem_p(semid, MUTEX_SEM);
    shared_data->passengers_on_bridge++;
    printf("Pasazer %d wchodzi na mostek. Liczba osob na mostku: %d\n", passenger_id, shared_data->passengers_on_bridge);
    sem_v(semid, MUTEX_SEM);
    usleep((rand() % 5000 + 5000) * 1000);
}

void enter_ship(int passenger_id, int semid, SharedData *shared_data) {
    
    sem_p(semid, SHIP_SEM);
    sem_p(semid, MUTEX_SEM);
    shared_data->passengers_on_board++;
    shared_data->passengers_on_bridge--;
    printf("Pasazer %d wchodzi na statek. Liczba osob na statku: %d\n", passenger_id, shared_data->passengers_on_board);
    sem_v(semid, MUTEX_SEM);
    sem_v(semid, BRIDGE_SEM);
    usleep((rand() % 1000 + 500) * 1000);
}

void exit_ship(int passenger_id, int semid, SharedData *shared_data) {
    sem_p(semid, MUTEX_SEM);
    shared_data->passengers_on_board--;
    printf("Pasazer %d opuszcza statek. Liczba osob na statku: %d\n", passenger_id, shared_data->passengers_on_board);
    sem_v(semid, MUTEX_SEM);
    sem_v(semid, SHIP_SEM);
    usleep((rand() % 1000 + 500) * 1000);
}

void exit_bridge(int passenger_id, int semid, SharedData *shared_data) {
    sem_p(semid, MUTEX_SEM);
    sem_v(semid, BRIDGE_SEM);
    shared_data->passengers_on_bridge--;
    printf("Pasazer %d schodzi z mostku. Liczba osob na mostku: %d\n", passenger_id, shared_data->passengers_on_bridge);
    sem_v(semid, MUTEX_SEM);
    usleep((rand() % 1000 + 500) * 1000);
}

typedef struct {
    int passenger_id;
    int semid;
    SharedData *shared_data;
} PassengerArgs;

void *Passenger(void* args);

int main() {
     srand(time(NULL));

    key_t shm_key = ftok("rejs", 'R');
    if (shm_key == -1) {
        perror("ftok shm_key");
        exit(EXIT_FAILURE);
    }

    int shm_id_local = shmget(shm_key, sizeof(SharedData), 0600);
    if (shm_id_local < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    SharedData *local_shared_data = (SharedData *)shmat(shm_id_local, NULL, 0);
    if (local_shared_data == (SharedData *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    key_t sem_key = ftok("rejs", 'S');
        if (sem_key == -1) {
        perror("ftok sem_key");
        exit(EXIT_FAILURE);
    }
    
    semid = semget(sem_key, 5, 0600);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    key_t msg_key = ftok("rejs", 'M');
    if (msg_key == -1) {
        perror("ftok msg_key");
        exit(EXIT_FAILURE);
    }

    msgid = msgget(msg_key, 0600);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    shared_data = local_shared_data;

    pthread_t threads[NUM_PASSENGERS];
    PassengerArgs args[NUM_PASSENGERS];

    for (int i = 0; i < NUM_PASSENGERS; i++) {
        args[i].passenger_id = i + 1;
        args[i].semid = semid;
        args[i].shared_data = shared_data;
        if (pthread_create(&threads[i], NULL, Passenger, &args[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    sleep(1);
    }
    for (int i = 0; i < NUM_PASSENGERS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Odłączenie pamięci współdzielonej
    if (shmdt(shared_data) == -1) {
        perror("shmdt");
    }

    printf("Program pasażera zakończył działanie.\n");
    return 0;
}

void *Passenger(void* args) {

    PassengerArgs *p_args = (PassengerArgs*) args;
    int passenger_id = p_args->passenger_id;
    int semid = p_args->semid;
    SharedData *shared_data = p_args->shared_data;

    while (1) {
        printf("Pasazer %d przybyl do kolejki. \n", passenger_id);

        struct msgbuf msg;
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), MSG_TYPE_BOARDING_ALLOWED, 0) == -1) {
            perror("msgrcv boarding_allowed");
            exit(EXIT_FAILURE);
        }

        enter_bridge(passenger_id, semid, shared_data);

        sem_p(semid, MUTEX_SEM);
        if (shared_data->loading == 1) {
            sem_v(semid, MUTEX_SEM);
            enter_ship(passenger_id, semid, shared_data);
        }
        else {
            sem_v(semid, MUTEX_SEM);
            exit_bridge(passenger_id, semid, shared_data);
            pthread_exit(NULL);
        }
        
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), MSG_TYPE_UNLOADING_ALLOWED, 0) == -1) {
            perror("msgrcv unloading_allowed");
            pthread_exit(NULL);
        }

        exit_ship(passenger_id, semid, shared_data);
        
        sem_p(semid, MUTEX_SEM);
        if (shared_data->loading == 2) {
            sem_v(semid, MUTEX_SEM);
            enter_bridge(passenger_id, semid, shared_data);
        } else {
            sem_v(semid, MUTEX_SEM);
            sleep(1);
        }

        usleep((rand() % 2000 + 500) * 1000);
        exit_bridge(passenger_id, semid, shared_data);

        sem_p(semid, MUTEX_SEM);
        if (shared_data->voyage_number > R) {
            sem_v(semid, MUTEX_SEM);
            break;
        }
        sem_v(semid, MUTEX_SEM);
    }
    pthread_exit(NULL);
}

