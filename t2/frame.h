#ifndef FRAME_H
#define FRAME_H

#include "card.h"

typedef struct frame_t {
    int source, destination; 
    int received;
    int status;
    card_t card;
} frame_t ;

void frame_init (frame_t frame);

void send_message (frame_t frame, int address);

void recieve_message (frame_t frame, int address);

#endif