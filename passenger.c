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
    //int val = semctl(semid, semnum, GETVAL);
    //printf("Aktualna wartosc na semaforze %d: %d\n" , semnum, val);
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
    //int val = semctl(semid, semnum, GETVAL);
    //printf("Aktualna wartosc na semaforze %d: %d\n", semnum, val);
}

void enter_bridge(int passenger_id, int semid, SharedData *shared_data) {
    usleep((rand() % 8000 + 2000) * 1000);
    sem_p(semid, BRIDGE_SEM);
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->passengers_on_bridge++;
    printf(GREEN "Pasazer %d wchodzi na mostek. Liczba osob na mostku: %d\n" RESET, passenger_id, shared_data->passengers_on_bridge);
    pthread_mutex_unlock(&shared_data->mutex);
    usleep((rand() % 5000 + 5000) * 1000);
    
}
void exit_bridge(int passenger_id, int semid, SharedData *shared_data) {
    pthread_mutex_lock(&shared_data->mutex);
    sem_v(semid, BRIDGE_SEM);
    shared_data->passengers_on_bridge--;
    printf(GREEN "Pasazer %d schodzi z mostku. Liczba osob na mostku: %d\n" RESET, passenger_id, shared_data->passengers_on_bridge);
    pthread_mutex_unlock(&shared_data->mutex);
}

void enter_ship(int passenger_id, int semid, SharedData *shared_data) {
    sem_p(semid, SHIP_SEM);
    shared_data->passengers_on_bridge--;
    sem_v(semid, BRIDGE_SEM);
    printf(GREEN "Pasazer %d wchodzi na statek. Liczba osob na statku: %d\n" RESET, passenger_id, shared_data->passengers_on_board);
    pthread_mutex_unlock(&shared_data->mutex);
}

void exit_ship(int passenger_id, int semid, SharedData *shared_data) {
    usleep((rand() % 8000 + 2000) * 1000);
    sem_v(semid, SHIP_SEM);
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->passengers_on_board--;
    printf(GREEN "Pasazer %d opuszcza statek. Liczba osob na statku: %d\n" RESET, passenger_id, shared_data->passengers_on_board);
    pthread_mutex_unlock(&shared_data->mutex);
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

    shm_id = shmget(shm_key, sizeof(SharedData), 0600);
    if (shm_id < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    key_t sem_key = ftok("rejs", 'S');
        if (sem_key == -1) {
        perror("ftok sem_key");
        exit(EXIT_FAILURE);
    }
    
    semid = semget(sem_key, 2, 0600);
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
        pthread_mutex_lock(&shared_data->mutex);
        shared_data->passengers++;
        if (shared_data->terminate == 1) {
            pthread_mutex_unlock(&shared_data->mutex);
            printf(GREEN "Koncze tworzenie pasazerow\n" RESET);
            break;
        }
        pthread_mutex_unlock(&shared_data->mutex);
    }
    //int n = shared_data->passengers;
    for (int i = 0; i < shared_data->passengers; i++) {
        pthread_join(threads[i], NULL);
        pthread_mutex_lock(&shared_data->mutex);
        //printf(GREEN "Pozostało %d pasazerow\n" RESET, shared_data->passengers - i);
        pthread_mutex_unlock(&shared_data->mutex);
    }
    shared_data->passengers = 0;

    if (shmdt(shared_data) == -1) {
        perror("shmdt");
    }

    printf(GREEN "Program pasazera zakonczyl dzialanie.\n" RESET);
    return 0;
}

void *Passenger(void* args) {

    PassengerArgs *p_args = (PassengerArgs*) args;
    int passenger_id = p_args->passenger_id;
    int semid = p_args->semid;
    SharedData *shared_data = p_args->shared_data;
    printf(GREEN "Pasazer %d przybyl do kolejki. \n" RESET, passenger_id);
    while (1) {
        while(1) {
            pthread_mutex_lock(&shared_data->mutex);
            if (shared_data->boarding_allowed == 1) {
                pthread_mutex_unlock(&shared_data->mutex);
                break;
            }
            else if (shared_data->voyage_number > R) {
                pthread_mutex_unlock(&shared_data->mutex);
                printf(GREEN "Pasazer %d zakonczyl prace. \n" RESET, passenger_id);
                pthread_exit(NULL);
            }
            else {
                pthread_mutex_unlock(&shared_data->mutex);
                sleep(1);
            }
        }
        
        enter_bridge(passenger_id, semid, shared_data);
        pthread_mutex_lock(&shared_data->mutex);
        if (shared_data->loading == 1) {
            if (shared_data->passengers_on_board < N) {
                shared_data->passengers_on_board++;
                enter_ship(passenger_id, semid, shared_data);

                struct msgbuf msg;    
                if (msgrcv(msgid, &msg, sizeof(msg.mtext), MSG_TYPE_UNLOADING_ALLOWED, 0) == -1) {
                    perror("msgrcv unloading_allowed");
                    pthread_exit(NULL);
                }
                if (strcmp(msg.mtext, "Unloading is now allowed.") == 0) {
                    exit_ship(passenger_id, semid, shared_data);
                    
                    while(1) {
                        pthread_mutex_lock(&shared_data->mutex);
                        if (shared_data->loading == 2) {
                            pthread_mutex_unlock(&shared_data->mutex);
                            enter_bridge(passenger_id, semid, shared_data);
                            break;
                        } else {
                            pthread_mutex_unlock(&shared_data->mutex);
                            sleep(1);
                        }
                    }
                    exit_bridge(passenger_id, semid, shared_data);
                }
                else if (strcmp(msg.mtext, "Abort voyages") == 0) {
                    printf(GREEN "Pasazer %d otrzymal sygnal o przedwczesnym zakonczeniu rejsow. \n" RESET, passenger_id);
                    exit_ship(passenger_id, semid, shared_data);
                    enter_bridge(passenger_id, semid, shared_data);
                    exit_bridge(passenger_id, semid, shared_data);
                    break;
                }
            }
            else {
                if (shared_data->loading_finished = 1) {
                    pthread_mutex_unlock(&shared_data->mutex);
                    printf(GREEN "Maksymalna ilosc osob na statku. Pasazer %d schodzi z mostku.\n" RESET, passenger_id);
                    exit_bridge(passenger_id, semid, shared_data);
                }
                else {
                    pthread_mutex_unlock(&shared_data->mutex);
                    sleep(1);
                }
            }
   
        }
        else {
            pthread_mutex_unlock(&shared_data->mutex);
            exit_bridge(passenger_id, semid, shared_data);
        }
        pthread_mutex_lock(&shared_data->mutex);
        if (shared_data->voyage_number >= R) {
            pthread_mutex_unlock(&shared_data->mutex);
            break;
        }
        pthread_mutex_unlock(&shared_data->mutex);   
    }
    printf(GREEN "Pasazer %d zakonczyl prace. \n" RESET, passenger_id);
    pthread_exit(NULL);
}

