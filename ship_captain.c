#include "common.h"

void *ShipCaptain(void *arg) {
    int voyages = 1;
    while (voyages <= R) {

        pthread_mutex_lock(&port_mutex);
        shared_data->loading = 1;
        pthread_cond_signal(&port_cond);
        printf("Kapitan Statku: Rozpoczecie zaladunku do rejsu %d. \n", voyages);
        pthread_mutex_unlock(&port_mutex);

        pthread_mutex_lock(&ship_mutex);
        while(!shared_data->loading_finished) {
            pthread_cond_wait(&ship_cond, &ship_mutex);
        }
        shared_data->loading_finished = 0;
        printf("Kapitan Statku: Odplywamy w rejs %d.\n", voyages);
        pthread_mutex_unlock(&ship_mutex);

        if (shared_data->passengers_on_bridge > 0) {
            pthread_mutex_unlock(&mutex);
            printf("Kapitan Statku: Czekam, az pasazerowie opuszcza mostek. \n");

            while(1) {
                pthread_mutex_lock(&mutex);
                if (shared_data->passengers_on_bridge == 0) {
                    pthread_mutex_unlock(&mutex);
                    break;
                }
                pthread_mutex_unlock(&mutex);
                sleep(1);
            }
        }
        else {
            pthread_mutex_unlock(&mutex);
        }

        printf("Kapitan Statku: Rejs %d w trakcie.\n", voyages);
        sleep(T2);

        pthread_mutex_lock(&port_mutex);
        shared_data->loading = 2;
        pthread_cond_signal(&port_cond);
        pthread_mutex_unlock(&port_mutex);
        printf("Kapitan Statku: Rejs %d zakonczony. Pasazerowie moga opuscic statek. \n", voyages);

        voyages++;
        shared_data->voyage_number = voyages;

        pthread_mutex_lock(&voyage_mutex);
        shared_data->unloading_allowed = 1;
        printf("Kapitan Statku: Rozpoczynamy rozladunek pasazerow. \n");
        pthread_cond_broadcast(&voyage_cond);
        pthread_mutex_unlock(&voyage_mutex);

        while(1) {
            pthread_mutex_lock(&mutex);
            if (shared_data->passengers_on_board == 0) {
                pthread_mutex_unlock(&mutex);
                break;
            }
        pthread_mutex_unlock(&mutex);
        sleep(1);
        }

        pthread_mutex_lock(&voyage_mutex);
        shared_data->unloading_allowed = 0;
        printf("Kapitan Statku: Pasazerowie opuscili statek.\n");
        pthread_mutex_unlock(&voyage_mutex);

        pthread_mutex_lock(&ship_mutex);
        shared_data->unloading_finished = 1;
        pthread_cond_signal(&ship_cond);
        pthread_mutex_unlock(&ship_mutex);

        pthread_mutex_lock(&bridge_empty_mutex);
        while (!shared_data->bridge_empty) {
            pthread_cond_wait(&bridge_empty_cond, &bridge_empty_mutex);
        }
        shared_data->bridge_empty = 0;
        pthread_mutex_unlock(&bridge_empty_mutex);
        
    }
    printf("Kapitan Statku: Wykonano maksymalna liczbe rejsow (%d). Koncze prace. \n", R);
    pthread_exit(NULL);
}
