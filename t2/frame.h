#ifndef FRAME_H
#define FRAME_H

#include "card.h"

#define BUFFER_SIZE 16

typedef struct info_t {
    int starter;
    int n_rounds, current_round;
    int n_cards;
    int n_wins[4];
    card_t cards[4];
} info_t;

typedef enum data_type {
    NOT,
    CARD,
    DEALER,
    BETS,
} data_type;

// 8 bytes
typedef union data_t {
    card_t card;
    int num;
    int n_wins[4];
} data_t;

// 10 bytes
typedef struct frame_t {
    int source, destination[3]; 
    int received[3];
    int status;
    int current_address;
    data_type type;
    data_t data;
} frame_t ;

void frame_init (frame_t *frame);

int get_port (int address, int type);

int send_message (frame_t *frame, int address);

int recieve_message (frame_t *frame, int address, char key);

#endif