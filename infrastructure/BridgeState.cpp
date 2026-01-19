#include "BridgeState.hpp"

BridgeState::BridgeState(int semid, SharedData* shared_data)
    : semid_(semid), shared_data_(shared_data) {}

void BridgeState::sem_p() {
    struct sembuf op;
    op.sem_num = BRIDGE_SEM;
    op.sem_op = -1;
    op.sem_flg = 0;
    while (semop(semid_, &op, 1) == -1) {
        if (errno != EINTR) {
            perror("semop P bridge");
            std::exit(EXIT_FAILURE);
        }
    }
}

void BridgeState::sem_v() {
    struct sembuf op;
    op.sem_num = BRIDGE_SEM;
    op.sem_op = 1;
    op.sem_flg = 0;
    while (semop(semid_, &op, 1) == -1) {
        if (errno != EINTR) {
            perror("semop V bridge");
            std::exit(EXIT_FAILURE);
        }
    }
}

void BridgeState::enter(int passenger_id) {
    sleep_ms(std::rand() % 2000);
    
    sem_p();  // Zajmij miejsce na mostku
    
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->passengers_on_bridge++;
    std::cout << GREEN << "Pasażer " << passenger_id << " wchodzi na mostek. Na mostku: " 
              << shared_data_->passengers_on_bridge << RESET << std::endl;
    pthread_mutex_unlock(&shared_data_->mutex);
    
    sleep_ms(std::rand() % 2000);
}

void BridgeState::exit(int passenger_id) {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->passengers_on_bridge--;
    std::cout << GREEN << "Pasażer " << passenger_id << " schodzi z mostku. Na mostku: " 
              << shared_data_->passengers_on_bridge << RESET << std::endl;
    pthread_mutex_unlock(&shared_data_->mutex);
    
    sem_v();  // Zwolnij miejsce na mostku
}

int BridgeState::getPassengersOnBridge() const {
    pthread_mutex_lock(&shared_data_->mutex);
    int count = shared_data_->passengers_on_bridge;
    pthread_mutex_unlock(&shared_data_->mutex);
    return count;
}