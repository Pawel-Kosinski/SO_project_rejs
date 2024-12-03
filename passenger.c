#include "common.h"

void enter_bridge(int passenger_id) {
    if (sem_wait(&bridge_sem) == -1) {
        perror("sem_wait bridge sem");
        pthread_exit(NULL);
    }
    pthread_mutex_lock(&mutex);
    shared_data->passengers_on_bridge++;
    printf("Pasazer %ld wchodzi na mostek. Liczba osob na mostku: %d\n", passenger_id, shared_data->passengers_on_bridge);
    pthread_mutex_unlock(&mutex);
    usleep((rand() % 1000 + 1000) * 1000);
}

void enter_ship(int passenger_id) {
    if (sem_wait(&ship_sem) == -1) {
        perror("sem_wait ship sem");
        pthread_exit(NULL);
    }
    pthread_mutex_lock(&mutex);
    shared_data->passengers_on_board++;
    shared_data->passengers_on_bridge--;
    printf("Pasazer %ld wchodzi na statek. Liczba osob na statku: %d\n", passenger_id, shared_data->passengers_on_board);
    pthread_mutex_unlock(&mutex);

    if (sem_post(&bridge_sem) == -1) {
        perror("sem_post bridge sem");
        pthread_exit(NULL);
    }
    usleep((rand() % 1000 + 500) * 1000);
}

void exit_ship(int passenger_id) {
    pthread_mutex_lock(&mutex);
    shared_data->passengers_on_board--;
    printf("Pasazer %ld opuszcza statek. Liczba osob na statku: %d\n", passenger_id, shared_data->passengers_on_board);
    pthread_mutex_unlock(&mutex);

    if (sem_post(&ship_sem) == -1) {
        perror("sem_post ship sem");
        pthread_exit(NULL);
    }
    usleep((rand() % 1000 + 500) * 1000);
}

void exit_bridge(int passenger_id) {
    if (sem_post(&bridge_sem) == -1) {
        perror("sem_post bridge sem");
        pthread_exit(NULL);
    }
    pthread_mutex_lock(&mutex);
    shared_data->passengers_on_bridge--;
    printf("Pasazer %ld schodzi z mostku. Liczba osob na mostku: %d\n", passenger_id, shared_data->passengers_on_bridge);
    pthread_mutex_unlock(&mutex);
    usleep((rand() % 1000 + 500) * 1000);
}

void *Passenger(void *arg) {
    while (1) {
        int passenger_id = *(int *)arg;

        printf("Pasazer przybyl do kolejki. \n", passenger_id);

        pthread_mutex_lock(&queue_mutex);
        while(!shared_data->boarding_allowed) {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }
        pthread_mutex_unlock(&queue_mutex);

        enter_bridge(passenger_id);
        pthread_mutex_lock(&mutex);
        if (shared_data->loading == 1) {
            pthread_mutex_unlock(&mutex);
            enter_ship(passenger_id);
        }
        else {
            pthread_mutex_unlock(&mutex);
            exit_bridge(passenger_id);
            pthread_exit(NULL);
        }
        
        pthread_mutex_lock(&voyage_mutex);
        while(!shared_data->unloading_allowed) {
            pthread_cond_wait(&voyage_cond, &voyage_mutex);
        }
        pthread_mutex_unlock(&voyage_mutex);

        exit_ship(passenger_id);

        pthread_mutex_lock(&mutex);
        if (shared_data->loading == 2) {
            pthread_mutex_unlock(&mutex);
            enter_bridge(passenger_id);
        }
        else {
            pthread_mutex_unlock(&mutex);
            sleep(1);
        }

        exit_bridge(passenger_id);
        pthread_mutex_lock(&ship_mutex);
        if (shared_data->voyage_number > R) {
            pthread_mutex_unlock(&ship_mutex);
            break;
        }
        pthread_mutex_unlock(&ship_mutex);
    }
    pthread_exit(NULL);

}