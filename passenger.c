#include "common.h"

void *Passenger() {
    while (1) {
        pthread_mutex_lock(&mutex);
        if (shared_data->loading == 1 && shared_data->passengers_on_board < N) {
            pthread_mutex_unlock(&mutex);

            if (sem_wait(&bridge_sem) == -1) {
                perror("sem_wait bridge sem");
                pthread_exit(NULL);
            }

            pthread_mutex_lock(&mutex);
            shared_data->passengers_on_bridge++;
            printf("Pasazer %ld wchodzi na mostek. Liczba osob na mostku: %d\n", pthread_self(), shared_data->passengers_on_bridge);
            pthread_mutex_unlock(&mutex);

            if (sem_wait(&ship_sem) == -1) {
                perror("sem_wait ship sem");
                pthread_exit(NULL);
            }

            pthread_mutex_unlock(&mutex);
            shared_data->passengers_on_board++;
            shared_data->passengers_on_bridge--;
            printf("Pasazer %ld wchodzi na statek. Liczba osob na statku: %d\n", pthread_self(), shared_data->passengers_on_board);
            pthread_mutex_unlock(&mutex);

            if (sem_post(&bridge_sem) == -1) {
                perror("sem_post bridge sem");
                pthread_exit(NULL);
            }

            while(1) {
                pthread_mutex_lock(&mutex);
                if(shared_data->loading == 2) {
                    pthread_mutex_unlock(&mutex);
                    break;
                }
                pthread_mutex_unlock(&mutex);
                sleep(1);
            }

            if (sem_post(&ship_sem) == -1) {
                perror("sem_post ship sem");
                pthread_exit(NULL);
            }
            pthread_mutex_lock(&mutex);
            shared_data->passengers_on_board--;
            printf("Pasazer %ld opuszcza statek. Liczba osob na statku: %d\n", pthread_self(), shared_data->passengers_on_board);

            if (sem_wait(&bridge_sem) == -1) {
                perror("sem_wait bridge sem");
                pthread_exit(NULL);
            }
            pthread_mutex_lock(&mutex);
            shared_data->passengers_on_bridge++;
            printf("Pasazer %ld schodzi na mostek. Liczba osob na mostku: %d\n", pthread_self(), shared_data->passengers_on_bridge);
            pthread_mutex_unlock(&mutex);

            if (sem_post(&bridge_sem) == -1) {
                perror("sem_post bridge sem");
                pthread_exit(NULL);
            }
            pthread_mutex_lock(&mutex);
            shared_data->passengers_on_bridge--;
            printf("Pasazer %ld schodzi z mostku. Liczba osob na mostku: %d\n", pthread_self(), shared_data->passengers_on_bridge);
            pthread_mutex_unlock(&mutex);

            pthread_exit(NULL);
        }
        else {
            pthread_mutex_unlock(&mutex);
            sleep(1);
        }
    }
}