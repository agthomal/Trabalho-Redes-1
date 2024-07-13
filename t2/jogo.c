// Rafael Gonçalves dos Santos - GRR20211798

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 33000
#define IP_ADRESS "192.168.0.166"
#define BUFFER_SIZE 1024

int main (int argc, char **argv) {
    if (strcmp (argv[1], "e") == 0) {
        int sockfd;
        struct sockaddr_in server_addr;
        char mensagem[BUFFER_SIZE];

        if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror ("erro ao criar socket");
            exit (0);
        }

        memset (&server_addr, 0, sizeof (server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr (IP_ADRESS);
        server_addr.sin_port = htons (PORT);

        while (1) {
            printf ("# mensagem: ");
            fgets (mensagem, BUFFER_SIZE, stdin);

            sendto (sockfd, mensagem, strlen (mensagem), 0, (struct sockaddr *) &server_addr, sizeof (server_addr));

            memset (mensagem, 0, BUFFER_SIZE);
        }

        close (sockfd);
    }

    else if (strcmp (argv[1], "r") == 0) {
        int sockfd, n;
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len = sizeof (client_addr);
        char mensagem[BUFFER_SIZE];

        if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror ("erro ao criar socket");
            exit (0);
        }

        memset (&server_addr, 0, sizeof (server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
        server_addr.sin_port = htons (PORT);

        if (bind (sockfd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
            perror("erro ao realizar a bind");
            exit(0);
        }

        printf ("# esperando...\n");

        while (1) {
            n = recvfrom (sockfd, mensagem, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &client_len);

            if (n < 0) {
                perror ("erro ao receber mensagem");
                exit (0);
            }

            mensagem[n] = '\0';

            printf ("%s", mensagem);
        }

        close (sockfd);
    }

    return 0;
}