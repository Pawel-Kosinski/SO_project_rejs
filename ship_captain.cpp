#include "ShipCaptain.hpp"
#include "IPCManager.hpp"

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

    IPCManager ipc_manager;
    
    try {
        ipc_manager.attach();
    } catch (const std::exception& e) {
        std::cerr << "Błąd podłączenia IPC: " << e.what() << std::endl;
        std::cerr << "Upewnij się, że port_captain jest uruchomiony!\n";
        return EXIT_FAILURE;
    }
    
    semid = ipc_manager.getSemaphoreId();
    msgid = ipc_manager.getMessageQueueId();
    shared_data = ipc_manager.getSharedData();

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