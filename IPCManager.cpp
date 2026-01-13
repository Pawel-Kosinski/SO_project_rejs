// IPCManager.cpp
#include "IPCManager.hpp"
#include <sys/stat.h>

IPCManager::IPCManager()
    : shm_id_(-1), semid_(-1), msgid_(-1), 
      shared_data_(nullptr), is_creator_(false) {
}

IPCManager::~IPCManager() {
    if (shared_data_ != nullptr) {
        detachSharedMemory();
    }
}

// 
// INICJALIZACJA - Tworzenie nowych zasobów (port_captain)
// 

void IPCManager::initialize() {
    std::cout << "IPCManager: Tworzenie zasobów IPC...\n";
    
    createSharedMemory();
    initializeSharedData();
    initializeMutex();
    createSemaphores();
    createMessageQueue();
    
    is_creator_ = true;
    std::cout << "IPCManager: Wszystkie zasoby IPC utworzone pomyślnie.\n";
}

void IPCManager::createSharedMemory() {
    // Utwórz plik do ftok jeśli nie istnieje
    FILE* file = fopen("rejs", "r");
    if (file == nullptr) {
        file = fopen("rejs", "w");
        if (file) {
            fclose(file);
        } else {
            perror("fopen rejs");
            exit(EXIT_FAILURE);
        }
    } else {
        fclose(file);
    }
    
    key_t key = ftok("rejs", 'R');
    if (key == -1) {
        perror("ftok shared memory");
        exit(EXIT_FAILURE);
    }
    
    // Utwórz shared memory
    shm_id_ = shmget(key, sizeof(SharedData), IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id_ < 0) {
        if (errno == EEXIST) {
            std::cerr << "BŁĄD: Zasoby IPC już istnieją! Uruchom cleanup najpierw.\n";
        } else {
            perror("shmget create");
        }
        exit(EXIT_FAILURE);
    }
    
    // Podłącz shared memory
    void* ptr = shmat(shm_id_, nullptr, 0);
    if (ptr == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    shared_data_ = static_cast<SharedData*>(ptr);
    std::cout << "IPCManager: Shared memory utworzona (ID: " << shm_id_ << ")\n";
}

void IPCManager::initializeSharedData() {
    if (shared_data_ == nullptr) {
        std::cerr << "BŁĄD: Shared data is nullptr!\n";
        exit(EXIT_FAILURE);
    }
    
    // Wyzeruj wszystkie pola
    shared_data_->passengers_on_board = 0;
    shared_data_->passengers_on_bridge = 0;
    shared_data_->passengers = 0;
    shared_data_->voyage_number = 0;
    shared_data_->loading = 0;
    shared_data_->boarding_allowed = 0;
    shared_data_->loading_finished = 0;
    shared_data_->unloading_allowed = 0;
    shared_data_->unloading_finished = 0;
    shared_data_->terminate = 0;
    
    std::cout << "IPCManager: Shared data zainicjalizowana.\n";
}

void IPCManager::initializeMutex() {
    pthread_mutexattr_t mutex_attr;
    
    if (pthread_mutexattr_init(&mutex_attr) != 0) {
        perror("pthread_mutexattr_init");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) != 0) {
        perror("pthread_mutexattr_setpshared");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_mutex_init(&shared_data_->mutex, &mutex_attr) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }
    
    pthread_mutexattr_destroy(&mutex_attr);
    std::cout << "IPCManager: Mutex (process-shared) zainicjalizowany.\n";
}

