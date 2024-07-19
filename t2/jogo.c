#include <stdio.h>
#include <stdlib.h>

#include "frame.h"

int main (int argc, char **argv) {
    frame_t frame;

    /* inicializa o frame */
    if (atoi (argv[1]) == 0) 
        frame_init (frame);
    
    while (1) {
        recieve_message (frame, atoi (argv[1]));   
    }

    return 0;
}