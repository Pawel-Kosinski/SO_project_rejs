#include "common.h"

SharedData *shared_data = NULL;
int shm_id;
int semid;
int msgid;

void sem_p(int semid, unsigned short semnum) {
    struct sembuf op;
    op.sem_num = semnum;   
    op.sem_op = -1;        
    op.sem_flg = 0;        

    int result = semop(semid, &op, 1);
        if (result == -1) {
            if(errno == EINTR){
                semop(semid, &op, 1);
            }
            else {
            perror("semop P");
            exit(EXIT_FAILURE);
            }
    }
}

void sem_v(int semid, unsigned short semnum) {
    struct sembuf op;
    op.sem_num = semnum;   
    op.sem_op = 1;        
    op.sem_flg = 0;        

    int result = semop(semid, &op, 1);
        if (result == -1) {
            if(errno == EINTR){
                semop(semid, &op, 1);
            }
            else {
            perror("semop P");
            exit(EXIT_FAILURE);
            }
    }
}

void send_message(long mtype, const char *text) {
    struct msgbuf msg;
    msg.mtype = mtype;
    strncpy(msg.mtext, text, sizeof(msg.mtext) - 1);
    msg.mtext[sizeof(msg.mtext) - 1] = '\0';

    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

int main() {
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

    semid = semget(sem_key, 4, 0600);
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

    printf("Kapitan Statku: Rozpoczynam prace.\n");
    for (int voyages = 1; voyages <= R; voyages++) {

        pthread_mutex_lock(&shared_data->mutex);
        shared_data->voyage_number = voyages;
        pthread_mutex_unlock(&shared_data->mutex);

        pthread_mutex_lock(&shared_data->mutex);
        shared_data->loading = 1;
        pthread_mutex_unlock(&shared_data->mutex);
        printf("Kapitan Statku: Rozpoczecie zaladunku do rejsu %d. \n", voyages);
        send_message(MSG_TYPE_START_BOARDING, "Start boarding");

        sleep(T1);

        pthread_mutex_lock(&shared_data->mutex);
        shared_data->boarding_allowed = 0; 
        shared_data->loading = 0;
        pthread_mutex_unlock(&shared_data->mutex);
        printf("Kapitan Statku: Zaladunek zakonczony dla rejsu %d. \n", voyages);
        
        pthread_mutex_lock(&shared_data->mutex);
        if (shared_data->passengers_on_bridge > 0) {
            pthread_mutex_unlock(&shared_data->mutex);
            printf("Kapitan Statku: Czekam, az pasazerowie opuszcza mostek. \n");

            while(1) {
                pthread_mutex_lock(&shared_data->mutex);
                if (shared_data->passengers_on_bridge == 0) {
                    pthread_mutex_unlock(&shared_data->mutex);
                    break;
                }
                pthread_mutex_unlock(&shared_data->mutex);
                sleep(1);
            }
        }
        else {
            pthread_mutex_unlock(&shared_data->mutex);
        }

        printf("Kapitan Statku: Rejs %d w trakcie.\n", voyages);
        sleep(T2);

        printf("Kapitan Statku: Rejs %d zakonczony. Pasazerowie moga opuscic statek. \n", voyages);
        pthread_mutex_lock(&shared_data->mutex);
        shared_data->loading = 2;
        pthread_mutex_unlock(&shared_data->mutex);

        send_message(MSG_TYPE_START_UNLOADING, "Start unloading");

        while (1) {
            pthread_mutex_lock(&shared_data->mutex);
            if (shared_data->passengers_on_board == 0) {
                shared_data->unloading_allowed = 0;
                pthread_mutex_unlock(&shared_data->mutex);
                printf("Kapitan Statku: Wszyscy pasazerowie opuscili statek.\n");
                break;
            }
            pthread_mutex_unlock(&shared_data->mutex);
            sleep(1); 
        }
        sleep(10);
        // Poczekanie, az wszyscy pasazerowie opuszcza mostek
        while (1) {
            pthread_mutex_lock(&shared_data->mutex);
            if (shared_data->passengers_on_bridge == 0) {
                shared_data->loading = 0;
                pthread_mutex_unlock(&shared_data->mutex);
                printf("Kapitan Statku: Wszyscy pasazerowie opuscili mostek.\n");
                break;
            }
            pthread_mutex_unlock(&shared_data->mutex);
            sleep(1);
        }

        printf("Kapitan Statku: Rozladunek zakonczony dla rejsu %d. \n", voyages);


    }
    printf("Kapitan Statku: Wykonano maksymalna liczbe rejsow (%d). Koncze prace. \n", R);
        if (shmdt(shared_data) == -1) {
        perror("shmdt");
    }
    return 0;
}
