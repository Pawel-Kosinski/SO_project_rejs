#include "common.h"

int main() {

    init_shared_memory();
    init_semaphores();

    pthread_t passenger_threads[NUM_PASSENGERS];
    int passenger_ids[NUM_PASSENGERS];

    srand(time(NULL));

    pthread_t ship_captain_thread;
    if (pthread_create(&ship_captain_thread, NULL, ShipCaptain, NULL) != 0) {
        perror("Nie udalo sie utworzyc Kapitana Statku");
    }

    pthread_t port_captain_thread;
    if (pthread_create(&port_captain_thread, NULL, PortCaptain, NULL) != 0) {
        perror("Nie udalo sie utworzyc Kapitana Portu");
    }

    for (int i = 0; i < NUM_PASSENGERS; i++) {
        passenger_ids[i] = i + 1;
        if (pthread_create(&passenger_threads[i], NULL, Passenger, &passenger_ids[i]) != 0) {
            perror("Nie udalo sie utworzyc pasazera");
        }
        usleep((rand() % 1000 + 1000) * 1000);
    }

    for (int i = 0; i < NUM_PASSENGERS; i++) {
        pthread_join(passenger_threads[i], NULL);
    }

    pthread_join(ship_captain_thread, NULL);
    pthread_join(port_captain_thread, NULL);

    if (sem_destroy(&bridge_sem) != 0) {
        perror("sem_destroy bridge_sem");
    }
    if (sem_destroy(&ship_sem) != 0) {
        perror("sem_destroy ship_sem");
    }
    if (shmdt(shared_data) == -1) {
        perror("shmdt");
    }
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }

    printf("Program zakonczył dzialanie.\n");
    return 0;
}