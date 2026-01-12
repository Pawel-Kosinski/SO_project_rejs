#include "PortCaptain.hpp"

PortCaptain::PortCaptain(int semid, int msgid, SharedData *sd)
    : semid(semid), msgid(msgid), shared_data(sd) {}

msgbuf_custom PortCaptain::waitForMessage() {
    struct msgbuf_custom msg;
    ssize_t msg_r = msgrcv(msgid, &msg, sizeof(msg.mtext), 0, 0);
    
    if (msg_r == -1) {
        if (errno == EINTR) {
            std::cout << RED << "msgrcv przerwane przez sygnal, ponawiam...\n" << RESET;
        } else {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
    }
    return msg;
}

void PortCaptain::startBoarding() {
    std::cout << RED << "Kapitan Portu: Otrzymano sygnal rozpoczecia zaladunku.\n" << RESET;
    sleep_ms((std::rand() % 2000 + 2000)); 

    pthread_mutex_lock(&shared_data->mutex);
    shared_data->boarding_allowed = 1;
    pthread_mutex_unlock(&shared_data->mutex);
    std::cout << RED << "Kapitan Portu: Wyslano komunikat do pasazerow o rozpoczeciu zaladunku.\n" << RESET;
}

void PortCaptain::startUnloading() {
    std::cout << RED << "Kapitan Portu: Otrzymano sygnal rozpoczecia rozladunku.\n" << RESET;

    struct msgbuf_custom unloading_msg;
    unloading_msg.mtype = MSG_TYPE_UNLOADING_ALLOWED;
    std::strncpy(unloading_msg.mtext, "Unloading is now allowed.", sizeof(unloading_msg.mtext) - 1);
    unloading_msg.mtext[sizeof(unloading_msg.mtext) - 1] = '\0';

    for (int i = 0; i < N; i++) {
        if (msgsnd(msgid, &unloading_msg, sizeof(unloading_msg.mtext), 0) == -1) {
            perror("msgsnd unloading_allowed");
            exit(EXIT_FAILURE);
        }
    }
    std::cout << RED << "Kapitan Portu: Wyslano komunikat do pasazerow o rozpoczeciu rozladunku.\n" << RESET;
}

void PortCaptain::endVoyages() {
    pthread_mutex_lock(&shared_data->mutex);
    if (shared_data->voyage_number >= R) {
        pthread_mutex_unlock(&shared_data->mutex);
        std::cout << RED << "Kapitan Portu: Osiagnieto maksymalna liczbe rejsow.\n" << RESET;
        voyages_ended = true;
    }
    pthread_mutex_unlock(&shared_data->mutex);
}