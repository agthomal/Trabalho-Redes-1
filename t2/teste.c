#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h>

#define IP_ADRESS "127.0.0.1"
#define N 4
#define N_CARDS 40
#define BUFFER 1024

int ports[4]   = {32000, 32001, 32002, 32003};
int values[10] = {52, 53, 54, 55, 81, 74, 75, 65, 50, 51};
int suits[4]   = {1, 2, 3, 4};

typedef struct card_t {
    int value, suit;
} card_t;

typedef struct bet_t {
    int n_bets, starter, bets[N];
} bet_t;

typedef struct round_t {
    card_t cards[N];
    int n_plays, starter, n_round;
} round_t;

typedef struct info_t {
    int dealer, starter, n_cards, crr_round, op, lifes[N], bets[N], wins[N];
} info_t;

typedef union data_t {
    char msg[BUFFER];
    card_t card;
    bet_t bet;
    round_t round;
    info_t info;
} data_t;

typedef enum type_t {
    MSG,
    BETS,
    VIRA,
    CARD,
    ROUND,
    END,
    NOT,
} type_t;

typedef struct frame_t {
    int status, src, addr, dest[N], rcv[N];
    type_t type;
    data_t data;
} frame_t ;

card_t cards[10];
int n_card = 0;

int get_port (int addr, int type) {
    /* envio */
    if (type == 1) {
        if (addr == 0) return 1;
        if (addr == 1) return 2;
        if (addr == 2) return 3;
        if (addr == 3) return 0;
    }
    
    /* recebimento */
    if (type == 0) {
        if (addr == 0) return 0;
        if (addr == 1) return 1;
        if (addr == 2) return 2;
        if (addr == 3) return 3;
    }

    return -1;
}

void get_dest (int *dest, int addr) {
    if (addr == 0) { dest[0] = 1; dest[1] = 2; dest[2] = 3; }
    if (addr == 1) { dest[0] = 2; dest[1] = 3; dest[2] = 0; }
    if (addr == 2) { dest[0] = 3; dest[1] = 0; dest[2] = 1; }
    if (addr == 3) { dest[0] = 0; dest[1] = 1; dest[2] = 2; }
}

char *print_card (card_t card) {
    char suit_draw[4];
    char card_draw[13];

    if      (suits[card.suit] == 1) 
        strcpy (suit_draw, "♦");
    else if (suits[card.suit] == 2) 
        strcpy (suit_draw, "♠");
    else if (suits[card.suit] == 3) 
        strcpy (suit_draw, "♥");
    else if (suits[card.suit] == 4)
        strcpy (suit_draw, "♣");

    sprintf (card_draw, "| %c   %s |\n", (char) values[card.value], suit_draw);

    char *result = malloc ((strlen (card_draw) + 1) * sizeof(char));
    strcpy(result, card_draw);
    
    return result;
}

int verify_card (card_t *used, card_t card) {
    int i = 0;

    while (used[i].value != -1) {
        if (used[i].value == card.value && used[i].suit  == card.suit) 
            return 0;

        i++;
    }

    return 1;
}

card_t get_card (card_t *used) {
    card_t card;

    card.value = rand() % (9 - 0 + 1) + 0;
    card.suit  = rand() % (3 - 0 + 1) + 0;

    while (!verify_card (used, card)) {
        card.value = rand() % (9 - 0 + 1) + 0;
        card.suit  = rand() % (3 - 0 + 1) + 0;
    }

    return card;
}

int cmp_card (card_t *card_1, card_t card_2, int shackle) {
    if (card_1 -> value == card_2.value) {
        if (card_1 -> suit > card_2.suit)
            return 0;

        else {
            *card_1 = card_2;
            return 1;
        }
    }

    if (card_1 -> value == shackle)
        return 0;

    if (card_2.value == shackle) {
        *card_1 = card_2;
        return 1;
    }

    if (card_1 -> value > card_2.value) 
        return 0;
    
    else {
        *card_1 = card_2;
        return 1;
    }
}

int verify_dest (int *dest, int addr) {
    for (int i = 0; i < N - 1; i++) {
        if (dest[i] == addr)
            return 1;
    }

    return 0;
}

