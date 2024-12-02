#include "common.h"

void enter_bridge() {
    if (sem_wait(&bridge_sem) == -1) {
        perror("sem_wait bridge sem");
        pthread_exit(NULL);
    }
    pthread_mutex_lock(&mutex);
    shared_data->passengers_on_bridge++;
    printf("Pasazer %ld wchodzi na mostek. Liczba osob na mostku: %d\n", pthread_self(), shared_data->passengers_on_bridge);
    pthread_mutex_unlock(&mutex);
}

void enter_ship() {
    if (sem_wait(&ship_sem) == -1) {
        perror("sem_wait ship sem");
        pthread_exit(NULL);
    }
    pthread_mutex_lock(&mutex);
    shared_data->passengers_on_board++;
    shared_data->passengers_on_bridge--;
    printf("Pasazer %ld wchodzi na statek. Liczba osob na statku: %d\n", pthread_self(), shared_data->passengers_on_board);
    pthread_mutex_unlock(&mutex);
}

void exit_ship() {
    pthread_mutex_lock(&mutex);
    shared_data->passengers_on_board--;
    printf("Pasazer %ld opuszcza statek. Liczba osob na statku: %d\n", pthread_self(), shared_data->passengers_on_board);
    pthread_mutex_unlock(&mutex);
}

void exit_bridge() {
    if (sem_wait(&bridge_sem) == -1) {
        perror("sem_wait bridge sem");
        pthread_exit(NULL);
    }
    pthread_mutex_lock(&mutex);
    shared_data->passengers_on_bridge--;
    printf("Pasazer %ld schodzi z mostku. Liczba osob na mostku: %d\n", pthread_self(), shared_data->passengers_on_bridge);
    pthread_mutex_unlock(&mutex);
}
