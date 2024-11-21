#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include "a2_helper.h"

sem_t sem_P3T5;
sem_t sem_P3T2;
sem_t sem_limit_threads_P5;
sem_t sem_P5T11;
sem_t sem_barier_P5;
sem_t sem_counter_equal_4_P5;
sem_t sem_counter_P5;
int counter_P5 = 0;

 sem_t *sem_P3T4;
 sem_t *sem_P4T2;

void *thread_function_process_3(void *param) {
    int *id = (int *) param;

    if(*id == 4) {
        sem_wait(sem_P4T2);
    }

    if(*id == 5) {
        sem_wait(&sem_P3T5);
    }

    info(BEGIN, 3, *id);

    if(*id == 2) {
        sem_post(&sem_P3T5);
        sem_wait(&sem_P3T2);
    }
    
    info(END, 3, *id);

    if(*id == 5) {
        sem_post(&sem_P3T2);
    }

    if(*id == 4) {
        sem_post(sem_P3T4);
    }

    return NULL;
}

void *thread_function_process_4(void *param) {
    int *id = (int *) param;

    if(*id == 3) {
        sem_wait(sem_P3T4);
    }

    info(BEGIN, 4, *id);

    
    
    info(END, 4, *id);

    if(*id == 2) {
        sem_post(sem_P4T2);
    }

    return NULL;
}

void *thread_function_process_5(void *param) {
    int *id = (int *) param;
    
    if(*id != 11) {
        sem_wait(&sem_P5T11);
        sem_post(&sem_P5T11);
    }

    sem_wait(&sem_limit_threads_P5);
    info(BEGIN, 5, *id);

    sem_wait(&sem_counter_P5);
    counter_P5++;
    if(counter_P5 == 4) {
        sem_post(&sem_counter_equal_4_P5);
    }
    sem_post(&sem_counter_P5);

    if(*id == 11) {
        sem_post(&sem_P5T11);
        sem_wait(&sem_counter_equal_4_P5);
    }
    else {
        sem_wait(&sem_barier_P5);
        sem_post(&sem_barier_P5);
    }

    info(END, 5, *id);

    if(*id == 11) {
        for(int i = 0; i < 4; i++) {
            sem_post(&sem_barier_P5);
        }
    }
    sem_post(&sem_limit_threads_P5);

    return NULL;
}

int main(){
    init();

    sem_P3T4 = sem_open("/P3T4", O_CREAT, 0644, 0);
    sem_P4T2 = sem_open("/P4T2", O_CREAT, 0644, 0);

    info(BEGIN, 1, 0);

    //P2
    if(fork() == 0) {
        info(BEGIN, 2, 0);

        //P6
        if(fork() == 0) {
            info(BEGIN, 6, 0);

            //P7
            if(fork() == 0) {
                info(BEGIN, 7, 0);

                //P8
                if(fork() == 0) {
                    info(BEGIN, 8, 0);

                    info(END, 8, 0);
                    exit(0);
                }

                wait(NULL);
                info(END, 7, 0);
                exit(0);
            }

            wait(NULL);
            info(END, 6, 0);
            exit(0);
        }

        wait(NULL);
        info(END, 2, 0);
        exit(0);
    }

    //P3
    if(fork() == 0) {
        info(BEGIN, 3, 0);

        sem_init(&sem_P3T2, 0, 0);
        sem_init(&sem_P3T5, 0, 0);

        pthread_t tids[5];
        int params[5];
        for(int i = 0; i < 5; i++) {
            params[i] = i + 1;
            pthread_create(&tids[i], NULL, thread_function_process_3, &params[i]);
        }

        for(int i = 0; i < 5; i++) {
            pthread_join(tids[i], NULL);
        }

        sem_destroy(&sem_P3T2);
        sem_destroy(&sem_P3T5);

        info(END, 3, 0);
        exit(0);
    }
    
    //P4
    if(fork() == 0) {
        info(BEGIN, 4, 0);

        //P5
        if(fork() == 0) {
            info(BEGIN, 5, 0);

            sem_init(&sem_limit_threads_P5, 0, 4);
            sem_init(&sem_P5T11, 0, 0);
            sem_init(&sem_barier_P5, 0, 0);
            sem_init(&sem_counter_P5, 0, 1);
            sem_init(&sem_counter_equal_4_P5, 0, 0);

            pthread_t tids[36];
            int params[36];
            for(int i = 0; i < 36; i++) {
                params[i] = i + 1;
                pthread_create(&tids[i], NULL, thread_function_process_5, &params[i]);
            }

            for(int i = 0; i < 36; i++) {
                pthread_join(tids[i], NULL);
            }

            sem_destroy(&sem_limit_threads_P5);
            sem_destroy(&sem_P5T11);
            sem_destroy(&sem_barier_P5);
            sem_destroy(&sem_counter_P5);
            sem_destroy(&sem_counter_equal_4_P5);

            info(END, 5, 0);
            exit(0);
        }

        pthread_t tids[5];
        int params[5];
        for(int i = 0; i < 5; i++) {
            params[i] = i + 1;
            pthread_create(&tids[i], NULL, thread_function_process_4, &params[i]);
        }

        for(int i = 0; i < 5; i++) {
            pthread_join(tids[i], NULL);
        }

        wait(NULL);
        info(END, 4, 0);
        exit(0);
    }

    wait(NULL);
    wait(NULL);
    wait(NULL);

    sem_unlink("/P3T4");
    sem_unlink("/P4T2");

    info(END, 1, 0);
    return 0;
}
