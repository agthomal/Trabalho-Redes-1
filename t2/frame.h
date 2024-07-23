#ifndef FRAME_H
#define FRAME_H

#include "card.h"

#define BUFFER_SIZE 16

typedef struct info_t {
    int starter;
    int n_rounds, current_round;
    int n_cards[4], n_recived[4];
    int n_wins[4];
    card_t cards[4];
} info_t;

typedef enum data_type {
    CARD,
    MESSAGE,
} data_type;

// 8 bytes
typedef union data_t {
    card_t card;
    char message[BUFFER_SIZE];
} data_t;

// 10 bytes
typedef struct frame_t {
    int source, destination; 
    int received;
    int status;
    data_type type;
    data_t data;
} frame_t ;

int get_port (int address, int type);

void frame_init (frame_t frame);

int send_message (frame_t frame, int address);

int recieve_message (frame_t frame, int address);

#endif