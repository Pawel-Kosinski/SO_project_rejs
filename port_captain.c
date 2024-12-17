// port_captain.c
#include "common.h"
#include <signal.h>

SharedData *shared_data = NULL;
int shm_id;
int semid;
int msgid;
ssize_t msg_r;

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
    shared_data->passengers_on_bridge = 0;
    shared_data->passengers_on_board = 0;
    shared_data->voyage_number = 0;
    shared_data->loading_finished = 0;
    
}

void create_semaphore() {

    key_t key = ftok("rejs", 'S');
    if (key == -1) {
        perror("ftok key_s");
        exit(EXIT_FAILURE);
    }

    semid = semget(key, 2, 0600 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    unsigned short initial_values[2] = {K, N}; 

    union semun arg;
    arg.array = initial_values;
    if (semctl(semid, 0, SETALL, arg) == -1) {
        perror("semctl SETALL");
        exit(EXIT_FAILURE);
    }
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
        // Inicjalizacja mutexa
    pthread_mutexattr_t mutex_attr;
    if (pthread_mutexattr_init(&mutex_attr) != 0) {
        perror("pthread_mutexattr_init");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) != 0) {
        perror("pthread_mutexattr_setpshared");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&shared_data->mutex, &mutex_attr) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }
    pthread_mutexattr_destroy(&mutex_attr);
    
}
// Definicja funkcji sem_p (P operation)
void sem_p(int semid, unsigned short semnum) {
    struct sembuf op;
    op.sem_num = semnum;   // Indeks semafora w zestawie
    op.sem_op = -1;        // P: dekrementacja o 1
    op.sem_flg = 0;        // Brak dodatkowych flag

    while (semop(semid, &op, 1) == -1) {
        if (errno != EINTR) {
            perror("semop P");
            exit(EXIT_FAILURE);
        }
        // Jeśli przerwane przez sygnał, powtórz operację
    }
}

// Definicja funkcji sem_v (V operation)
void sem_v(int semid, unsigned short semnum) {
    struct sembuf op;
    op.sem_num = semnum;   // Indeks semafora w zestawie
    op.sem_op = 1;         // V: inkrementacja o 1
    op.sem_flg = 0;        // Brak dodatkowych flag

    if (semop(semid, &op, 1) == -1) {
        perror("semop V");
        exit(EXIT_FAILURE);
    }
}

void handle_sighup(int sig) {
    pthread_mutex_lock(&shared_data->mutex);
    if (shared_data->loading == 1) {
        printf("Kapitan Portu: Zakanczam przedwczesnie zaladunek");
        shared_data->loading_finished = 1;
        
    }
    pthread_mutex_unlock(&shared_data->mutex);
}

