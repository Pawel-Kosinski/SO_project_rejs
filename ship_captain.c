#include "common.h"

SharedData *shared_data = NULL;
int shm_id;
int semid;
int msgid;


void send_message(long mtype, const char *text) {
    struct msgbuf msg;
    msg.mtype = mtype;
    strncpy(msg.mtext, text, sizeof(msg.mtext) - 1);
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

void alarm_handler(int sig) {
    pthread_mutex_lock(&shared_data->mutex);
    if (shared_data->loading_finished == 0 && shared_data->terminate != 1) {
        printf(BLUE "Kapitan Statku: Czas na zaladunek uplynal. \n" RESET);
        shared_data->loading_finished = 1;
    }
    pthread_mutex_unlock(&shared_data->mutex);
}

int main() {
    signal(SIGALRM, alarm_handler);

    key_t shm_key = ftok("rejs", 'R');
    if (shm_key == -1) {
        perror("ftok shm_key");
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(shm_key, sizeof(SharedData), 0600);
    if (shm_id < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    key_t sem_key = ftok("rejs", 'S');
    if (sem_key == -1) {
        perror("ftok sem_key");
        exit(EXIT_FAILURE);
    }

    semid = semget(sem_key, 2, 0600);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    key_t msg_key = ftok("rejs", 'M');
    if (msg_key == -1) {
        perror("ftok msg_key");
        exit(EXIT_FAILURE);
    }

    msgid = msgget(msg_key, 0600);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf(BLUE "Kapitan Statku: Rozpoczynam prace.\n" RESET);
    int voyages = 0;
    while (voyages < R && !shared_data->terminate) {
        voyages++;
        pthread_mutex_lock(&shared_data->mutex);
        shared_data->voyage_number = voyages;
        pthread_mutex_unlock(&shared_data->mutex);

        pthread_mutex_lock(&shared_data->mutex);
        shared_data->loading_finished = 0;
        shared_data->loading = 1;
        pthread_mutex_unlock(&shared_data->mutex);
        printf(BLUE "Kapitan Statku: Rozpoczecie zaladunku do rejsu %d. \n" RESET, voyages);
        send_message(MSG_TYPE_START_BOARDING, "Start boarding");
        alarm(T1);
        pthread_mutex_lock(&shared_data->mutex);
        while (shared_data->loading_finished == 0) {
            if (shared_data->passengers_on_board == N || shared_data->passengers_on_board == NUM_PASSENGERS) {
                printf(BLUE "Kapitan Statku: Osiagnieto maksymalna ilosc pasazerow na statku. Przedwczesne zakonczenie zaladunku.\n" RESET);
                shared_data->loading_finished = 1;
                pthread_mutex_unlock(&shared_data->mutex);
            } else if (shared_data->terminate == 1) {
                pthread_mutex_unlock(&shared_data->mutex);
                printf(BLUE "Kapitan Statku: Otrzymano sygnal o zakonczeniu rejsow.\n" RESET);
                break;
            } else {
                pthread_mutex_unlock(&shared_data->mutex);
                sleep(1);
            }
        }
        shared_data->boarding_allowed = 0;
        shared_data->unloading_finished = shared_data->passengers_on_board;

        if (shared_data->terminate != 1) {
            pthread_mutex_lock(&shared_data->mutex);
            shared_data->loading = 0;
            pthread_mutex_unlock(&shared_data->mutex);
            pthread_mutex_lock(&shared_data->mutex);
            if (shared_data->passengers_on_bridge > 0) {
                pthread_mutex_unlock(&shared_data->mutex);
                printf(BLUE "Kapitan Statku: Czekam, az pasazerowie opuszcza mostek. \n" RESET);

                while (1) {
                    pthread_mutex_lock(&shared_data->mutex);
                    if (shared_data->passengers_on_bridge == 0) {
                        pthread_mutex_unlock(&shared_data->mutex);
                        break;
                    }
                    pthread_mutex_unlock(&shared_data->mutex);
                    sleep(1);
                }
            } else {
                pthread_mutex_unlock(&shared_data->mutex);
            }
            printf(BLUE "Kapitan Statku: Rejs %d w trakcie.\n" RESET, voyages);
            sleep(T2);
            printf(BLUE "Kapitan Statku: Rejs %d zakonczony. Pasazerowie moga opuscic statek. \n" RESET, voyages);
            send_message(MSG_TYPE_START_UNLOADING, "Start unloading");
        }
        pthread_mutex_lock(&shared_data->mutex);
        shared_data->loading = 2;
        pthread_mutex_unlock(&shared_data->mutex);
        while (1) {
            pthread_mutex_lock(&shared_data->mutex);
            if (shared_data->passengers_on_board == 0) {
                shared_data->unloading_allowed = 0;
                pthread_mutex_unlock(&shared_data->mutex);
                printf(BLUE "Kapitan Statku: Wszyscy pasazerowie opuscili statek.\n" RESET);
                break;
            }
            pthread_mutex_unlock(&shared_data->mutex);
            sleep(1);
        }
        while (1) {
            pthread_mutex_lock(&shared_data->mutex);
            if (shared_data->unloading_finished == 0) {
                shared_data->loading = 0;
                pthread_mutex_unlock(&shared_data->mutex);
                printf(BLUE "Kapitan Statku: Wszyscy pasazerowie opuscili mostek.\n" RESET);
                break;
            }
            pthread_mutex_unlock(&shared_data->mutex);
           sleep(1);
        }

        printf(BLUE "Kapitan Statku: Rozladunek zakonczony dla rejsu %d. \n" RESET, voyages);
    }
    printf(BLUE "Kapitan Statku: Koncze prace. \n" RESET);
    voyages++;
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->voyage_number = voyages;
    pthread_mutex_unlock(&shared_data->mutex);
    if (shmdt(shared_data) == -1) {
        perror("shmdt");
    }
    return 0;
}