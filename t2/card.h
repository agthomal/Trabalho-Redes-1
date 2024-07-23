#ifndef CARD_H
#define CARD_H

// 4 bytes
typedef struct card_t {
    int value, suit;
} card_t;

void print_card (card_t current_card);

int verify_card (card_t *used_cards, card_t current_card);

card_t get_card (card_t *used_cards);

#endif