#include "common.h"

void *PortCaptain(void *arg) {

    pthread_mutex_lock(&queue_mutex);
    shared_data->boarding_allowed = 1;
    pthread_cond_broadcast(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
    printf("Kapitan Portu: Rozpoczynam zaladunek pasazerow. \n");
        
}