#include "ShipCaptain.hpp"

SharedData *shared_data = nullptr;
int shm_id = -1;
int semid = -1;
int msgid = -1;

void alarmHandler(int sig) {
    if (!shared_data) return;
    pthread_mutex_lock(&shared_data->mutex);
    if (shared_data->loading_finished == 0 && shared_data->terminate != 1) {
        std::cout << BLUE << "Kapitan Statku: Czas na zaladunek uplynal.\n" << RESET;
        shared_data->loading_finished = 1;
    }
    pthread_mutex_unlock(&shared_data->mutex);
}

int main() {
    signal(SIGALRM, alarmHandler);

    // 1. Podłączenie SHM
    key_t shm_key = ftok("rejs", 'R');
    if (shm_key == -1) { perror("ftok shm"); exit(EXIT_FAILURE); }
    shm_id = shmget(shm_key, sizeof(SharedData), 0600);
    if (shm_id < 0) { perror("shmget"); exit(EXIT_FAILURE); }
    
    void* ptr = shmat(shm_id, nullptr, 0);
    if (ptr == (void*)-1) { perror("shmat"); exit(EXIT_FAILURE); }
    shared_data = static_cast<SharedData*>(ptr);

    // 2. Podłączenie Sem
    key_t sem_key = ftok("rejs", 'S');
    if (sem_key == -1) { perror("ftok sem"); exit(EXIT_FAILURE); }
    semid = semget(sem_key, 2, 0600);
    if (semid == -1) { perror("semget"); exit(EXIT_FAILURE); }

    // 3. Podłączenie Msg
    key_t msg_key = ftok("rejs", 'M');
    if (msg_key == -1) { perror("ftok msg"); exit(EXIT_FAILURE); }
    msgid = msgget(msg_key, 0600);
    if (msgid == -1) { perror("msgget"); exit(EXIT_FAILURE); }

    std::cout << BLUE << "Kapitan Statku: Rozpoczynam prace.\n" << RESET;
    //int voyages = 0;
    ShipCaptain captain(semid, msgid, shared_data);

    while (captain.voyages < R && !shared_data->terminate) {
        captain.startBoarding();
        captain.waitWhileBoarding();
        captain.finishBoarding();
        captain.startVoyage();
        captain.finishVoyage();            
        captain.shipUnload();        
        captain.bridgeUnload();
    }

    captain.end();
    
    
    return 0;
}