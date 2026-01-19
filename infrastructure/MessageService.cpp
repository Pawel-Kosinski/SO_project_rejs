#include "MessageService.hpp"

MessageService::MessageService(int msgid, SharedData* shared_data)
    : msgid_(msgid), shared_data_(shared_data) {}

void MessageService::sendMessage(long mtype, const char* text) {
    struct msgbuf_custom msg;
    msg.mtype = mtype;
    std::strncpy(msg.mtext, text, sizeof(msg.mtext) - 1);
    msg.mtext[sizeof(msg.mtext) - 1] = '\0';
    
    int ret;
    while ((ret = msgsnd(msgid_, &msg, sizeof(msg.mtext), 0)) == -1) {
        if (errno == EINTR) {
            continue;
        } else {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
    }
}

// Dla kapitana statku
void MessageService::sendStartBoardingMessage() {
    sendMessage(MSG_TYPE_START_BOARDING, "Start boarding");
}

void MessageService::sendStartUnloadingMessage() {
    sendMessage(MSG_TYPE_START_UNLOADING, "Start unloading");
}

// Dla kapitana portu
msgbuf_custom MessageService::waitForMessage() {
    struct msgbuf_custom msg;
    while (true)
    {
    ssize_t msg_r = msgrcv(msgid_, &msg, sizeof(msg.mtext), 0, 0);
        if (msg_r == -1) {
            if (errno == EINTR) {
                //sleep_ms(5000);
                continue;
            } else {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
        }
        return msg;
    }
}

void MessageService::sendUnloadingAllowedMessages(int count) {
    struct msgbuf_custom unloading_msg;
    unloading_msg.mtype = MSG_TYPE_UNLOADING_ALLOWED;
    std::strncpy(unloading_msg.mtext, "Unloading is now allowed.", sizeof(unloading_msg.mtext) - 1);
    unloading_msg.mtext[sizeof(unloading_msg.mtext) - 1] = '\0';

    for (int i = 0; i < count; i++) {
        if (msgsnd(msgid_, &unloading_msg, sizeof(unloading_msg.mtext), 0) == -1) {
            perror("msgsnd unloading_allowed");
            exit(EXIT_FAILURE);
        }
    }
}

void MessageService::sendAbortMessages(int count) {
    struct msgbuf_custom terminate_msg;
    terminate_msg.mtype = MSG_TYPE_UNLOADING_ALLOWED;
    std::strncpy(terminate_msg.mtext, "Abort voyages", sizeof(terminate_msg.mtext) - 1);
    terminate_msg.mtext[sizeof(terminate_msg.mtext) - 1] = '\0';

    for (int i = 0; i < count; i++) {
        if (msgsnd(msgid_, &terminate_msg, sizeof(terminate_msg.mtext), 0) == -1) {
            perror("msgsnd abort");
            exit(EXIT_FAILURE);
        }
    }
}

// Dla pasażerów
msgbuf_custom MessageService::waitForUnloadingMessage() {
    struct msgbuf_custom msg;
    if (msgrcv(msgid_, &msg, sizeof(msg.mtext), MSG_TYPE_UNLOADING_ALLOWED, 0) == -1) {
        perror("msgrcv unloading");
        exit(EXIT_FAILURE);
    }
    return msg;
}

bool MessageService::isBoardingAllowed() const {
    pthread_mutex_lock(&shared_data_->mutex);
    bool allowed = (shared_data_->boarding_allowed == 1);
    bool ended = (shared_data_->voyage_number > R);
    pthread_mutex_unlock(&shared_data_->mutex);
    
    return allowed || !ended;  
}