
#include "monitor.h"
#include "pthread.h"
#include "globals.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "reader.h"
#include "writer.h"

#define SOLUTION 3

#define MAX_READERS 10
#define MAX_WRITERS 5


void mon_print_status();

//TODO: add extra variables here
int n_readers;      //aantal lezers
int n_writers;      //aantal schrijver
BOOL busy;          //schrijver is bezig

pthread_mutex_t console_mutex;                          //This mutex is only for the printing to the console
pthread_mutex_t mutex;                                  //Use this mutex handle for the monitor
pthread_cond_t ok_reader, ok_writer;                    //You can use these handles to condition variables for the monitor
unsigned char g_reader_status[NUMBER_READERS];          //This is used to set the status of the reader to demonstrate your solution
unsigned char g_writer_status[NUMBER_WRITERS];          //This is used to set the status of the writer to demonstrate your solution

//TODO: add extra variables here
int readers_count;
int writers_count;
BOOL writer_slot;

void mon_init() {
    int i;
    //TODO: add extra variables here
    n_readers = 0;
    n_writers = 0;

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&console_mutex, NULL);
    pthread_cond_init(&ok_reader, NULL);
    pthread_cond_init(&ok_writer, NULL);

    for (i = 0; i < NUMBER_READERS; i++) {
        g_reader_status[i] = READER_STATUS_IDLE;
    }

    for (i = 0; i < NUMBER_WRITERS; i++) {
        g_writer_status[i] = WRITER_STATUS_IDLE;
    }

    //TODO: add here extra initializations of variables that you need for your solution
    readers_count = 0;
    writers_count = 0;
    writer_slot = FALSE;
}

void mon_cleanup() {
    pthread_cond_destroy(&ok_reader);
    pthread_cond_destroy(&ok_writer);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&console_mutex);
}

void mon_get_status(unsigned char *reader_status, unsigned char *writer_status) {
    memmove(reader_status, g_reader_status, sizeof(g_reader_status));
    memmove(writer_status, g_writer_status, sizeof(g_writer_status));
}

void mon_print_status() {
    int i;
    int readers = 0;
    int writers = 0;
    unsigned char reader_status[NUMBER_READERS];
    unsigned char writer_status[NUMBER_WRITERS];

    mon_get_status(reader_status, writer_status);

    pthread_mutex_lock(&console_mutex);

    system("Clear");
    printf("Status\n");
    printf("Read : ");

    for (i = 0; i < NUMBER_READERS; i++) {
        switch (reader_status[i]) {
            case READER_STATUS_IDLE:
                printf("[IDL] ");
                break;
            case READER_STATUS_REQUEST:
                printf("[REQ] ");
                break;
            case READER_STATUS_WAIT:
                printf("[WA ] ");
                break;
            case READER_STATUS_READ:
                readers++;
                printf("[RD ] ");
                break;
            default:
                printf("[???] ");
                break;
        }
    }

    printf("#RD: %i\n", readers);

    printf("Write: ");
    for (i = 0; i < NUMBER_WRITERS; i++) {
        switch (writer_status[i]) {
            case WRITER_STATUS_IDLE:
                printf("[IDL] ");
                break;
            case WRITER_STATUS_REQUEST:
                printf("[REQ] ");
                break;
            case WRITER_STATUS_WAIT:
                printf("[WA ] ");
                break;
            case WRITER_STATUS_WRITE:
                writers++;
                printf("[WR ] ");
                break;
            default:
                printf("[???] ");
                break;
        }
    }

    printf("#WR: %i\n\n", writers);

    //TODO: add extra debug to demonstrate correctness of your solution
    printf("schrijfslot =%i\n\n", writer_slot);
    printf("lezers aantal = %i - schrijvers aantal=%i \n\n", readers_count, writers_count);
    printf("n lezers=%i - n schrijvers=%i\n", n_readers, n_writers);

    pthread_mutex_unlock(&console_mutex);
}
//________________________ SOLUTION 1 ________________________ (lezers voorrang)
#if SOLUTION == 1

