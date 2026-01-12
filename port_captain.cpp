#include "PortCaptain.hpp"

SharedData *shared_data = nullptr;
int shm_id = -1;
int semid = -1;
int msgid = -1;

void cleanup_resources();

void init_shared_memory() {
    key_t key = ftok("rejs", 'R');
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(key, sizeof(SharedData), IPC_CREAT | 0600);
    if (shm_id < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    void* ptr = shmat(shm_id, nullptr, 0);
    if (ptr == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    shared_data = static_cast<SharedData*>(ptr);

    // Inicjalizacja zmiennych
    shared_data->loading = 0;
    shared_data->boarding_allowed = 0;
    shared_data->unloading_allowed = 0;
    shared_data->passengers_on_bridge = 0;
    shared_data->passengers_on_board = 0;
    shared_data->voyage_number = 0;
    shared_data->loading_finished = 0;
    shared_data->unloading_finished = 0;
    shared_data->passengers = 0;
    shared_data->terminate = 0;
}

void create_ipc_objects() {
    // 1. Semafory
    key_t key_s = ftok("rejs", 'S');
    if (key_s == -1) {
        perror("ftok key_s");
        exit(EXIT_FAILURE);
    }

    semid = semget(key_s, 2, 0600 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    unsigned short initial_values[2] = {static_cast<unsigned short>(K), static_cast<unsigned short>(N)};
    union semun arg;
    arg.array = initial_values;
    if (semctl(semid, 0, SETALL, arg) == -1) {
        perror("semctl SETALL");
        exit(EXIT_FAILURE);
    }

    // 2. Kolejka komunikatów
    key_t key_m = ftok("rejs", 'M');
    if (key_m == -1) {
        perror("ftok msg_key");
        exit(EXIT_FAILURE);
    }

    msgid = msgget(key_m, IPC_CREAT | 0600);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // 3. Mutex w pamięci współdzielonej (Process Shared)
    pthread_mutexattr_t mutex_attr;
    if (pthread_mutexattr_init(&mutex_attr) != 0) {
        perror("pthread_mutexattr_init");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) != 0) {
        perror("pthread_mutexattr_setpshared");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&shared_data->mutex, &mutex_attr) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }
    pthread_mutexattr_destroy(&mutex_attr);
}

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
    init_shared_memory();
    create_ipc_objects();

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

    cleanup_resources();
    return 0;
}

void cleanup_resources() {
    std::cout << RED << "Rozpoczynanie procesu czyszczenia zasobow IPC.\n" << RESET;

    if (shmdt(shared_data) == -1) perror("shmdt");

    // Utworzenie pliku jeśli nie istnieje (do ftok)
    FILE *file = fopen("rejs", "r");
    if (file == NULL) {
        file = fopen("rejs", "w");
        if (file) fclose(file);
    } else {
        fclose(file);
    }

    key_t shm_key = ftok("rejs", 'R');
    if (shm_key != -1) {
        int sid = shmget(shm_key, sizeof(SharedData), 0);
        if (sid != -1) {
            // Tymczasowe podłączenie, żeby zniszczyć mutex
            SharedData *sd = (SharedData *)shmat(sid, NULL, 0);
            if (sd != (SharedData *)-1) {
                pthread_mutex_destroy(&sd->mutex);
                shmdt(sd);
            }
            if (shmctl(sid, IPC_RMID, NULL) == -1) perror("shmctl IPC_RMID");
            else std::cout << "Pamiec wspoldzielona usunieta.\n";
        }
    }

    key_t sem_key = ftok("rejs", 'S');
    if (sem_key != -1) {
        int sid = semget(sem_key, 2, 0600);
        if (sid != -1) {
            if (semctl(sid, 0, IPC_RMID) == -1) perror("semctl IPC_RMID");
            else std::cout << "Semafora usuniete.\n";
        }
    }

    key_t msg_key = ftok("rejs", 'M');
    if (msg_key != -1) {
        int mid = msgget(msg_key, 0600);
        if (mid != -1) {
            if (msgctl(mid, IPC_RMID, NULL) == -1) perror("msgctl IPC_RMID");
            else std::cout << "Kolejka komunikatow usunieta.\n";
        }
    }

    std::cout << RED << "Kapitan Portu: Koncze prace.\n" << RESET;
}