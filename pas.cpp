#include "pas.hpp"

// Implementacja semaforów tutaj, żeby nie było błędów linkera
void Passenger::sem_p(int semnum) {
    struct sembuf op;
    op.sem_num = (unsigned short)semnum;
    op.sem_op = -1;
    op.sem_flg = 0;
    while (semop(semid, &op, 1) == -1) {
        if (errno != EINTR) { perror("semop P"); exit(EXIT_FAILURE); }
    }
}

void Passenger::sem_v(int semnum) {
    struct sembuf op;
    op.sem_num = (unsigned short)semnum;
    op.sem_op = 1;
    op.sem_flg = 0;
    while (semop(semid, &op, 1) == -1) {
        if (errno != EINTR) { perror("semop V"); exit(EXIT_FAILURE); }
    }
}

Passenger::Passenger(int id, int semid, int msgid, SharedData *sd)
    : passenger_id(id), semid(semid), msgid(msgid), sd(sd) {}

void Passenger::enter_bridge() {
    sleep_ms((std::rand() % 3000 + 2000));
    
    sem_p(BRIDGE_SEM);
    
    pthread_mutex_lock(&sd->mutex);
    sd->passengers_on_bridge++;
    std::cout << GREEN << "Pasazer " << passenger_id << " wchodzi na mostek. Na mostku: " 
              << sd->passengers_on_bridge << RESET << std::endl;
    pthread_mutex_unlock(&sd->mutex);
    
    sleep_ms((std::rand() % 2000 + 1000));
}

void Passenger::exit_bridge() {
    pthread_mutex_lock(&sd->mutex);
    sem_v(BRIDGE_SEM);
    sd->passengers_on_bridge--;
    std::cout << GREEN << "Pasazer " << passenger_id << " schodzi z mostku. Na mostku: " 
              << sd->passengers_on_bridge << RESET << std::endl;
    pthread_mutex_unlock(&sd->mutex);
}

void Passenger::enter_ship() {
    // UWAGA: Funkcja zakłada, że mutex jest ZABLOKOWANY przez wywołującego (tryToBoard)
    sem_p(SHIP_SEM);
    
    sd->passengers_on_bridge--; 
    sem_v(BRIDGE_SEM); 
    
    std::cout << GREEN << "Pasazer " << passenger_id << " wchodzi na statek. Na statku: " 
              << sd->passengers_on_board << RESET << std::endl;
    
    pthread_mutex_unlock(&sd->mutex); // Zwalniamy mutex tutaj
}

void Passenger::exit_ship() {
    sleep_ms((std::rand() % 3000 + 1000));
    sem_v(SHIP_SEM);
    
    pthread_mutex_lock(&sd->mutex);
    sd->passengers_on_board--;
    std::cout << GREEN << "Pasazer " << passenger_id << " opuszcza statek. Na statku: " 
              << sd->passengers_on_board << RESET << std::endl;
    pthread_mutex_unlock(&sd->mutex);
}

bool Passenger::isBoardingAllowed() {
    sleep_ms(std::rand() % 5000); 

    
    while (true) {
        pthread_mutex_lock(&sd->mutex);
        if (sd->boarding_allowed == 1) {
            pthread_mutex_unlock(&sd->mutex);
            enter_bridge();
            return true; // MOŻNA WCHODZIĆ
        } else if (sd->voyage_number > R) {
            pthread_mutex_unlock(&sd->mutex);
            return false; // KONIEC PRACY
        } else {
            pthread_mutex_unlock(&sd->mutex);
            sleep(1);
        }
    }
}

bool Passenger::tryToBoard() {
    pthread_mutex_lock(&sd->mutex);
    
    if (sd->loading == 1) {
        // Sprawdzamy miejsce i czas
        if (sd->passengers_on_board < N && sd->loading_finished == 0) {
            sd->passengers_on_board++;
            // enter_ship zwalnia mutex!
            enter_ship(); 
            return true; // SUKCES
        } 
        else {
            // Brak miejsca lub koniec czasu
            pthread_mutex_unlock(&sd->mutex);
            exit_bridge();
            sleep_ms(3000); // Karny spacer
            return false; // PORAŻKA
        }
    } 
    else {
        // Załadunek się skończył zanim weszliśmy
        pthread_mutex_unlock(&sd->mutex);
        exit_bridge();
        sleep_ms(3000);
        return false; // PORAŻKA
    }
}

void Passenger::tryToUnload() {
    exit_ship();
    // Powrót przez mostek
    while(true) {
        pthread_mutex_lock(&sd->mutex);
        if (sd->loading == 2) { 
            pthread_mutex_unlock(&sd->mutex);
            enter_bridge();
            break;
        } else {
            pthread_mutex_unlock(&sd->mutex);
            sleep(1);
        }
    }
    exit_bridge();
    pthread_mutex_lock(&sd->mutex);
    sd->unloading_finished--;
    pthread_mutex_unlock(&sd->mutex);
}

void Passenger::voyageAbort() {
    exit_ship();
    enter_bridge();
    exit_bridge();
    pthread_mutex_lock(&sd->mutex);
    sd->unloading_finished--;
    pthread_mutex_unlock(&sd->mutex);
}