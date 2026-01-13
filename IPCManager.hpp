// IPCManager.hpp
#ifndef IPC_MANAGER_HPP
#define IPC_MANAGER_HPP

#include "common.hpp"
#include <memory>

enum class IPCMode {
    CREATE,  // Tworzy nowe zasoby (używa port_captain)
    ATTACH   // Podłącza się do istniejących (używają ship_captain, passenger)
};

class IPCManager {
public:
    IPCManager();
    ~IPCManager();
    
    // Tworzenie zasobów (tylko port_captain)
    void initialize();
    
    // Podłączanie się do istniejących zasobów (ship_captain, passenger)
    void attach();
    
    // Czyszczenie zasobów (tylko port_captain)
    void cleanup();
    
    // Gettery dla zasobów
    int getSemaphoreId() const { return semid_; }
    int getMessageQueueId() const { return msgid_; }
    SharedData* getSharedData() const { return shared_data_; }
    
    // Sprawdzenie czy zasoby istnieją
    static bool resourcesExist();

private:
    void createSharedMemory();
    void attachSharedMemory();
    
    void createSemaphores();
    void attachSemaphores();
    
    void createMessageQueue();
    void attachMessageQueue();
    
    void initializeSharedData();
    void initializeMutex();
    
    void detachSharedMemory();
    void destroySharedMemory();
    void destroySemaphores();
    void destroyMessageQueue();
    void destroyMutex();
    
    int shm_id_;
    int semid_;
    int msgid_;
    SharedData* shared_data_;
    bool is_creator_;  // Czy ten proces utworzył zasoby?
};

#endif // IPC_MANAGER_HPP