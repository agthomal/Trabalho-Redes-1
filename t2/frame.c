#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "frame.h"
#include "card.h"

#define IP_ADRESS "127.0.0.1"

int ports[4] = {32000, 32001, 32002, 32003};

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

int recieve_message (frame_t *frame, int address) {
    int type = 1;
    int sockfd, n;
    struct sockaddr_in dest_addr, src_addr;
    socklen_t src_len = sizeof (src_addr);

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
        exit(0);
    }
    
    n = recvfrom (sockfd, frame, sizeof (*frame), 0, (struct sockaddr *) &src_addr, &src_len);
    
    #ifdef DEBUG
    printf ("# pacote recebido\n");
    #endif 

    if (n < 0) {
        perror ("erro ao receber mensagem");
        exit (0);
    }

    else {
        if (frame -> status == 0) {
            send_message (frame, address);
        }

        else {
            if (frame -> source == address) {
                if (frame -> received) {
                    #ifdef DEBUG
                    printf ("# pacote retornou\n");
                    #endif
                }

                frame -> status = 0;

                send_message (frame, address);
            }

            else if (frame -> destination == address) {
                if (frame -> type == CARD) {
                    frame -> received = 1;

                    print_card (frame -> data.card);

                    send_message (frame, address);
                }

                else if (frame -> type == PRINT) {
                    frame -> received = 1;

                    printf ("%s", frame -> data.print);

                    send_message (frame, address);
                }
            }

            else {
                send_message (frame, address);
            }
        }

        close (sockfd);

        return 1;
    }

    return 0;

}
