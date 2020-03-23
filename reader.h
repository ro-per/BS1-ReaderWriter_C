
#ifndef READER_H
#define	READER_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

#define NUMBER_READERS  10

void* reader_run(void* arg);
void reader_init();
void reader_destroy();

void reader_stop();

#endif	/* READER_H */