int rcv_msg (frame_t *frame, int addr) {
    int sockfd;
    struct sockaddr_in dest_addr, src_addr;
    socklen_t src_len = sizeof (src_addr);

    sockfd = socket (AF_INET, SOCK_DGRAM, 0);

    memset (&dest_addr, 0, sizeof (dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr (IP_ADRESS);
    dest_addr.sin_port = htons (ports[get_port (addr, 0)]);

    bind (sockfd, (struct sockaddr *) &dest_addr, sizeof (dest_addr));

    recvfrom (sockfd, frame, sizeof (*frame), 0, (struct sockaddr *) &src_addr, &src_len);

    //sleep (1);

    #ifdef DEBUG
    printf ("# pacote recebido\n");
    #endif 

    if (frame -> type != NOT) {
        if (frame -> src == addr) {
            frame -> status = 1;

            int i = 0;

            while (frame -> rcv[i] != -1 && i != 3) {
                if (!frame -> rcv[i]) 
                    frame -> status = 0;

                i++;
            }

            for (int i = 0; i < N - 1; i++) frame -> rcv[i]  = -1;
        }

        else if (verify_dest (frame -> dest, addr)) {
            int i = 0;
        
            while (frame -> rcv[i] != -1) i++;

            frame -> rcv[i] = 1;

            if (frame -> type == CARD) {
                printf ("%d. ", n_card);

                cards[n_card++] = frame -> data.card;

                printf ("%s", print_card (frame -> data.card));
            }

            else if (frame -> type == VIRA) {
                cards[n_card] = frame -> data.card;

                printf ("\nV. ");
                printf ("%s", print_card (frame -> data.card));
                printf ("\n");
            }

            else if (frame -> type == BETS) {
                int aux = frame -> data.bet.starter;

                for (int i = 0; i < frame -> data.bet.n_bets; i++) {
                    printf ("jogador %d faz %d\n", aux, frame -> data.bet.bets[aux]);

                    if (++aux == 4)
                        aux = 0;
                }

                char buffer[BUFFER];

                printf ("faz quantas: ");
                fgets (buffer, sizeof (buffer), stdin);
                printf ("\n");

                frame -> data.bet.bets[addr] = atoi (buffer);
                frame -> data.bet.n_bets++;
            }

            else if (frame -> type == MSG)
                printf ("%s", frame -> data.msg);

            else if (frame -> type == ROUND) {
                if ((frame -> data.round.starter == addr || frame -> data.round.n_plays != 0) && frame -> data.round.n_plays != 4) {
                    printf ("RODADA %d ----------\n", frame -> data.round.n_round);
                    for (int i = 0; i < n_card; i++) {
                        if (cards[i].value != -1) {
                            printf ("%d. ", i);
                            printf ("%s", print_card (cards[i]));
                        }
                    }
                    printf ("\nV. ");
                    printf ("%s", print_card (cards[n_card]));
                    printf ("\n");

                    int aux = frame -> data.round.starter;

                    for (int i = 0; i < frame -> data.round.n_plays; i++) {
                        printf ("jogador %d jogou ", aux);
                        printf ("%s", print_card (frame -> data.round.cards[i]));

                        if (++aux == 4)
                            aux = 0;
                    }

                    char buffer[BUFFER];

                    printf ("\nqual carta (indice): ");
                    fgets (buffer, sizeof (buffer), stdin);
                    printf ("\n");

                    frame -> data.round.cards[frame -> data.round.n_plays++] = cards[atoi (buffer)];

                    cards[atoi (buffer)].value = -1;
                }
                
            } 
        }
    }

    close (sockfd);

    return 1;
}

int send_msg (frame_t *frame, int addr) {
    int sockfd;
    struct sockaddr_in dest_addr;
    
    sockfd = socket (AF_INET, SOCK_DGRAM, 0);

    memset (&dest_addr, 0, sizeof (dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr (IP_ADRESS);
    dest_addr.sin_port = htons (ports[get_port (addr, 1)]);

    //sleep (1);

    frame -> addr = get_port (addr, 1);

    sendto (sockfd, frame, sizeof (*frame), 0, (struct sockaddr *) &dest_addr, sizeof (dest_addr));

    #ifdef DEBUG
    printf ("# pacote enviado\n");
    #endif

    close (sockfd);

    return 1;
}

int main (int argc, char **argv) {
    int run = 1, id = atoi (argv[1]);
    char buffer[BUFFER];

    frame_t *frame = malloc (sizeof (frame_t));
    for (int i = 0; i < N; i++) frame -> dest[i] = -1;
    for (int i = 0; i < N; i++) frame -> rcv[i]  = -1;
    frame -> addr = id;
    frame -> src  = id;
    frame -> status = 0;

    info_t *info = malloc (sizeof (info_t));
    info -> dealer = 0;
    info -> n_cards = 9;
    info -> starter = 1;
    info -> crr_round = 0;
    info -> op = 0;
    for (int i = 0; i < N; i++) info -> lifes[i] = 12;
    for (int i = 0; i < N; i++) info -> bets[i] = 0;
    for (int i = 0; i < N; i++) info -> wins[i] = 0;

    srand (time (NULL));
    
    while (run) {
        if (id == info -> dealer) {
            printf ("digite (d) para distribuir as cartas: ");
            fgets (buffer, sizeof (buffer), stdin);
            
            frame -> type = CARD;
            frame -> src  = id;
            frame -> addr = id;
            for (int i = 0; i < N; i++) frame -> dest[i] = -1;
            frame -> dest[0] = id;
            frame -> status = 0;

            card_t card, used[N_CARDS + 1];
            for (int i = 0; i < N_CARDS + 1; i++) {used[i].suit = -1; used[i].value = -1;}

            // sortea as cartas para os jogadores
            for (int i = 0; i < N * info -> n_cards; i++) {
                card = get_card (used);

                used[i] = card;

                if (frame -> dest[0] == id) {
                    printf ("%d. ", n_card);
                    
                    cards[n_card++] = card;

                    printf ("%s", print_card (card));
                    
                }

                else {
                    frame -> data.card = card;

                    send_msg (frame, id);
                    rcv_msg  (frame, id);

                    if (frame -> status == 1) {
                        #ifdef DEBUG
                        printf ("# pacote retornou\n");
                        #endif
                    }

                    frame -> status = 0;
                }

                if (++frame -> dest[0] == 4)
                    frame -> dest[0] = 0;
            }

            // carta vira
            card_t vira;
            int shackle;

            frame -> type = VIRA;
            get_dest (frame -> dest, id);

            vira = get_card (used);

            frame -> data.card = vira;

            send_msg (frame, id);
            rcv_msg  (frame, id);

            if (frame -> status == 1) {
                #ifdef DEBUG
                printf ("# pacote retornou\n");
                #endif
            }

            frame -> status = 0;

            printf ("\nV. ");
            printf ("%s", print_card (vira));
            printf ("\n");

            shackle = vira.value + 1;
            if (shackle == 10) shackle = 0;

            // pergunta as apostas dos jogadores
            int sum = info -> n_cards;

            frame -> type = BETS;
            get_dest (frame -> dest, id);
            frame -> data.bet.n_bets = 0;
            frame -> data.bet.starter = info -> starter;
            for (int i = 0; i < N; i++) frame -> data.bet.bets[i] = -1;

            while (sum == info -> n_cards) {
                send_msg (frame, id);
                rcv_msg  (frame, id);

                if (frame -> status == 1) {
                    #ifdef DEBUG
                    printf ("# pacote retornou\n");
                    #endif
                }

                frame -> status = 0;

                int aux = frame -> data.bet.starter;

                for (int i = 0; i < frame -> data.bet.n_bets; i++) {
                    printf ("jogador %d faz %d\n", aux, frame -> data.bet.bets[frame -> data.bet.starter + i]);

                    if (++aux == 4)
                        aux = 0;
                }

                printf ("faz quantas: ");
                fgets (buffer, sizeof (buffer), stdin);
                printf ("\n");

                frame -> data.bet.bets[id] = atoi (buffer);
                frame -> data.bet.n_bets++;

                for (int i = 0; i < N; i++) info -> bets[i] = frame -> data.bet.bets[i];

                sum = frame -> data.bet.bets[0] + frame -> data.bet.bets[1] + 
                      frame -> data.bet.bets[2] + frame -> data.bet.bets[3];
            }

            // envia as apostas para os jogadores
            frame -> type = MSG;
            get_dest (frame -> dest, id);
            
            sprintf (frame -> data.msg, "jogador 0 faz %d\njogador 1 faz %d\njogador 2 faz %d\njogador 3 faz %d\n\n", 
                     frame -> data.bet.bets[0], frame -> data.bet.bets[1], frame -> data.bet.bets[2], frame -> data.bet.bets[3]);

            send_msg (frame, id);
            rcv_msg  (frame, id); 

            if (frame -> status == 1) {
                #ifdef DEBUG
                printf ("# pacote retornou\n");
                #endif
            }

            frame -> status = 0;

            printf ("%s", frame -> data.msg);

            while (info -> n_cards != info -> crr_round) {
                // recolhe as jogadas
                frame -> type = ROUND;
                get_dest (frame -> dest, id);
                frame -> data.round.n_plays = 0;
                frame -> data.round.starter = info -> starter;
                frame -> data.round.n_round = ++info -> crr_round;

                while (frame -> data.round.n_plays != N) {
                    send_msg (frame, id);
                    rcv_msg  (frame, id);

                    if (frame -> status == 1) {
                        #ifdef DEBUG
                        printf ("# pacote retornou\n");
                        #endif
                    }

                    frame -> status = 0;

                    if (frame -> data.round.n_plays != 4) {
                        printf ("RODADA %d ----------\n", frame -> data.round.n_round);
                        for (int i = 0; i < n_card; i++) {
                            if (cards[i].value != -1) {
                                printf ("%d. ", i);
                                printf ("%s", print_card (cards[i]));
                            }
                        }
                        printf ("\nV. ");
                        printf ("%s", print_card (vira));
                        printf ("\n");

                        int aux = frame -> data.round.starter;

                        for (int i = 0; i < frame -> data.round.n_plays; i++) {
                            printf ("jogador %d jogou ", aux);
                            printf ("%s", print_card (frame -> data.round.cards[i]));

                            if (++aux == 4)
                                aux = 0;
                        }

                        printf ("\nqual carta (indice): ");
                        fgets (buffer, sizeof (buffer), stdin);
                        printf ("\n");

                        frame -> data.round.cards[frame -> data.round.n_plays++] = cards[atoi (buffer)];

                        cards[atoi (buffer)].value = -1;
                    }
                }

                // computa resultado
                int gtr_addr, aux;
                card_t grt_card;

                grt_card = frame -> data.round.cards[0];
                gtr_addr = info -> starter;
                aux = info -> starter;
                for (int i = 1; i < N; i++) {
                    if (++aux == 4)
                        aux = 0;

                    if (cmp_card (&grt_card, frame -> data.round.cards[i], shackle))
                        gtr_addr = aux;
                }

                // envia as cartas para os jogadores
                frame -> type = MSG;
                get_dest (frame -> dest, id);
                
                sprintf (frame -> data.msg, "cartas jogadas:\n%s%s%s%s\n", 
                        print_card (frame -> data.round.cards[0]), print_card (frame -> data.round.cards[1]), 
                        print_card (frame -> data.round.cards[2]), print_card (frame -> data.round.cards[3]));

                send_msg (frame, id);
                rcv_msg  (frame, id); 

                if (frame -> status == 1) {
                    #ifdef DEBUG
                    printf ("# pacote retornou\n");
                    #endif
                }

                frame -> status = 0;

                printf ("%s", frame -> data.msg);

                // envia o resultado para os jogadores
                frame -> type = MSG;
                get_dest (frame -> dest, id);

                sprintf (frame -> data.msg, "jogador %d venceu a rodada %d\n\n", gtr_addr, info -> crr_round);

                send_msg (frame, id);
                rcv_msg  (frame, id);

                if (frame -> status == 1) {
                    #ifdef DEBUG
                    printf ("# pacote retornou\n");
                    #endif
                }

                frame -> status = 0;

                printf ("%s", frame -> data.msg);

                info -> starter = gtr_addr;
                info -> wins[gtr_addr]++;
            }

            // repasse as informacoes para o novo dealer
            if (++info -> dealer == 4) info -> dealer = 0;
            info -> starter = info -> dealer + 1;
            if (info -> starter == 4) info -> starter = 0;
            info -> crr_round = 0;

            int diff;
            for (int i = 0; i < N; i++) {
                diff = info -> bets[i] - info -> wins[i];
                if (diff < 0)
                    diff = diff * -1;

                info -> lifes[i] = info -> lifes[i] - diff;
            }

            for (int i = 0; i < N; i++) info -> bets[i] = 0;
            for (int i = 0; i < N; i++) info -> wins[i] = 0;

            if (info -> op == 0) {
                if (--info -> n_cards == 0) {
                    info -> n_cards = info -> n_cards + 2;
                    info -> op = 1;
                }
            }

            else {
                if (++info -> n_cards == 10) {
                    info -> n_cards = info -> n_cards - 2;
                    info -> op = 0;
                }
            }

            // informa as vidas
            frame -> type = MSG;
            get_dest (frame -> dest, id);

            sprintf (frame -> data.msg, "jogador 0 tem %d vida\njogador 1 tem %d vidas\njogador 2 tem %d vidas\njogador 3 tem %d vidas\n\n", 
                     info -> lifes[0], info -> lifes[1], info -> lifes[2], info -> lifes[3]);

            send_msg (frame, id);
            rcv_msg  (frame, id);

            if (frame -> status == 1) {
                #ifdef DEBUG
                printf ("# pacote retornou\n");
                #endif
            }

            frame -> status = 0;

            printf ("%s", frame -> data.msg);

            // finaliza a rodada do carteador
            frame -> type = END;
            frame -> dest[0] = info -> dealer;
            frame -> data.info = *info;

            send_msg (frame, id);
            rcv_msg  (frame, id);

            if (frame -> status == 1) {
                #ifdef DEBUG
                printf ("# pacote retornou\n");
                #endif
            }

            frame -> status = 0;

            n_card = 0;
        }

        else {
            rcv_msg  (frame, id);

            if (frame -> type == END) {
                *info = frame -> data.info;
                n_card = 0;
            }

            send_msg (frame, frame -> addr);
        }

        for (int i = 0; i < N; i++) {
            if (info -> lifes[i] <= 0) {
                run = 0;
                break;
            }
        }

    }

    printf ("fim de jogo !");

    return 0;
}