void mon_open_read(int reader_select) {
    // TAKE MUTEX
    pthread_mutex_lock(&mutex);
    n_readers++;
    //SET TO REQUEST
    g_reader_status[reader_select] = READER_STATUS_REQUEST;
    mon_print_status();
    //WAIT WHILE WRITER IS WRITING
    while (busy) {
        pthread_cond_wait(&ok_reader, &mutex);
    }
    //SET TO READ
    g_reader_status[reader_select] = READER_STATUS_READ;
    mon_print_status();

    //NOTIFY
    pthread_cond_signal(&ok_reader);
    // FREE MUTEX
    pthread_mutex_unlock(&mutex);

}

void mon_close_read(int reader_select) {
    //TAKE MUTEX
    pthread_mutex_lock(&mutex);
    n_readers--;

    // SET TO IDLE
    g_reader_status[reader_select] = READER_STATUS_IDLE;
    mon_print_status();

    // IF READERS ==0,WRITER CAN WRITE
    if (n_readers == 0) {
        pthread_cond_signal(&ok_writer);
    }
    //FREE MUTEX
    pthread_mutex_unlock(&mutex);
}

void mon_open_write(int writer_select) {
    // TAKE MUTEX
    pthread_mutex_lock(&mutex);

    // SET TO REQUEST
    g_writer_status[writer_select] = WRITER_STATUS_REQUEST;
    mon_print_status();

    //IF THERE ARE READERS, OR ANYONE IS WRITING, YOU MUST WAIT
    while (n_readers > 0 || (busy)) {
        pthread_cond_wait(&ok_writer, &mutex);
    }

    //SET TO WRTIE + SET TO BUSY
    g_writer_status[writer_select] = WRITER_STATUS_WRITE;
    busy = TRUE;
    mon_print_status();


    // FREE MUTEX
    pthread_mutex_unlock(&mutex);
}

void mon_close_write(int writer_select) {
    //TAKE MUTEX + FREE BUSY
    pthread_mutex_lock(&mutex);
    busy = FALSE;

    // SET TO IDLE
    g_writer_status[writer_select] = WRITER_STATUS_IDLE;
    mon_print_status();

    // IF THERE ARE NO READERS, SIGNAL READERS
    if (n_readers > 0) {
        pthread_cond_signal(&ok_reader);
    }
    //ELSE SIGNAL WRITERS
    else {
        pthread_cond_signal(&ok_writer);
    }
    //FREE MUTEX
    pthread_mutex_unlock(&mutex);
}

//________________________ SOLUTION 2 ________________________ (schrijvers voorrang)
#elif SOLUTION == 2
void mon_open_read(int reader_select) {
    // TAKE MUTEX
    pthread_mutex_lock(&mutex);

    // SET TO REQUEST
    g_reader_status[reader_select]= READER_STATUS_REQUEST;
    mon_print_status();

    //WAIT WHILE THERE ARE WRITERS WAITING
    while(n_writers>0){
        pthread_cond_wait(&ok_reader, & mutex);
    }
    n_readers++;

    //SET TO READ
    g_reader_status[reader_select]= READER_STATUS_READ;
    mon_print_status();

    //NOTIFY
    pthread_cond_signal(&ok_reader);
    pthread_mutex_unlock(& mutex);

}

void mon_close_read(int reader_select) {
    //TAKE MUTEX
    pthread_mutex_lock(&mutex);
    n_readers--;

    // SET TO IDLE
    g_reader_status[reader_select] = READER_STATUS_IDLE;
    mon_print_status();

    // IF READERS ==0,WRITER CAN WRITE
    if(n_readers==0){
        pthread_cond_signal(&ok_writer);
    }

    //FREE MUTEX
    pthread_mutex_unlock(&mutex);
}

