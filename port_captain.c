// port_captain.c
#include "common.h"

SharedData *shared_data = NULL;
int shm_id;
int semid;
int msgid;

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

int main() {
    // Uzyskanie kluczy do pamięci współdzielonej, semaforów i kolejki komunikatów
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
    if (shared_data == (SharedData *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Uzyskanie semaforów
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

    // Uzyskanie kolejki komunikatów
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

    printf("Port Captain: Rozpoczynam pracę.\n");

    // Port Captain nasłuchuje na komunikaty od ShipCaptain
    while (1) {
        struct msgbuf msg;
        // Odbieranie komunikatów o rozpoczęciu załadunku lub rozładunku
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        if (msg.mtype == MSG_TYPE_START_BOARDING) {
            printf("Port Captain: Otrzymano sygnał rozpoczęcia załadunku.\n");
            
            // Ustawienie flagi boarding_allowed
            sem_p(semid, MUTEX_SEM);
            shared_data->boarding_allowed = 1;
            sem_v(semid, MUTEX_SEM);

            // Wysłanie komunikatów do pasażerów o rozpoczęciu załadunku
            struct msgbuf boarding_msg;
            boarding_msg.mtype = MSG_TYPE_BOARDING_ALLOWED;
            strncpy(boarding_msg.mtext, "Boarding is now allowed.", sizeof(boarding_msg.mtext) - 1);
            boarding_msg.mtext[sizeof(boarding_msg.mtext) - 1] = '\0';

            for (int i = 0; i < NUM_PASSENGERS; i++) {
                if (msgsnd(msgid, &boarding_msg, sizeof(boarding_msg.mtext), 0) == -1) {
                    perror("msgsnd boarding_allowed");
                    exit(EXIT_FAILURE);
                }
            }
            printf("Port Captain: Wysłano komunikaty do pasażerów o rozpoczęciu załadunku.\n");

        } else if (msg.mtype == MSG_TYPE_START_UNLOADING) {
            printf("Port Captain: Otrzymano sygnał rozpoczęcia rozładunku.\n");
            
            // Ustawienie flagi unloading_allowed
            sem_p(semid, MUTEX_SEM);
            shared_data->unloading_allowed = 1;
            sem_v(semid, MUTEX_SEM);

            // Wysłanie komunikatów do pasażerów o rozpoczęciu rozładunku
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
            printf("Port Captain: Wysłano komunikaty do pasażerów o rozpoczęciu rozładunku.\n");
        }

        sem_p(semid, MUTEX_SEM);
        if (shared_data->voyage_number > R) {
            sem_v(semid, MUTEX_SEM);
            printf("Port Captain: Osiągnięto maksymalną liczbę rejsów. Kończę pracę.\n");
            break;
        }
        sem_v(semid, MUTEX_SEM);
    }

    // Odłączenie pamięci współdzielonej
    if (shmdt(shared_data) == -1) {
        perror("shmdt");
    }

    return 0;
}