int main() {
    init_shared_memory();
    create_semaphore();
    //signal(SIGINT, handle_sigint);
    struct sigaction sa;
    sa.sa_handler = handle_sighup;
    sa.sa_flags = SA_RESTART; 
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sighup");
        exit(EXIT_FAILURE);
    }
    /*key_t shm_key = ftok("rejs", 'R');
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
    if (shared_data == (SharedData *)-1) {
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
    }*/

    printf("Kapitan Portu: Rozpoczynam prace.\n");

    while (1) {
        struct msgbuf msg;
        msg_r = msgrcv(msgid, &msg, sizeof(msg.mtext), 0, 0);
        if (msg_r== -1) {
            if (errno == EINTR) {
                printf("msgrcv przerwane przez sygnal, ponawiam...\n");
                continue;
            }
            else {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
        }


        
        if (msg.mtype == MSG_TYPE_START_BOARDING) {
            printf("Kapitan Portu: Otrzymano sygnal rozpoczecia zaladunku.\n");
            usleep((rand() % 2000 + 2000) * 1000);
            
            pthread_mutex_lock(&shared_data->mutex);
            shared_data->boarding_allowed = 1;
            pthread_mutex_unlock(&shared_data->mutex);
            printf("Kapitan Portu: Wyslano komunikat do pasazerow o rozpoczeciu zaladunku.\n");

        } else if (msg.mtype == MSG_TYPE_START_UNLOADING) {
            printf("Kapitan Portu: Otrzymano sygnal rozpoczecia rozladunku.\n");
            
            struct msgbuf unloading_msg;
            unloading_msg.mtype = MSG_TYPE_UNLOADING_ALLOWED;
            strncpy(unloading_msg.mtext, "Unloading is now allowed.", sizeof(unloading_msg.mtext) - 1);
            unloading_msg.mtext[sizeof(unloading_msg.mtext) - 1] = '\0';

            for (int i = 0; i < NUM_PASSENGERS; i++) {
                if (msgsnd(msgid, &unloading_msg, sizeof(unloading_msg.mtext), 0) == -1) {
                    perror("msgsnd unloading_allowed");
                    exit(EXIT_FAILURE);
                }
            }
            
            printf("Kapitan Portu: Wyslano komunikat do pasazerow o rozpoczeciu rozladunku.\n");
            pthread_mutex_lock(&shared_data->mutex);
            if (shared_data->voyage_number >= R) {
                pthread_mutex_unlock(&shared_data->mutex);
                printf("Kapitan Portu: Osiagnieto maksymalna liczbe rejsow. Koncze prace.\n");
                break;
            }
            pthread_mutex_unlock(&shared_data->mutex);
        }
            else if (msg.mtype == MSG_TYPE_TERMINATE) {
                printf("Kapitan Statku: koncze przedwczesnie");
                break;
            }
    }

    while (1) {
        if (shared_data->loading != 0) {
            sleep(1);
        }
        else 
            break;
    }
    // Odłączenie pamięci współdzielonej
    if (shmdt(shared_data) == -1) {
        perror("shmdt");
    }

    printf("Rozpoczynanie procesu czyszczenia zasobow IPC.\n");

    FILE *file = fopen("rejs", "R");
    if (file == NULL) {
        file = fopen("rejs", "w");
        if (file == NULL) {
            perror("fopen rejs");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);

    key_t shm_key = ftok("rejs", 'R');
    if (shm_key == -1) {
        perror("ftok shm_key");
    }

    key_t sem_key = ftok("rejs", 'S');
    if (sem_key == -1) {
        perror("ftok sem_key");
    }

    key_t msg_key = ftok("rejs", 'M');
    if (msg_key == -1) {
        perror("ftok msg_key");
    }

    if (shm_key != -1) {
        int shm_id = shmget(shm_key, sizeof(SharedData), 0);
        if (shm_id == -1) {
            perror("shmget");
        } else {
            SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
            if (shared_data == (SharedData *) -1) {
                perror("shmat");
            } else {
                // Najpierw niszczymy obiekty synchronizacyjne
                if (pthread_mutex_destroy(&shared_data->mutex) != 0) {
                    perror("pthread_mutex_destroy");
                }
                //if (pthread_cond_destroy(&shared_data->cond_boarding) != 0) {
                   // perror("pthread_cond_destroy cond_boarding");
                //}
                if (shmdt(shared_data) == -1) {
                    perror("shmdt");
                }
                if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
                    perror("shmctl IPC_RMID");
                } else {
                    printf("Pamiec wspoldzielona zostala usunieta.\n");
                }
            }
        }
    } else {
        printf("Nie mozna usunac pamieci wspoldzielonej ze wzgledu na blad ftok.\n");
    }

    // Usuwanie semaforow
    if (sem_key != -1) {
        int semid_local = semget(sem_key, 2, 0600); 
        if (semid_local == -1) {
            if (errno == ENOENT) {
                printf("Zestaw semaforow nie istnieje.\n");
            } else {
                perror("semget");
            }
        } else {
            if (semctl(semid_local, 0, IPC_RMID) == -1) {
                perror("semctl IPC_RMID");
            } else {
                printf("Zestaw semaforow zostal usuniety.\n");
            }
        }
    } else {
        printf("Nie mozna usunac zestawu semaforow ze wzgledu na blad ftok.\n");
    }

    // Usuwanie kolejki komunikatow
    if (msg_key != -1) {
        int msgid_local = msgget(msg_key, 0600);
        if (msgid_local == -1) {
            if (errno == ENOENT) {
                printf("Kolejka komunikatow nie istnieje.\n");
            } else {
                perror("msgget");
            }
        } else {
            if (msgctl(msgid_local, IPC_RMID, NULL) == -1) {
                perror("msgctl IPC_RMID");
            } else {
                printf("Kolejka komunikatow zostala usunieta.\n");
            }
        }
    } else {
        printf("Nie mozna usunac kolejki komunikatow ze wzgledu na blad ftok.\n");
    }

    printf("Proces czyszczenia zasobow IPC zakonczony.\n");
    return 0;
}
