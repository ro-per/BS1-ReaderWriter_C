
#ifndef MONITOR_H
#define    MONITOR_H

#ifdef    __cplusplus
extern "C" {
#endif


#ifdef    __cplusplus
}
#endif

#define READER_STATUS_IDLE                  0
#define READER_STATUS_REQUEST               1
#define READER_STATUS_WAIT                  2
#define READER_STATUS_READ                  3

#define WRITER_STATUS_IDLE                  0
#define WRITER_STATUS_REQUEST               1
#define WRITER_STATUS_WAIT                  2
#define WRITER_STATUS_WRITE                 3

extern void mon_init();

extern void mon_cleanup();

extern void mon_open_read(int reader_select);

extern void mon_close_read(int reader_select);

extern void mon_open_write(int reader_select);

extern void mon_close_write(int reader_select);

extern void mon_reader_get_status(unsigned char *status);

extern void mon_writer_get_status(unsigned char *status);

void mon_print_status();

#endif    /* MONITOR */

