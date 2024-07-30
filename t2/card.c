#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "card.h"

/* cartas com codigo da tabela ascii */
int values[10] = {52, 53, 54, 55, 81, 74, 75, 65, 50, 51};

/*
 * 1 = ouros
 * 2 = espadas
 * 3 = copas
 * 4 = paus
 */
int suits[4]   = {1, 2, 3, 4};

void print_card (card_t current_card) {
    char suit_draw[4];

    if      (suits[current_card.suit] == 1) 
        strcpy (suit_draw, "♦");
    else if (suits[current_card.suit] == 2) 
        strcpy (suit_draw, "♠");
    else if (suits[current_card.suit] == 3) 
        strcpy (suit_draw, "♥");
    else if (suits[current_card.suit] == 4)
        strcpy (suit_draw, "♣");

    printf ("| %c   %s |\n", (char) values[current_card.value], suit_draw);
}

int verify_card (card_t *used_cards, card_t current_card) {
    int i = 0;

    while (used_cards[i].value != -1) {
        if (used_cards[i].value == current_card.value &&
            used_cards[i].suit  == current_card.suit) {
                return 0;
        }
        i++;
    }

    return 1;
}

card_t get_card (card_t *used_cards) {
    card_t current_card;

    current_card.value = rand() % (9 - 0 + 1) + 0;
    current_card.suit  = rand() % (3 - 0 + 1) + 0;

    while (!verify_card (used_cards, current_card)) {
        current_card.value = rand() % (9 - 0 + 1) + 0;
        current_card.suit  = rand() % (3 - 0 + 1) + 0;
    }

    return current_card;
}
