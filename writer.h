
#ifndef WRITER_H
#define	WRITER_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

#define NUMBER_WRITERS  10

void* writer_run(void* arg);
void writer_init();
void writer_destroy();

void writer_stop();

#endif	/* WRITER_H */

