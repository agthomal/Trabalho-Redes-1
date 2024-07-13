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

void send_msg (char *message) {
    int sockdg;
    struct sockaddr_in server_addr;
    
    if ((sockdg = socket (AF_INET, SOCK_DGRAM, 0) < 0)) {
        perror ("erro ao criar socket");
        exit (0);
    }

    memset (&server_addr, 0, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr (IP_ADRESS);
    server_addr.sin_port = htons (PORT);
    
    sendto (sockdg, message, strlen (message), 0, 
           (struct sockaddr *) &server_addr, sizeof (server_addr));

    close (sockdg);
}    

void receive_msg () {
    int sockdg;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof (client_addr);

    if ((sockdg = socket (AF_INET, SOCK_DGRAM, 0) < 0)) {
        perror ("erro ao criar socket");
        exit (0);
    }

    memset (&server_addr, 0, sizeof (server_addr));
    memset (&client_addr, 0, sizeof (client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    server_addr.sin_port = htons (PORT);

    if (bind 
       (sockdg, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
           perror ("erro ao fazer bind");
           exit (0);
    }

    if (recvfrom (sockdg, buffer, BUFFER_SIZE, 0, 
       (struct sockaddr *) &client_addr, &client_len) < 0) {
            perror ("erro ao receber mensagem");
            exit (0);
    }

    printf ("# %s\n", buffer);

    close (sockdg);
}

int main (int argc, char **argv) {
    int sockdg;
    char message[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof (client_addr);

    if (strcmp (argv[1], "e") == 0) {
        if ((sockdg = socket (AF_INET, SOCK_DGRAM, 0) < 0)) {
            perror ("erro ao criar socket");
            exit (0);
        }

        memset (&server_addr, 0, sizeof (server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr (IP_ADRESS);
        server_addr.sin_port = htons (PORT);
        
        while (1) {
            printf ("# mensagem: ");
            fgets(message, BUFFER_SIZE, stdin);
            
            sendto (sockdg, message, strlen (message), 0, 
                   (struct sockaddr *) &server_addr, sizeof (server_addr));

            memset (message, 0, BUFFER_SIZE);
        }

        close (sockdg);
    }

    else if (strcmp (argv[1], "r") == 0) {
        if ((sockdg = socket (AF_INET, SOCK_DGRAM, 0) < 0)) {
            perror ("erro ao criar socket");
            exit (0);
        }

        memset (&server_addr, 0, sizeof (server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
        server_addr.sin_port = htons (PORT);

        if (bind (sockdg, (struct sockaddr *) &server_addr, 
            sizeof (server_addr)) < 0) {
                perror ("erro ao fazer bind");
                exit (0);
        }

        while (1) {
            if (recvfrom (sockdg, message, BUFFER_SIZE, 0, 
               (struct sockaddr *) &client_addr, &client_len) < 0) {
                    perror ("erro ao receber mensagem");
                    exit (0);
            }

            printf ("# %s\n", message);
        }
    }

    return 0;
}