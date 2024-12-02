#include "common.h"

void *ShipCaptain(void *arg) {
    int voyages = 1;
    while (voyages < R) {
        pthread_mutex_lock(&mutex);
        shared_data->loading = 1;
        printf("Kapitan: Rozpoczecie zaladunku do rejsu %d. \n", voyages);
        pthread_mutex_unlock(&mutex);

        sleep(T1);
        pthread_mutex_lock(&mutex);
        shared_data->loading = 0; 
        printf("Kapitan: Zakonczenie zaladunku. Odplywamy w rejs %d.\n", voyages);

        if (shared_data->passengers_on_bridge > 0) {
            pthread_mutex_unlock(&mutex);
            printf("Kapitan: Czekam, az pasazerowie opuszcza mostek. \n");

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

        printf("Kapitan: Rejs %d w trakcie.\n", voyages);
        sleep(T2);

        pthread_mutex_lock(&mutex);
        shared_data->loading = 2;
        printf("Kapitan: Rejs %d zakonczony. Pasazerowie moga opuscic statek. \n", voyages);
        pthread_mutex_unlock(&mutex);
        while(1) {
            pthread_mutex_lock(&mutex);
            if (shared_data->passengers_on_board == 0) {
                pthread_mutex_unlock(&mutex);
                break;
            }
            pthread_mutex_unlock(&mutex);
            sleep(1);
        }
        
        shared_data->voyage_number = voyages;
        voyages++;
    }
    printf("Kapitan: Wykonano maksymalna liczbe rejsow (%d). Koncze prace. \n", R);
    pthread_exit(NULL);
}
