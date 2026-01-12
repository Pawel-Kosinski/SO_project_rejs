#include "common.hpp"
#include "pas.hpp" // INCLUDE HPP, NIE CPP!

SharedData *shared_data = nullptr;
int shm_id;
int semid;
int msgid;

struct PassengerArgs {
    int passenger_id;
    int semid;
    int msgid;
    SharedData *shared_data;
};

void PassengerLogic(PassengerArgs args) {
    Passenger passenger(args.passenger_id, args.semid, args.msgid, args.shared_data);
    std::cout << GREEN << "Pasazer " << args.passenger_id << " przybyl do portu.\n" << RESET;
    while (true) {
        if (!passenger.isBoardingAllowed()) {
            break;
        }
        
        if (passenger.tryToBoard()) {
            struct msgbuf_custom msg;
            if (msgrcv(msgid, &msg, sizeof(msg.mtext), MSG_TYPE_UNLOADING_ALLOWED, 0) == -1) return;

            if (strcmp(msg.mtext, "Unloading is now allowed.") == 0) {
                passenger.tryToUnload();
            }
            else if (strcmp(msg.mtext, "Abort voyages") == 0) {
                passenger.voyageAbort();
            }
        } 
        else {
            // PORAŻKA: tryToBoard zwróciło false (brak miejsca/czasu).
            // Pasażer już zszedł z mostku (wewnątrz tryToBoard).
            // Pętla while(true) leci od nowa -> passenger wraca do isBoardingAllowed.
            continue; 
        }
                
        pthread_mutex_lock(&shared_data->mutex);
        if (shared_data->voyage_number >= R) {
            pthread_mutex_unlock(&shared_data->mutex);
            break;
        }
        pthread_mutex_unlock(&shared_data->mutex);
    }
    std::cout << GREEN << "Pasazer " << args.passenger_id << " idzie do domu.\n" << RESET;
}

int main() {
    std::srand(std::time(nullptr) ^ getpid());

    key_t shm_key = ftok("rejs", 'R');
    shm_id = shmget(shm_key, sizeof(SharedData), 0600);
    shared_data = static_cast<SharedData*>(shmat(shm_id, nullptr, 0));

    key_t sem_key = ftok("rejs", 'S');
    semid = semget(sem_key, 2, 0600);

    key_t msg_key = ftok("rejs", 'M');
    msgid = msgget(msg_key, 0600);

    std::vector<std::thread> threads;
    threads.reserve(NUM_PASSENGERS);

    std::cout << GREEN << "Generowanie pasazerow (obiektowo)...\n" << RESET;

    for (int i = 0; i < NUM_PASSENGERS; i++) {
        PassengerArgs args = {i + 1, semid, msgid, shared_data};
        threads.emplace_back(PassengerLogic, args);
        sleep_ms(200 + (std::rand() % 300)); 
        
        pthread_mutex_lock(&shared_data->mutex);
        shared_data->passengers++;
        int terminate_now = shared_data->terminate;
        pthread_mutex_unlock(&shared_data->mutex);

        if (terminate_now == 1) break;
    }

    for (auto &t : threads) {
        if (t.joinable()) t.join();
    }

    shared_data->passengers = 0;
    shmdt(shared_data);
    return 0;
}