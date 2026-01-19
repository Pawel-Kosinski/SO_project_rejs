#include "ShipState.hpp"

ShipState::ShipState(int semid, SharedData* shared_data)
    : semid_(semid), shared_data_(shared_data) {}

void ShipState::sem_p() {
    struct sembuf op;
    op.sem_num = SHIP_SEM;
    op.sem_op = -1;
    op.sem_flg = 0;
    while (semop(semid_, &op, 1) == -1) {
        if (errno != EINTR) {
            perror("semop P ship");
            exit(EXIT_FAILURE);
        }
    }
}

void ShipState::sem_v() {
    struct sembuf op;
    op.sem_num = SHIP_SEM;
    op.sem_op = 1;
    op.sem_flg = 0;
    while (semop(semid_, &op, 1) == -1) {
        if (errno != EINTR) {
            perror("semop V ship");
            exit(EXIT_FAILURE);
        }
    }
}

// 
// Implementacja IPassengerShipAccess
// 

bool ShipState::canBoard() const {
    pthread_mutex_lock(&shared_data_->mutex);
    bool result = (shared_data_->boarding_allowed == 1 && 
                  shared_data_->loading == 1 &&
                  shared_data_->loading_finished == 0);
    pthread_mutex_unlock(&shared_data_->mutex);
    return result;
}

bool ShipState::hasSpace() const {
    pthread_mutex_lock(&shared_data_->mutex);
    bool result = (shared_data_->passengers_on_board < N);
    pthread_mutex_unlock(&shared_data_->mutex);
    return result;
}

void ShipState::board(int passenger_id) {
    sem_p();  // Zajmij miejsce na statku
    
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->passengers_on_board++;
    std::cout << GREEN << "Pasażer " << passenger_id 
              << " wszedł na statek. Na statku: " 
              << shared_data_->passengers_on_board << RESET << std::endl;
    pthread_mutex_unlock(&shared_data_->mutex);
}

void ShipState::disembark(int passenger_id) {
    //sleep_ms(std::rand() % 3000 + 1000);
    
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->passengers_on_board--;
    std::cout << GREEN << "Pasażer " << passenger_id 
              << " zszedł ze statku. Na statku: " 
              << shared_data_->passengers_on_board << RESET << std::endl;
    pthread_mutex_unlock(&shared_data_->mutex);
    
    sem_v();  // Zwolnij miejsce na statku
}

int ShipState::getVoyageNumber() const {
    pthread_mutex_lock(&shared_data_->mutex);
    int num = shared_data_->voyage_number;
    pthread_mutex_unlock(&shared_data_->mutex);
    return num;
}

// 
// Implementacja IShipCaptainControl
// 

void ShipState::startBoarding() {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->loading = 1;
    shared_data_->loading_finished = 0;
    pthread_mutex_unlock(&shared_data_->mutex);
}

void ShipState::finishBoarding() {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->boarding_allowed = 0;
    shared_data_->unloading_finished = shared_data_->passengers_on_board;
    pthread_mutex_unlock(&shared_data_->mutex);
}

void ShipState::startVoyage() {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->loading = 0;  // Statek płynie
    pthread_mutex_unlock(&shared_data_->mutex);
}

void ShipState::finishVoyage() {
    // Rejs zakończony - brak akcji na stanie
}

void ShipState::startUnloading() {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->loading = 2;  // Rozładunek
    pthread_mutex_unlock(&shared_data_->mutex);
}

int ShipState::getPassengersOnBoard() const {
    pthread_mutex_lock(&shared_data_->mutex);
    int count = shared_data_->passengers_on_board;
    pthread_mutex_unlock(&shared_data_->mutex);
    return count;
}

int ShipState::getPassengersOnBridge() const {
    pthread_mutex_lock(&shared_data_->mutex);
    int count = shared_data_->passengers_on_bridge;
    pthread_mutex_unlock(&shared_data_->mutex);
    return count;
}

bool ShipState::isFull() const {
    pthread_mutex_lock(&shared_data_->mutex);
    bool full = (shared_data_->passengers_on_board >= N || 
                 shared_data_->passengers_on_board >= NUM_PASSENGERS);
    pthread_mutex_unlock(&shared_data_->mutex);
    return full;
}

bool ShipState::isTerminated() const {
    pthread_mutex_lock(&shared_data_->mutex);
    bool terminated = (shared_data_->terminate == 1);
    pthread_mutex_unlock(&shared_data_->mutex);
    return terminated;
}

void ShipState::setVoyageNumber(int voyage_num) {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->voyage_number = voyage_num;
    pthread_mutex_unlock(&shared_data_->mutex);
}

void ShipState::setFlag() {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->alarm_fired = 1;
    pthread_mutex_unlock(&shared_data_->mutex);
}

bool ShipState::getFlag() {
    pthread_mutex_lock(&shared_data_->mutex);
    auto res = shared_data_->alarm_fired;
    if (res) {
        shared_data_->alarm_fired = 0;
    }
    pthread_mutex_unlock(&shared_data_->mutex);
    return res;
}

bool ShipState::isLoadingFinished() {
    pthread_mutex_lock(&shared_data_->mutex);
    auto res = shared_data_->loading_finished;
    pthread_mutex_unlock(&shared_data_->mutex);
    return res;
}
// 
// Implementacja IPortCaptainShipControl
// 

void ShipState::allowBoarding() {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->boarding_allowed = 1;
    pthread_mutex_unlock(&shared_data_->mutex);
    std::cout << RED << "Kapitan Portu: Pozwalam na załadunek.\n" << RESET;
}

void ShipState::forceFinishBoarding() {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->loading_finished = 1;
    pthread_mutex_unlock(&shared_data_->mutex);
    std::cout << RED << "Kapitan Portu: Wymuszam zakończenie załadunku.\n" << RESET;
    while (true)
    {
        pthread_mutex_lock(&shared_data_->mutex);
        if (shared_data_->loading == 2) {
            pthread_mutex_unlock(&shared_data_->mutex);
            break;
        }
        pthread_mutex_unlock(&shared_data_->mutex);
        sleep(1);
    }
}

void ShipState::initiateEmergencyStop() {
    pthread_mutex_lock(&shared_data_->mutex);
    shared_data_->terminate = 1;
    shared_data_->voyage_number = R + 1;
    shared_data_->loading = 2;
    shared_data_->boarding_allowed = 0;
    pthread_mutex_unlock(&shared_data_->mutex);
    std::cout << RED << "Kapitan Portu: Awaryjne zatrzymanie!\n" << RESET;
}