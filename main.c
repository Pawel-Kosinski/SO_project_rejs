#include "common.h"

int main() {

    init_shared_memory();
    init_semaphores();

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