void IPCManager::createSemaphores() {
    key_t key = ftok("rejs", 'S');
    if (key == -1) {
        perror("ftok semaphores");
        exit(EXIT_FAILURE);
    }
    
    // Utwórz 2 semafory: [0]=mostek(K), [1]=statek(N)
    semid_ = semget(key, 2, IPC_CREAT | IPC_EXCL | 0600);
    if (semid_ == -1) {
        if (errno == EEXIST) {
            std::cerr << "BŁĄD: Semafory już istnieją!\n";
        } else {
            perror("semget create");
        }
        exit(EXIT_FAILURE);
    }
    
    // Inicjalizuj wartości semaforów
    unsigned short initial_values[2] = {
        static_cast<unsigned short>(K),  // BRIDGE_SEM = 0
        static_cast<unsigned short>(N)   // SHIP_SEM = 1
    };
    
    union semun arg;
    arg.array = initial_values;
    
    if (semctl(semid_, 0, SETALL, arg) == -1) {
        perror("semctl SETALL");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "IPCManager: Semafory utworzone (ID: " << semid_ 
              << ", Mostek=" << K << ", Statek=" << N << ")\n";
}

void IPCManager::createMessageQueue() {
    key_t key = ftok("rejs", 'M');
    if (key == -1) {
        perror("ftok message queue");
        exit(EXIT_FAILURE);
    }
    
    msgid_ = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
    if (msgid_ == -1) {
        if (errno == EEXIST) {
            std::cerr << "BŁĄD: Kolejka komunikatów już istnieje!\n";
        } else {
            perror("msgget create");
        }
        exit(EXIT_FAILURE);
    }
    
    std::cout << "IPCManager: Kolejka komunikatów utworzona (ID: " << msgid_ << ")\n";
}

// 
// PODŁĄCZANIE - Do istniejących zasobów (ship_captain, passenger)
// 

void IPCManager::attach() {
    std::cout << "IPCManager: Podłączanie do istniejących zasobów IPC...\n";
    
    attachSharedMemory();
    attachSemaphores();
    attachMessageQueue();
    
    is_creator_ = false;
    std::cout << "IPCManager: Pomyślnie podłączono do zasobów IPC.\n";
}

void IPCManager::attachSharedMemory() {
    key_t key = ftok("rejs", 'R');
    if (key == -1) {
        perror("ftok shared memory");
        exit(EXIT_FAILURE);
    }
    
    // Podłącz się do istniejącej shared memory (bez IPC_CREAT)
    shm_id_ = shmget(key, sizeof(SharedData), 0600);
    if (shm_id_ < 0) {
        std::cerr << "BŁĄD: Nie można znaleźć shared memory! Uruchom port_captain najpierw.\n";
        perror("shmget attach");
        exit(EXIT_FAILURE);
    }
    
    void* ptr = shmat(shm_id_, nullptr, 0);
    if (ptr == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    shared_data_ = static_cast<SharedData*>(ptr);
    std::cout << "IPCManager: Podłączono shared memory (ID: " << shm_id_ << ")\n";
}

void IPCManager::attachSemaphores() {
    key_t key = ftok("rejs", 'S');
    if (key == -1) {
        perror("ftok semaphores");
        exit(EXIT_FAILURE);
    }
    
    semid_ = semget(key, 2, 0600);
    if (semid_ == -1) {
        std::cerr << "BŁĄD: Nie można znaleźć semaforów!\n";
        perror("semget attach");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "IPCManager: Podłączono semafory (ID: " << semid_ << ")\n";
}

void IPCManager::attachMessageQueue() {
    key_t key = ftok("rejs", 'M');
    if (key == -1) {
        perror("ftok message queue");
        exit(EXIT_FAILURE);
    }
    
    msgid_ = msgget(key, 0600);
    if (msgid_ == -1) {
        std::cerr << "BŁĄD: Nie można znaleźć kolejki komunikatów!\n";
        perror("msgget attach");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "IPCManager: Podłączono kolejkę komunikatów (ID: " << msgid_ << ")\n";
}

// 
// CLEANUP - Usuwanie zasobów (port_captain)
// 

void IPCManager::cleanup() {
    if (!is_creator_) {
        std::cout << "IPCManager: Ten proces nie utworzył zasobów, pomijam cleanup.\n";
        return;
    }
    
    std::cout << "IPCManager: Rozpoczynanie czyszczenia zasobów IPC...\n";
    
    destroyMutex();
    detachSharedMemory();
    destroySharedMemory();
    destroySemaphores();
    destroyMessageQueue();
    
    std::cout << "IPCManager: Wszystkie zasoby IPC usunięte.\n";
}

void IPCManager::detachSharedMemory() {
    if (shared_data_ != nullptr) {
        if (shmdt(shared_data_) == -1) {
            perror("shmdt");
        } else {
            std::cout << "IPCManager: Odłączono shared memory.\n";
        }
        shared_data_ = nullptr;
    }
}

void IPCManager::destroyMutex() {
    if (shared_data_ != nullptr) {
        if (pthread_mutex_destroy(&shared_data_->mutex) != 0) {
            perror("pthread_mutex_destroy");
        } else {
            std::cout << "IPCManager: Mutex zniszczony.\n";
        }
    }
}

void IPCManager::destroySharedMemory() {
    if (shm_id_ != -1) {
        if (shmctl(shm_id_, IPC_RMID, nullptr) == -1) {
            perror("shmctl IPC_RMID");
        } else {
            std::cout << "IPCManager: Shared memory usunięta.\n";
        }
        shm_id_ = -1;
    }
}

void IPCManager::destroySemaphores() {
    if (semid_ != -1) {
        if (semctl(semid_, 0, IPC_RMID) == -1) {
            perror("semctl IPC_RMID");
        } else {
            std::cout << "IPCManager: Semafory usunięte.\n";
        }
        semid_ = -1;
    }
}

void IPCManager::destroyMessageQueue() {
    if (msgid_ != -1) {
        if (msgctl(msgid_, IPC_RMID, nullptr) == -1) {
            perror("msgctl IPC_RMID");
        } else {
            std::cout << "IPCManager: Kolejka komunikatów usunięta.\n";
        }
        msgid_ = -1;
    }
}

// 
// STATIC - Sprawdzanie czy zasoby istnieją
// 

bool IPCManager::resourcesExist() {
    // Sprawdź czy plik rejs istnieje
    struct stat buffer;
    if (stat("rejs", &buffer) != 0) {
        return false;
    }
    
    key_t shm_key = ftok("rejs", 'R');
    if (shm_key == -1) return false;
    
    int shm_id = shmget(shm_key, sizeof(SharedData), 0600);
    return (shm_id != -1);
}