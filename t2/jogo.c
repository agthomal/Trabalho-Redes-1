// Rafael Gonçalves dos Santos - GRR20211798

/*
 * AJEITAR OS NOMES DA VARIAVEIS 
 * DEIXAR EM INGLES
 * DIMINUIR O TAMAMHO
 * 
 * CARTAS_NUMEROS = 8 =  Q, 9 = J, 10 = K, A = 1
 * CARTAS_NAIPES  = 1 = ouro, 2 = espadas, 3 = copas, 4 = paus
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define N 2
#define IP_ADRESS "192.168.0.166"
#define BUFFER_SIZE 1024

int main (int argc, char **argv) {
    int ports[N] = {32000, 32001};
    int port = ports[atoi (argv[1])];
    int envia = 0, dealer = 0, n_cartas, carta, naipe;
    int cartas_numeros[10] = {4, 5, 6, 7, 8, 9, 10, 1, 2, 3};
    int cartas_naipes[4] = {1, 2, 3, 4};

    if ((port % 32000) == 0) {
        envia  = 1;
        dealer = 1;
    }

    srand (time (NULL));
    
    while (1) {
        if (envia) {
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

            if (dealer)
                n_cartas = (N - 1) * 3;
            else
                n_cartas = N - 1;

            for (int i = 0; i < n_cartas; i++) {
                if (ports[i] != port)
                    server_addr.sin_port = htons (ports[i]);
                else
                    server_addr.sin_port = htons (ports[i + 1]);

                if (!dealer) {
                    printf ("# mensagem: ");
                    fgets (mensagem, BUFFER_SIZE, stdin);
                }
                else {
                    carta = rand() % (9 - 0 + 1) + 0;
                    naipe = rand() % (3 - 0 + 1) + 0;
                    printf ("| %d %d ", cartas_numeros[carta], cartas_naipes[naipe]);

                    carta = rand() % (9 - 0 + 1) + 0;
                    naipe = rand() % (3 - 0 + 1) + 0;
                    sprintf (mensagem, "%d %d", cartas_numeros[carta], cartas_naipes[naipe]);
                }

                sendto (sockfd, mensagem, strlen (mensagem), 0, (struct sockaddr *) &server_addr, sizeof (server_addr));

                memset (mensagem, 0, BUFFER_SIZE);
            }

            close (sockfd);
        }

        else if (!envia) {
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
            server_addr.sin_port = htons (port);

            if (bind (sockfd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
                perror("erro ao realizar a bind");
                exit(0);
            }

            printf ("# esperando...\n");

            for (int i = 0; i < (N - 1); i++) {
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
    }

    return 0;
}