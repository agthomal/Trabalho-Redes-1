#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "frame.h"
#include "input.h"

void get_destination (int *dests, int address) {
    if (address == 0) {
        dests[0] = 1; 
        dests[1] = 2; 
        dests[2] = 3;
    }

    else if (address == 1) {
        dests[0] = 2; 
        dests[1] = 3; 
        dests[2] = 0;         
    }

    else if (address == 2) {
        dests[0] = 3; 
        dests[1] = 0; 
        dests[2] = 1;
    }

    else {
        dests[0] = 0; 
        dests[1] = 1; 
        dests[2] = 2;
    }
}

int main (int argc, char **argv) {
    set_input ();

    srand (time (NULL));

    int address = atoi (argv[1]);
    int dealer_f = 0, cards_f = 0, ret = 0;
    frame_t *frame;
    card_t use_cards[41], card;
    info_t *info;

    for (int i = 0; i < 41; i++) {
        use_cards[i].value = -1;
        use_cards[i].suit = -1;
    }

    frame = malloc (sizeof (frame_t));
    if (!frame) {
        perror ("erro ao criar socket");
        exit (0);
    }

    info = malloc (sizeof (info_t));
    if (!info) {
        perror ("erro ao criar socket");
        exit (0);
    }

    frame_init (frame);

    // info init
    info -> n_cards = 2;
    memset (info -> n_wins, -1, 4 * sizeof (int));

    printf ("# digite (d) para ser o dealer: ");

    // espera o dealer
    while (dealer_f == 0) {
        dealer_f = recieve_message (frame, address, 'd');

        if (dealer_f == 2) 
            send_message (frame, frame -> current_address);
    }

    if (dealer_f == 1) {
        frame -> type = DEALER;
        frame -> data.num = address;
        frame -> status = 1;
        frame -> source = address;
        frame -> current_address = address;

        get_destination (frame -> destination, address);

        send_message (frame, address);

        while (ret != 3)
            ret = recieve_message (frame, address, 'K');
        
        send_message (frame, address);

        ret = 0;

        printf ("\n# digite (e) para enviar as cartas: ");
    }

    while (cards_f == 0) {
        cards_f = recieve_message (frame, address, 'e');

        if (cards_f == 0 || cards_f == 2)
            send_message (frame, frame -> current_address);

        if (cards_f == 1)
            printf ("\n");
    }

    if (cards_f == 1) {
        while (frame -> current_address != address) {
            recieve_message (frame, address, 'K');
        }

        int dest = address;

        for (int i = 0; i < info -> n_cards * 4; i++) {
            frame -> type = CARD;
            frame -> status = 1;
            frame -> source = address;
            frame -> destination[0] = dest;

            card = get_card (use_cards);

            use_cards[i] = card;

            if (frame -> destination[0] == frame -> source) 
                print_card (card);

            else {
                frame -> data.card = card;

                send_message (frame, address);

                while (ret != 3)
                    ret = recieve_message (frame, address, 'K');

                ret = 0;
            }

            if (++dest == 4)
                dest = 0;
        }

        frame -> type = BETS;
        memset (frame -> data.n_wins, -1, 4 * sizeof (int));
        frame -> status = 1;
        frame -> source = address;
        frame -> current_address = address;

        get_destination (frame -> destination, address);

        send_message (frame, address);

        while (ret != 3)
            ret = recieve_message (frame, address, 'K');

        ret = 0;
    }

    else {
        while (1) {
            recieve_message (frame, address, 'K');
            
            send_message (frame, frame -> current_address);
        }
    }

    reset_terminal ();

    return 0;
}