void mon_open_write(int writer_select) {
    // TAKE MUTEX
    pthread_mutex_lock(&mutex);
    n_writers++;

    // SET TO REQUEST
    g_writer_status[writer_select]=WRITER_STATUS_REQUEST;
    mon_print_status();

    //IF THERE ARE READERS, OR ANYONE IS WRITING, YOU MUST WAIT
    while((n_readers>0)||busy){
        pthread_cond_wait(&ok_writer,&mutex);
    }
    //SET TO WRTIE + SET TO BUSY
    g_writer_status[writer_select]=WRITER_STATUS_WRITE;
    busy=TRUE;
    mon_print_status();

    // FREE MUTEX
    pthread_mutex_unlock(&mutex);
}

void mon_close_write(int writer_select) {
    //TAKE MUTEX + FREE BUSY
    pthread_mutex_lock(&mutex);
    n_writers--;
    busy=FALSE;

    // SET TO IDLE
    g_writer_status[writer_select]=WRITER_STATUS_IDLE;
    mon_print_status();
    // IF THERE ARE NO WRITERS, SIGNAL WRITERS
    if(n_writers>0){
        pthread_cond_signal(&ok_writer);
    }
        //ELSE SIGNAL READERS
    else{
        pthread_cond_signal(&ok_reader);
    }

    //FREE MUTEX
    pthread_mutex_unlock(&mutex);
}

//________________________ SOLUTION 3 ________________________ (no starvation)
#elif SOLUTION == 3

void mon_open_read(int reader_select) {
    writer_slot;
    //TODO: implement the body of this function
    pthread_mutex_lock(&mutex);
    n_readers++;

    g_reader_status[reader_select] = READER_STATUS_REQUEST;
    mon_print_status();

    while ((readers_count>=MAX_READERS)||writer_slot) {
        g_reader_status[reader_select] = READER_STATUS_WAIT;
        mon_print_status();
        pthread_cond_wait(&ok_reader, &mutex);
    }
    readers_count++;

    g_reader_status[reader_select] = READER_STATUS_READ;
    mon_print_status();

    if(readers_count<MAX_READERS){
        pthread_cond_signal(&ok_reader);

    }
    pthread_mutex_unlock(&mutex);

}

void mon_close_read(int reader_select) {
    //TODO: implement the body of this function
    pthread_mutex_lock(&mutex);
    n_readers--;

    g_reader_status[reader_select] = READER_STATUS_IDLE;
    mon_print_status();

    if ((n_readers == 0)||(readers_count>=MAX_READERS)) {
        writer_slot=TRUE;
        readers_count=0;
        writers_count=0;
        pthread_cond_signal(&ok_writer);
    } else{
        pthread_cond_signal(&ok_reader);

    }
    pthread_mutex_unlock(&mutex);
}

void mon_open_write(int writer_select) {
    //TODO: implement the body of this function
    pthread_mutex_lock(&mutex);
    n_writers++;

    g_writer_status[writer_select]=WRITER_STATUS_REQUEST;
    mon_print_status();

    while(busy || (writers_count >= MAX_WRITERS) ||(!writer_slot)){
        g_writer_status[writer_select] = WRITER_STATUS_WAIT;
        mon_print_status();
        pthread_cond_wait(&ok_writer,&mutex);
    }
    writers_count++;
    busy=TRUE;

    g_writer_status[writer_select]=WRITER_STATUS_WRITE;
    mon_print_status();

    pthread_mutex_unlock(&mutex);
}

void mon_close_write(int writer_select) {
    //TODO: implement the body of this function
    pthread_mutex_lock(&mutex);
    n_writers--;
    busy = FALSE;

    g_writer_status[writer_select] = WRITER_STATUS_IDLE;
    mon_print_status();

    if ((n_writers > 0)||(writers_count>=MAX_WRITERS)) {
        writer_slot=FALSE;
        writers_count=0;
        readers_count=0;
        pthread_cond_signal(&ok_reader);
    } else {
        pthread_cond_signal(&ok_writer);
    }
    pthread_mutex_unlock(&mutex);
}

#endif