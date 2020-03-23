
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "monitor.h"
#include "reader.h"
#include "writer.h"


int main(int argc, char** argv) {

    unsigned char* reader_status;
    unsigned char* writer_status;

    mon_init();
    reader_init();
    writer_init();

    while(1) {
        mon_print_status();

        printf("Press enter for the next step\n");
        getc(stdin);
    }



    //getc(stdin);

    reader_stop();
    writer_stop();
    mon_cleanup();

    return (EXIT_SUCCESS);
}

