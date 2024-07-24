#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "frame.h"

#define N 4

int main (int argc, char **argv) {
    int dealer = 0, address = atoi (argv[1]);
    frame_t *frame = malloc (sizeof (frame_t));
    info_t info;
    card_t card;

    srand (time (NULL));

    /* inicializa o jogo */
    if (address == 0) {
        dealer = 1;

        info.starter = 1;
        info.n_rounds = 5;
        for (int i = 0; i < N; i++) {
            info.n_cards[i]   = 5;
            info.n_recived[i] = 0;
        }
    }

    while (1) {
        if (dealer) {
            card_t cards[20];
            card_t used_cards[20];
            for (int i = 0; i < 20; i++) {
                used_cards[i].suit  = -1;
                used_cards[i].value = -1;
                cards[i].suit  = -1;
                cards[i].value = -1;
            }

            frame -> source      = address;
            frame -> destination = frame -> source;
            frame -> received    = 0;
            frame -> type        = CARD;

            for (int i = 0; i < info.n_rounds * N; i++) {
                card = get_card (used_cards);

                used_cards[i] = card;

                if (frame -> destination == frame -> source) 
                    print_card (card);

                else {
                    frame -> data.card = card;

                    frame -> status = 1;

                    send_message (frame, address);

                    recieve_message (frame, address);
                }

                cards[(5 * frame -> destination) + info.n_recived[frame -> destination]++] = card;
                
                if (++frame -> destination == 4)
                    frame -> destination = 0;
            }

            exit(0);
            
            // pula inha
            frame -> type = PRINT;

            printf ("\n");

            strcpy (frame -> data.print, "\n");

            frame -> destination = address;

            for (int i = 0; i < (N - 1); i++) {
                if (++frame -> destination == 4)
                    frame -> destination = 0;

                frame -> status = 1;

                send_message (frame, address);

                recieve_message (frame, address);
            }

            // pega carta vira
            frame -> type = CARD;

            card = get_card (used_cards);

            print_card (card);

            frame -> data.card = card;

            frame -> destination = address;

            for (int i = 0; i < (N - 1); i++) {
                if (++frame -> destination == 4)
                    frame -> destination = 0;

                frame -> status = 1;

                send_message (frame, address);

                recieve_message (frame, address);
            }

            // pede o palpite de cada jogador
            frame -> type = PRINT;

            printf ("\nfaz quantas?\n");

            strcpy (frame -> data.print, "\nfaz quantas?");

            frame -> destination = address;

            for (int i = 0; i < (N - 1); i++) {
                if (++frame -> destination == 4)
                    frame -> destination = 0;

                frame -> status = 1;

                send_message (frame, address);

                recieve_message (frame, address);
            }

            // avisar a vez
            /* frame -> type = PRINT;

            printf ("(vez do %d)\n", info.starter);

            frame -> destination = address;

            for (int i = 0; i < N; i++) {
                if (++frame -> destination == 4)
                    frame -> destination = 0;

                frame -> status = 1;

                if (frame -> destination == info.starter) {
                    strcpy (frame -> data.print, "(sua vez)\n");
                }

                else {
                    sprintf(frame -> data.print, "(vez do %d)\n", info.starter);
                }

                send_message (frame, address);

                recieve_message (frame, address);
            } */
                    
        }

        dealer = 0;

        recieve_message (frame, address);   
    }

    return 0;
}