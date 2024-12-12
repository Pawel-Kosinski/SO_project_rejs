// cleanup.c
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>


int main() {
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
