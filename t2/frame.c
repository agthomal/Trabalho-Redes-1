#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>

#include "frame.h"
#include "card.h"
#include "input.h"

#define IP_ADRESS "127.0.0.1"

int ports[4] = {32000, 32001, 32002, 32003};

/*
 * return 0: bastao passou sem informacao
 * return 1: interrupcao de teclado
 * return 2: bastao passou com informacao
 * return 3: bastao retornou para a fonte corretamente 
 */


void frame_init (frame_t *frame) {
    // desativa o buffer da saida padrao (stdout), usado pela função printf 
    setvbuf (stdout, 0, _IONBF, 0);

    for (int i = 0; i < 3; i++) {
        frame -> destination[i] = -1;
        frame -> received[i] = -1;
    }

    frame -> status = 0;
    frame -> source = -1;
    frame -> type = NOT;
    memset (&frame -> data, 0, sizeof (data_t));
}

int get_port (int address, int type) {
    /* envio */
    if (type == 0) {
        switch (address) {
            case 0:
                return ports[1];
            case 1:
                return ports[2];
            case 2:
                return ports[3];
            case 3:
                return ports[0];
        }
    }
    
    /* recebimento */
    switch (address) {
        case 0:
            return ports[0];
        case 1:
            return ports[1];
        case 2:
            return ports[2];
        case 3:
            return ports[3];
    }

    return 0;
}

int verify_dest (int *destinations, int address) {
    for (int i = 0; i < 3; i++) {
        if (destinations[i] == address)
            return 1;
    }

    return 0;
}

int send_message (frame_t *frame, int address) {
    int type = 0;
    int sockfd;
    struct sockaddr_in dest_addr;
    
    if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror ("erro ao criar socket");
        exit (0);
    }

    memset (&dest_addr, 0, sizeof (dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr (IP_ADRESS);
    dest_addr.sin_port = htons (get_port (address, type));

    //sleep (1);

    switch (address) {
        case 0:
            frame -> current_address = 1;
            break;
        case 1:
            frame -> current_address = 2;
            break;
        case 2:
            frame -> current_address = 3;
            break;
        case 3:
            frame -> current_address = 0;
            break;
    }


    if (sendto (sockfd, frame, sizeof (*frame), 0, (struct sockaddr *) &dest_addr, 
        sizeof (dest_addr)) < 0) {
            perror("erro ao enviar mensagem");
            exit(0);
    }

    #ifdef DEBUG
    printf ("# pacote enviado\n");
    #endif

    close (sockfd);

    return 1;
}

int recieve_message (frame_t *frame, int address, char key) {
    int type = 1;
    int sockfd, n = 0;
    struct sockaddr_in dest_addr, src_addr;
    socklen_t src_len = sizeof (src_addr);
    fd_set readfds;

    if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror ("erro ao criar socket");
        exit (0);
    }

    memset (&dest_addr, 0, sizeof (dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr (IP_ADRESS);
    
    dest_addr.sin_port = htons (get_port (address, type));

    if (bind (sockfd, (struct sockaddr *) &dest_addr, sizeof (dest_addr)) < 0) {
        perror("erro ao realizar a bind");
        reset_terminal ();
        exit(0);
    }

    FD_ZERO (&readfds);
    FD_SET (sockfd, &readfds);
    FD_SET (STDIN_FILENO, &readfds);

    int activity = select (sockfd + 1, &readfds, NULL, NULL, NULL);

    if (activity > 0) {
        if (FD_ISSET (sockfd, &readfds)) {
            n = recvfrom (sockfd, frame, sizeof (*frame), 0, (struct sockaddr *) &src_addr, &src_len);

            if (n < 0) {
                perror ("erro ao receber mensagem");
                exit (0);
            }
        }

        if (FD_ISSET (STDIN_FILENO, &readfds)) {
            printf ("clicou\n");
            close (sockfd);
            
            return 1;
        }
    }

    //sleep (1);

    #ifdef DEBUG
    printf ("# pacote recebido\n");
    #endif 

    if (frame -> status == 1) {
        if (frame -> source == address) {
            int i = 0;

            while (frame -> received[i] != -1 && i != 3) {
                if (!frame -> received[i]) {
                    // tratar reenvio
                    #ifdef DEBUG
                    printf ("# pacote nao foi recebido por %d\n", frame -> destination[i]);
                    #endif
                }

                else {
                    #ifdef DEBUG
                    printf ("# pacote foi recebido por %d\n", frame -> destination[i]);
                    #endif
                }

                i++;
            }

            // limpar pacote
            for (int i = 0; i < 3; i++) {
                frame -> destination[i] = -1;
                frame -> received[i] = -1;
            }

            frame -> status = 0;
            frame -> source = -1;
            frame -> type = NOT;
            memset (&frame -> data, 0, sizeof (data_t));

            close (sockfd);

            return 3;
        }

        else if (verify_dest (frame -> destination, address)) {
            if (frame -> type == CARD) {
                int i = 0;

                while (frame -> received[i] != -1) i++;

                frame -> received[i] = 1;

                print_card (frame -> data.card);
            }

            if (frame -> type == DEALER) {
                int i = 0;

                while (frame -> received[i] != -1) i++;

                frame -> received[i] = 1;

                printf ("\n# o dealer é o jogador %d\n", frame -> data.num);
            }

            if (frame -> type == BETS) {
                int i = 0, num = -1;
                char buffer[16];

                while (frame -> received[i] != -1) i++;

                frame -> received[i] = 1;

                printf("faz quantas: ");

                while (num == -1) 
                    fgets (buffer, sizeof(buffer), stdin);

                sscanf (buffer, "%d", &num);

                frame -> data.n_wins[address] = num;

                printf ("calma %d\n", num);
            }

            close (sockfd);

            return 2;
        }
    }

    close (sockfd);

    return 0;
}
