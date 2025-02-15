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



#define N 90   // Maksymalna liczba pasazerow na statku
#define K 50   // Maksymalna liczba pasazerow na mostku
#define T1 80     // Czas miedzy rejsami
#define T2 10    // Czas trwania rejsu
#define R 5     // Maksymalna liczba rejsow
#define NUM_PASSENGERS 100 // Liczba pasazerow  
#define BRIDGE_SEM 0 //nr semafora mostek
#define SHIP_SEM   1 //nr semafora statek

#define MSG_TYPE_START_BOARDING  1    // Typ komunikatu o rozpoczeciu zaladunku
#define MSG_TYPE_START_UNLOADING 2     // Typ komunikatu o rozpoczeciu rozladunku   
#define MSG_TYPE_UNLOADING_ALLOWED 3   // Typ komunikatu do pasazerow: rozladunek dozwolony
//#define MSG_TYPE_TERMINATE 4 // Typ komunikatu o zakonczeniu rejsow na dzis

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
    int passengers;
    int voyage_number;
    int loading; // 0 - brak zaladunku, 1 - zaladunek, 2 - rozladunek
    int boarding_allowed; // 0 - nie mozna wchodzic, 1 - mozna wchodzic
    int loading_finished; // 0 - zaladunek trwa, 1 - zaladunek skonczony
    int unloading_allowed; // 0 - nie mozna schodzic, 1 - mozna schodzic
    int unloading_finished; // zakonczenie rozladunku, jest ustawiane na liczbe pasazerow na statku, gdy pasazer zejdzie z mostku to dekrementuje ta zmienna. 
                            // Koniec rozladunku gdy = 0
    int terminate; // zakonczenie rejsu przez sygnal
    pthread_mutex_t mutex;
} SharedData;

// Deklaracje globalnych zmiennych
extern SharedData *shared_data;
extern int shm_id;
extern int semid;
extern int msgid;

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define BLUE    "\033[34m" //kolorowanie wyjscia

#endif // COMMON_H