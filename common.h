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
#include <sys/sem.h>
#include <sys/msg.h>


#define N 29    // Maksymalna liczba pasazerow na statku
#define K 15    // Maksymalna liczba pasazerow na mostku
#define T1 30     // Czas miedzy rejsami
#define T2 20    // Czas trwania rejsu
#define R 2     // Maksymalna liczba rejsow
#define NUM_PASSENGERS 30 // Liczba pasazerow
#define MUTEX_SEM 0   
#define BRIDGE_SEM 1
#define SHIP_SEM   2
#define BOARDING_SEM 3          
#define UNLOADING_SEM 4 

#define MSG_TYPE_START_BOARDING  1    // Typ komunikatu o rozpoczeciu zaladunku
#define MSG_TYPE_START_UNLOADING 2     // Typ komunikatu o rozpoczeciu rozladunku
#define MSG_TYPE_BOARDING_ALLOWED  3   // Typ komunikatu do pasazerow: zaladunek dozwolony
#define MSG_TYPE_UNLOADING_ALLOWED 4   // Typ komunikatu do pasazerow: rozladunek dozwolony

struct msgbuf {
    long mtype;     // typ wiadomosci
    char mtext[64]; // tresc
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

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
    int bridge_empty; // Flaga informujaca o zakonczeniu schodzenia z mostku
} SharedData;

// Deklaracje globalnych zmiennych
extern SharedData *shared_data;
extern int shm_id;
extern int semid;
extern int msgid;


#endif // COMMON_H