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

void frame_init (frame_t frame) {
    frame.status = 0;

    send_message (frame, 0);
}

void send_message (frame_t frame, int address) {
    int sockfd;
    struct sockaddr_in dest_addr;
    
    if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror ("erro ao criar socket");
        exit (0);
    }

    memset (&dest_addr, 0, sizeof (dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr (IP_ADRESS);

    switch (address) {
        case 0:
            dest_addr.sin_port = htons (ports[1]);
            break;
        case 1:
            dest_addr.sin_port = htons (ports[2]);
            break;
        case 2:
            dest_addr.sin_port = htons (ports[3]);
            break;
        case 3:
            dest_addr.sin_port = htons (ports[0]);
            break;
    }

    sendto (sockfd, &frame, sizeof (frame), 0, (struct sockaddr *) &dest_addr, sizeof (dest_addr));
    
    #ifdef DEBUG
    printf ("# pacote enviado\n");
    #endif

    close (sockfd);
}

void recieve_message (frame_t frame, int address) {
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

    switch (address) {
        case 0:
            dest_addr.sin_port = htons (ports[0]);
            break;
        case 1:
            dest_addr.sin_port = htons (ports[1]);
            break;
        case 2:
            dest_addr.sin_port = htons (ports[2]);
            break;
        case 3:
            dest_addr.sin_port = htons (ports[3]);
            break;
    }

    if (bind (sockfd, (struct sockaddr *) &dest_addr, sizeof (dest_addr)) < 0) {
        perror("erro ao realizar a bind");
        exit(0);
    }
    
    n = recvfrom (sockfd, &frame, sizeof (frame), 0, (struct sockaddr *) &src_addr, &src_len);
    
    #ifdef DEBUG
    printf ("# pacote recebido\n");
    #endif

    if (n < 0) {
        perror ("erro ao receber mensagem");
        exit (0);
    }

    else {
        if (frame.status == 0) {
            send_message (frame, address);
        }
    }

    close (sockfd);
}
