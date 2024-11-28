#include "common.h"

void *Passenger() {
    while (1) {
        pthread_mutex_lock(&mutex);
        if (shared_data->loading == 1 && shared_data->passengers_on_board < N) {
            
        }
    }
}