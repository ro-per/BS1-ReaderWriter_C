
#include "reader.h"
#include "globals.h"
#include "monitor.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "sys/time.h"

#define WAIT_TIME_SECONDS              5

pthread_mutex_t g_reader_mutex;
pthread_t g_reader_thread_id[NUMBER_READERS];
BOOL g_reader_running[NUMBER_READERS];
int g_reader_selects[NUMBER_READERS];

BOOL reader_get_running(int select) {
    BOOL running;

    if(select > NUMBER_READERS) {
        return FALSE;
    }

    pthread_mutex_lock(&g_reader_mutex);
    running = g_reader_running[select];
    pthread_mutex_unlock(&g_reader_mutex);

    return running;
}

void reader_get_timespec(struct timespec * ts) {
    struct timeval tp;
    gettimeofday(&tp, NULL);

    //Convert from timeval to timespec
    ts->tv_sec  = tp.tv_sec;
    ts->tv_nsec = tp.tv_usec * 1000;
    ts->tv_sec += WAIT_TIME_SECONDS;
}

void reader_get_timespec1(struct timespec * ts) {
    struct timeval tp;
    gettimeofday(&tp, NULL);

    //Convert from timeval to timespec
    ts->tv_sec  = tp.tv_sec;
    ts->tv_nsec = tp.tv_usec * 1000;
    ts->tv_sec += 10;
}

void* reader_run(void* arg) {
    int select = *((int*)(arg));
    pthread_cond_t wait;
    pthread_mutex_t timer_mutex;
    struct timespec ts,ts1;

    pthread_cond_init(&wait, NULL);
    pthread_mutex_init(&timer_mutex, NULL);

    while(reader_get_running(select)) {
        mon_open_read(select);

        //Here you are inside the monitor
        reader_get_timespec1(&ts1);
        pthread_cond_timedwait(&wait, &timer_mutex, &ts1);

        mon_close_read(select);
        reader_get_timespec(&ts);
        pthread_cond_timedwait(&wait, &timer_mutex, &ts);
    }

    pthread_cond_destroy(&wait);
    pthread_mutex_destroy(&timer_mutex);

    return NULL;
}

void reader_init() {
    int i;

    pthread_mutex_init(&g_reader_mutex, NULL);

    pthread_mutex_lock(&g_reader_mutex);

    for(i=0; i<NUMBER_READERS; i++) {
        g_reader_running[i] = TRUE;
        g_reader_selects[i] = i;
    }

    pthread_mutex_unlock(&g_reader_mutex);

    for(i=0; i<NUMBER_READERS; i++) {
        pthread_create(&g_reader_thread_id[i], NULL, reader_run, &g_reader_selects[i]);
    }
}

void reader_stop() {
    int i;
    void* exit_status;

    for(i=0; i<NUMBER_READERS; i++) {
        pthread_mutex_lock(&g_reader_mutex);
        g_reader_running[i] = FALSE;
        pthread_mutex_unlock(&g_reader_mutex);

        pthread_join(g_reader_thread_id[i], &exit_status);
    }

    pthread_mutex_destroy(&g_reader_mutex);
}
