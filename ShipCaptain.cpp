#include "ShipCaptain.hpp"

ShipCaptain::ShipCaptain(int semid, int msgid, SharedData *shared_data)
    : semid(semid), msgid(msgid), shared_data(shared_data) {}

void ShipCaptain::sendMessage(long mtype, const char *text) {
    struct msgbuf_custom msg;
    msg.mtype = mtype;
    std::strncpy(msg.mtext, text, sizeof(msg.mtext) - 1);
    msg.mtext[sizeof(msg.mtext) - 1] = '\0';
    
    int ret;
    while ((ret = msgsnd(msgid, &msg, sizeof(msg.mtext), 0)) == -1) {
        if (errno == EINTR) {
            continue;
        } else {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
    }
}

void ShipCaptain::startBoarding() {
    voyages++;
        
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->voyage_number = voyages;
    shared_data->loading_finished = 0;
    shared_data->loading = 1;
    pthread_mutex_unlock(&shared_data->mutex);

    std::cout << BLUE << "Kapitan Statku: Rozpoczecie zaladunku do rejsu " << voyages << ".\n" << RESET;
    sendMessage(MSG_TYPE_START_BOARDING, "Start boarding");
}

void ShipCaptain::waitWhileBoarding() {
    alarm(T1_SEC); // Ustawienie budzika

    // Oczekiwanie na koniec załadunku
    pthread_mutex_lock(&shared_data->mutex);
    while (shared_data->loading_finished == 0) {
        if (shared_data->passengers_on_board == N || shared_data->passengers_on_board == NUM_PASSENGERS) {
            std::cout << BLUE << "Kapitan Statku: Max pasazerow. Przedwczesne zakonczenie zaladunku.\n" << RESET;
            shared_data->loading_finished = 1;
            pthread_mutex_unlock(&shared_data->mutex);
        } else if (shared_data->terminate == 1) {
            pthread_mutex_unlock(&shared_data->mutex);
            std::cout << BLUE << "Kapitan Statku: Sygnal o zakonczeniu rejsow.\n" << RESET;
            break;
        } else {
            pthread_mutex_unlock(&shared_data->mutex);
            sleep(1);
            pthread_mutex_lock(&shared_data->mutex); // Ponowne zajęcie przed pętlą while
        }
    }
}

void ShipCaptain::finishBoarding() {
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->boarding_allowed = 0;
    shared_data->unloading_finished = shared_data->passengers_on_board;
    pthread_mutex_unlock(&shared_data->mutex);
}

void ShipCaptain::startVoyage() {
    pthread_mutex_lock(&shared_data->mutex);
    if (shared_data->terminate != 1) {
        shared_data->loading = 0; // Statek płynie
        int bridge_count = shared_data->passengers_on_bridge;
        pthread_mutex_unlock(&shared_data->mutex);

        if (bridge_count > 0) {
            std::cout << BLUE << "Kapitan Statku: Czekam, az pasazerowie opuszcza mostek.\n" << RESET;
            while (true) {
                pthread_mutex_lock(&shared_data->mutex);
                if (shared_data->passengers_on_bridge == 0) {
                    pthread_mutex_unlock(&shared_data->mutex);
                    break;
                }
                pthread_mutex_unlock(&shared_data->mutex);
                sleep(1);
            }
        }
    }
    std::cout << BLUE << "Kapitan Statku: Rejs " << voyages << " w trakcie.\n" << RESET;
    sleep(T2_SEC);

}

void ShipCaptain::finishVoyage() {
    std::cout << BLUE << "Kapitan Statku: Rejs " << voyages << " zakonczony. Pasazerowie moga opuscic statek.\n" << RESET;
    sendMessage(MSG_TYPE_START_UNLOADING, "Start unloading");
}

void ShipCaptain::shipUnload() {
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->loading = 2; // Rozładunek
    pthread_mutex_unlock(&shared_data->mutex);

    // Oczekiwanie na opuszczenie statku
    while (true) {
        pthread_mutex_lock(&shared_data->mutex);
        if (shared_data->passengers_on_board == 0) {
            shared_data->unloading_allowed = 0;
            pthread_mutex_unlock(&shared_data->mutex);
            std::cout << BLUE << "Kapitan Statku: Wszyscy pasazerowie opuscili statek.\n" << RESET;
            break;
        }
        pthread_mutex_unlock(&shared_data->mutex);
        sleep(1);
    }
}

void ShipCaptain::bridgeUnload() {
    while (true) {
        pthread_mutex_lock(&shared_data->mutex);
        if (shared_data->unloading_finished == 0) {
            shared_data->loading = 0;
            pthread_mutex_unlock(&shared_data->mutex);
            std::cout << BLUE << "Kapitan Statku: Wszyscy pasazerowie opuscili mostek.\n" << RESET;
            break;
        }
        pthread_mutex_unlock(&shared_data->mutex);
        sleep(1);
    }

    std::cout << BLUE << "Kapitan Statku: Rozladunek zakonczony dla rejsu " << voyages << ".\n" << RESET;
}

void ShipCaptain::end() {
    std::cout << BLUE << "Kapitan Statku: Koncze prace.\n" << RESET;
    
    // Zaktualizuj numer rejsu na koniec, żeby pasażerowie wiedzieli że koniec
    voyages++;
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->voyage_number = voyages;
    pthread_mutex_unlock(&shared_data->mutex);

    shmdt(shared_data);
}