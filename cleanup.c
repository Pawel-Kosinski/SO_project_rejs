// cleanup.c

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>

// Funkcja do usuwania pamieci wspoldzielonej
void cleanup_shared_memory(key_t shm_key) {
    // Uzyskanie ID pamieci wspoldzielonej
    int shm_id_local = shmget(shm_key, sizeof(SharedData), 0600);
    if (shm_id_local == -1) {
        if (errno == ENOENT) {
            printf("Pamiec wspoldzielona nie istnieje.\n");
        } else {
            perror("shmget");
        }
    } else {
        // Usuniecie segmentu pamieci wspoldzielonej
        if (shmctl(shm_id_local, IPC_RMID, NULL) == -1) {
            perror("shmctl IPC_RMID");
        } else {
            printf("Pamiec wspoldzielona zostala usunieta.\n");
        }
    }
}

// Funkcja do usuwania zestawu semaforow
void cleanup_semaphores(key_t sem_key) {
    // Uzyskanie ID zestawu semaforow
    int semid_local = semget(sem_key, 5, 0600);
    if (semid_local == -1) {
        if (errno == ENOENT) {
            printf("Zestaw semaforow nie istnieje.\n");
        } else {
            perror("semget");
        }
    } else {
        // Usuniecie zestawu semaforow
        if (semctl(semid_local, 0, IPC_RMID) == -1) {
            perror("semctl IPC_RMID");
        } else {
            printf("Zestaw semaforow zostal usuniety.\n");
        }
    }
}

// Funkcja do usuwania kolejki komunikatow
void cleanup_message_queue(key_t msg_key) {
    // Uzyskanie ID kolejki komunikatow
    int msgid_local = msgget(msg_key, 0600);
    if (msgid_local == -1) {
        if (errno == ENOENT) {
            printf("Kolejka komunikatow nie istnieje.\n");
        } else {
            perror("msgget");
        }
    } else {
        // Usuniecie kolejki komunikatow
        if (msgctl(msgid_local, IPC_RMID, NULL) == -1) {
            perror("msgctl IPC_RMID");
        } else {
            printf("Kolejka komunikatow zostala usunieta.\n");
        }
    }
}

int main() {
    printf("Rozpoczynanie procesu czyszczenia zasobow IPC.\n");

    // Upewnienie sie, ze plik "rejs" istnieje
    FILE *file = fopen("rejs", "r");
    if (file == NULL) {
        // Jesli plik nie istnieje, probujemy go utworzyc
        file = fopen("rejs", "w");
        if (file == NULL) {
            perror("fopen rejs");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);

    // Generowanie kluczy IPC
    key_t shm_key = ftok("rejs", 'R');
    if (shm_key == -1) {
        perror("ftok shm_key");
        // Kontynuujemy, mimo ze moze nie udac sie wygenerowac klucz
    }

    key_t sem_key = ftok("rejs", 'S');
    if (sem_key == -1) {
        perror("ftok sem_key");
        // Kontynuujemy, mimo ze moze nie udac sie wygenerowac klucz
    }

    key_t msg_key = ftok("rejs", 'M');
    if (msg_key == -1) {
        perror("ftok msg_key");
        // Kontynuujemy, mimo ze moze nie udac sie wygenerowac klucz
    }

    // Usuwanie pamieci wspoldzielonej
    if (shm_key != -1) {
        cleanup_shared_memory(shm_key);
    } else {
        printf("Nie mozna usunac pamieci wspoldzielonej ze wzgledu na blad ftok.\n");
    }

    // Usuwanie semaforow
    if (sem_key != -1) {
        cleanup_semaphores(sem_key);
    } else {
        printf("Nie mozna usunac zestawu semaforow ze wzgledu na blad ftok.\n");
    }

    // Usuwanie kolejki komunikatow
    if (msg_key != -1) {
        cleanup_message_queue(msg_key);
    } else {
        printf("Nie mozna usunac kolejki komunikatow ze wzgledu na blad ftok.\n");
    }

    printf("Proces czyszczenia zasobow IPC zakonczony.\n");
    return 0;
}
