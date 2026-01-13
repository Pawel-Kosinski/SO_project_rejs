#include "PortCaptain.hpp"
#include "IPCManager.hpp"

SharedData *shared_data = nullptr;
int shm_id = -1;
int semid = -1;
int msgid = -1;

void handle_sighup(int sig) {
    if (!shared_data) return;
    pthread_mutex_lock(&shared_data->mutex);
    if (shared_data->loading == 1) {
        std::cout << RED << "Kapitan Portu: Zakanczam przedwczesnie zaladunek.\n" << RESET;
        shared_data->loading_finished = 1;
    }
    pthread_mutex_unlock(&shared_data->mutex);
}

void handle_sigabrt(int sig) {
    std::cout << RED << "SIGABRT received: Przerywam rejsy na dany dzień.\n" << RESET;
    
    // Czekaj aż statek nie będzie w trakcie normalnego załadunku (żeby nie przerwać w połowie mutexa w złym stanie)
    while (true) {
        pthread_mutex_lock(&shared_data->mutex);
        if (shared_data->loading == 0) {
             pthread_mutex_unlock(&shared_data->mutex);
             sleep(1);
        } else {
            pthread_mutex_unlock(&shared_data->mutex);
            break;
        }
    }

    pthread_mutex_lock(&shared_data->mutex);
    shared_data->voyage_number = R + 1;
    shared_data->terminate = 1;
    shared_data->loading = 2; // wymuszenie trybu rozładunku/koniec
    shared_data->boarding_allowed = 0;
    pthread_mutex_unlock(&shared_data->mutex);

    struct msgbuf_custom terminate_msg;
    terminate_msg.mtype = MSG_TYPE_UNLOADING_ALLOWED;
    std::strncpy(terminate_msg.mtext, "Abort voyages", sizeof(terminate_msg.mtext) - 1);
    terminate_msg.mtext[sizeof(terminate_msg.mtext) - 1] = '\0';

    // Wysłanie wiadomości do pasażerów
    for (int i = 0; i < N; i++) {
        msgsnd(msgid, &terminate_msg, sizeof(terminate_msg.mtext), 0);
    }
}

int main() {
    std::srand(std::time(nullptr));
    IPCManager ipc_manager;
    ipc_manager.cleanup();

    try {
        ipc_manager.initialize(); 
    } catch (const std::exception& e) {
        std::cerr << "Błąd inicjalizacji IPC: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    semid = ipc_manager.getSemaphoreId();
    msgid = ipc_manager.getMessageQueueId();
    shared_data = ipc_manager.getSharedData();

    // Rejestracja sygnałów
    struct sigaction sa_hup;
    sa_hup.sa_handler = handle_sighup;
    sa_hup.sa_flags = SA_RESTART;
    sigemptyset(&sa_hup.sa_mask);
    if (sigaction(SIGHUP, &sa_hup, nullptr) == -1) {
        perror("sigaction SIGHUP");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa_abrt;
    sa_abrt.sa_handler = handle_sigabrt;
    sa_abrt.sa_flags = SA_RESTART;
    sigemptyset(&sa_abrt.sa_mask);
    if (sigaction(SIGABRT, &sa_abrt, nullptr) == -1) {
        perror("sigaction SIGABRT");
        exit(EXIT_FAILURE);
    }

    PortCaptain port_captain(semid, msgid, shared_data);
    std::cout << RED << "Kapitan Portu: Rozpoczynam prace.\n" << RESET;

    while (!shared_data->terminate and !port_captain.voyages_ended) {
        struct msgbuf_custom msg = port_captain.waitForMessage();

        if (msg.mtype == MSG_TYPE_START_BOARDING) {
            port_captain.startBoarding();
        } else if (msg.mtype == MSG_TYPE_START_UNLOADING) {
            port_captain.startUnloading();
            port_captain.endVoyages();
        }
    }

    // Czekanie na zakończenie wszystkich pasażerów
    while (true) {
        pthread_mutex_lock(&shared_data->mutex);
        int p = shared_data->passengers;
        pthread_mutex_unlock(&shared_data->mutex);
        
        if (p != 0) {
            sleep(1);
        } else {
            break;
        }
    }

    ipc_manager.cleanup();
    std::cout << RED << "Kapitan Portu: Koniec dnia, zamykam port.\n" << RESET;
    return 0;
}