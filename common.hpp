#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

// System V IPC & POSIX headers
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <pthread.h>

// Stałe konfiguracyjne
constexpr int N = 101;               // Maksymalna liczba pasazerow na statku
constexpr int K = 50;               // Maksymalna liczba pasazerow na mostku
constexpr int T1_SEC = 30;          // Czas miedzy rejsami (w sekundach)
constexpr int T2_SEC = 20;          // Czas trwania rejsu
constexpr int R = 3;                // Maksymalna liczba rejsow
constexpr int NUM_PASSENGERS = 100; // Liczba pasazerow

constexpr int BRIDGE_SEM = 0;       // nr semafora mostek
constexpr int SHIP_SEM = 1;         // nr semafora statek

constexpr long MSG_TYPE_START_BOARDING = 1;
constexpr long MSG_TYPE_START_UNLOADING = 2;
constexpr long MSG_TYPE_UNLOADING_ALLOWED = 3;

// Kolory
constexpr const char* RESET = "\033[0m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* RED   = "\033[31m";
constexpr const char* BLUE  = "\033[34m";

struct msgbuf_custom {
    long mtype;
    char mtext[64];
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// Struktura pamieci dzielonej
struct SharedData {
    int passengers_on_board;
    int passengers_on_bridge;
    int passengers;
    int voyage_number;
    int loading;            // 0 - brak, 1 - zaladunek, 2 - rozladunek
    int boarding_allowed;   // 0 - nie, 1 - tak
    int loading_finished;   // 0 - trwa, 1 - skonczony
    int unloading_allowed;  // 0 - nie, 1 - tak
    int unloading_finished; // licznik do zakonczenia rozladunku
    int terminate;          // flaga zakonczenia
    int alarm_fired;          
    
    // Używamy pthread_mutex, ponieważ musi być process-shared
    pthread_mutex_t mutex;
};

// Funkcja pomocnicza do spania w milisekundach
inline void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#endif // COMMON_HPP