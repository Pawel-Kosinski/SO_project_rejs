#include "common.h"

void *PortCaptain(void *arg) {
    while (1) {
        pthread_mutex_lock(&port_mutex);
        while(shared_data->loading == 0) {
            pthread_cond_wait(&port_cond, &port_mutex);
        }
        int current_loading = shared_data->loading;
        pthread_mutex_unlock(&port_mutex);

        if (current_loading == 1) {
            pthread_mutex_lock(&queue_mutex);
            shared_data->boarding_allowed = 1;
            pthread_cond_broadcast(&queue_cond);
            pthread_mutex_unlock(&queue_mutex);
            printf("Kapitan Portu: Otwieram mostek dla wchodzacych pasazerow. \n");

            sleep(T1);

            pthread_mutex_lock(&queue_mutex);
            shared_data->boarding_allowed = 0;
            printf("Kapitan Portu: Zakonczenie zaladunku. \n");
            pthread_mutex_unlock(&queue_mutex);

            pthread_mutex_lock(&ship_mutex);
            shared_data->loading_finished = 1;
            pthread_cond_signal(&ship_cond);
            pthread_mutex_unlock(&ship_mutex);
        }
    }
}