
#include "writer.h"
#include "globals.h"
#include "monitor.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "sys/time.h"

#define WAIT_TIME_SECONDS              1

pthread_mutex_t g_writer_mutex;
pthread_t g_writer_thread_id[NUMBER_WRITERS];
BOOL g_writer_running[NUMBER_WRITERS];
int g_writer_selects[NUMBER_WRITERS];

BOOL writer_get_running(int select) {
    BOOL running;

    if(select > NUMBER_WRITERS) {
        return FALSE;
    }

    pthread_mutex_lock(&g_writer_mutex);
    running = g_writer_running[select];
    pthread_mutex_unlock(&g_writer_mutex);

    return running;
}

void writer_get_timespec(struct timespec * ts) {
    struct timeval tp;
    gettimeofday(&tp, NULL);

    //Convert from timeval to timespec
    ts->tv_sec  = tp.tv_sec;
    ts->tv_nsec = tp.tv_usec * 1000;
    ts->tv_sec += WAIT_TIME_SECONDS;
}

void writer_get_timespec1(struct timespec * ts) {
    struct timeval tp;
    gettimeofday(&tp, NULL);

    //Convert from timeval to timespec
    ts->tv_sec  = tp.tv_sec;
    ts->tv_nsec = tp.tv_usec * 1000;
    ts->tv_sec += 1;
}

void* writer_run(void* arg) {
    int select = *((int*)(arg));
    pthread_cond_t wait;
    pthread_mutex_t timer_mutex;
    struct timespec ts,ts1;

    pthread_cond_init(&wait, NULL);
    pthread_mutex_init(&timer_mutex, NULL);

    while(writer_get_running(select)) {
        mon_open_write(select);

        //Here you are inside the monitor
        writer_get_timespec1(&ts1);
        pthread_cond_timedwait(&wait, &timer_mutex, &ts1);

        mon_close_write(select);
        writer_get_timespec(&ts);
        pthread_cond_timedwait(&wait, &timer_mutex, &ts);
    }

    pthread_cond_destroy(&wait);
    pthread_mutex_destroy(&timer_mutex);

    return NULL;
}

void writer_init() {
    int i;

    pthread_mutex_init(&g_writer_mutex, NULL);

    pthread_mutex_lock(&g_writer_mutex);

    for(i=0; i<NUMBER_WRITERS; i++) {
        g_writer_running[i] = TRUE;
        g_writer_selects[i] = i;
    }

    pthread_mutex_unlock(&g_writer_mutex);

    for(i=0; i<NUMBER_WRITERS; i++) {
        pthread_create(&g_writer_thread_id[i], NULL, writer_run, &g_writer_selects[i]);
    }
}

void writer_stop() {
    int i;
    void* exit_status;

    for(i=0; i<NUMBER_WRITERS; i++) {
        pthread_mutex_lock(&g_writer_mutex);
        g_writer_running[i] = FALSE;
        pthread_mutex_unlock(&g_writer_mutex);

        pthread_join(g_writer_thread_id[i], &exit_status);
    }

    pthread_mutex_destroy(&g_writer_mutex);
}
